#pragma once

#include <unifex/task.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/when_all.hpp>
#include <unifex/inline_scheduler.hpp>
#include <iostream>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

// A function that simulates asynchronous work
unifex::task<int> async_work(int value, int delay_in_ms) {
	// Simulate asynchronous work
	std::this_thread::sleep_for(std::chrono::milliseconds(delay_in_ms));
	co_return value * 2; // Return the doubled value
}

unifex::task<void> example_task() {
	unifex::inline_scheduler scheduler;

	// Launching multiple async tasks
	auto task1 = async_work(10, 100);
	auto task2 = async_work(20, 200);
	auto task3 = async_work(30, 300);

	// Wait for all tasks to complete concurrently
	auto [result1, result2, result3] = co_await unifex::when_all(std::move(task1), std::move(task2), std::move(task3));

	// Print the results
	std::cout << "Result 1: " << result1.index() << "\n";
	std::cout << "Result 2: " << result2.index() << "\n";
	std::cout << "Result 3: " << result3.index() << "\n";
}

int main() {
	// Run the example_task
	unifex::sync_wait(example_task());
	return 0;
}
