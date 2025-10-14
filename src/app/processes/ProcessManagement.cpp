#include "ProcessManagement.hpp"
#include <iostream>
#include <string>
#include <sys/wait.h>

// Placeholder for actual encryption/decryption implementation.
static void executeCryption(const std::string &taskStr)
{
    std::cout << "[executeCryption] called with: " << taskStr << std::endl;
}

ProcessManagement::ProcessManagement() {}

bool ProcessManagement::submitToQueue(std::unique_ptr<Task> task)
{
    taskQueue.push(std::move(task));
    return true;
}

void ProcessManagement::executeTasks()
{
    while (!taskQueue.empty())
    {
        // Move the unique_ptr out of the queue's front
        std::unique_ptr<Task> tasktoExecute = std::move(taskQueue.front());
        taskQueue.pop();
        std::cout << "Executing Task: " << tasktoExecute->toString() << std::endl;
        executeCryption(tasktoExecute->toString());
    }
}