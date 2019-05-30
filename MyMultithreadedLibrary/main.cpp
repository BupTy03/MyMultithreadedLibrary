#include "TaskExecutor.hpp"
#include <iostream>
#include <chrono>

using std::cout;
using std::endl;

inline unsigned long long fibonacci(unsigned val) noexcept
{
	return (val == 0)
		? val
		: (val == 1 || val == 2)
		? 1
		: fibonacci(val - 1) + fibonacci(val - 2);
}

void print_fibonacci(unsigned n)
{
	cout << "Fibonacci " << n << ": " << fibonacci(n) << endl;
}

int main(int argc, char* argv[])
{
	using namespace std::chrono_literals;

	TaskExecutor executor(4);
	executor.addTask([] { print_fibonacci(38); });
	executor.addTask([] { print_fibonacci(39); });
	executor.addTask([] { print_fibonacci(40); });
	executor.addTask([] { print_fibonacci(41); });

	std::this_thread::sleep_for(10s);

	return 0;
}