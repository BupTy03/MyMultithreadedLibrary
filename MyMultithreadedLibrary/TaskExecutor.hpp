#pragma once
#ifndef TASK_EXECUTOR
#define TASK_EXECUTOR

#include <vector>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>

struct TaskExecutor
{
	TaskExecutor(std::size_t count_threads)
		: mutexes_(count_threads)
		, cond_variables_(count_threads)
		, tasks_in_queues_(count_threads)
	{
		threads_.reserve(count_threads);
		for (std::size_t i = 0; i < count_threads; ++i) {
			threads_.emplace_back([this, i]() {
				std::unique_lock<std::mutex> lock(mutexes_[i]);
				while (!stop_all_) {
					while (!stop_all_ && tasks_in_queues_[i].empty()) {
						cond_variables_[i].wait(lock);
					}

					if (stop_all_) return;

					while (!stop_all_ && !tasks_in_queues_[i].empty()) {
						(tasks_in_queues_[i].front())();
						tasks_in_queues_[i].pop();
					}
				}
			});
		}
	}

	~TaskExecutor()
	{
		stop_all_ = true;
		for (auto&& cond_var : cond_variables_) {
			cond_var.notify_one();
		}
		for (auto&& th : threads_) {
			th.join();
		}
	}

	template<class Task>
	bool addTask(Task tsk)
	{
		auto it = std::min_element(
			std::begin(tasks_in_queues_),
			std::end(tasks_in_queues_),
			compare_threads_queues);

		if (it == std::end(tasks_in_queues_)) {
			return false;
		}

		const auto index = std::distance(std::begin(tasks_in_queues_), it);
		if (it->empty()) {
			it->push(tsk);
			cond_variables_[index].notify_one();
		}
		else {
			it->push(tsk);
		}
		return true;
	}

	static bool compare_threads_queues(
		const std::queue<std::function<void()>>& left, 
		const std::queue<std::function<void()>>& right)
	{
		return left.size() < right.size();
	}

private:
	std::vector<std::thread> threads_;
	std::vector<std::mutex> mutexes_;
	std::vector<std::condition_variable> cond_variables_;
	std::vector<std::queue<std::function<void()>>> tasks_in_queues_;
	bool stop_all_{ false };
};

#endif // !TASK_EXECUTOR
