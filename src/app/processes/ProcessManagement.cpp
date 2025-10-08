#include "ProcessManagement.hpp"
#include <iostream>
#include <string.h>
#include <sys/wait.h>
// #include "../encyptDecrypt/Cryption.hpp"

ProcessManagement::ProcessManagement(){}

bool ProcessManagement::submitToQueue(std::unique_ptr<Task> task)
{
    taskQueue.push(std::move(task));
    return true;
}

void ProcessManagement::executeTasks()
{
    while(!taskQueue.empty())
    {
        std::unique_ptr<Task> tasktoExecute = std::movea(taskQueue.front());
        taskQueue.pop();
        std::cout << "Executing Task: " << tasktoExecute->toString << std::endl;
        executeCryption(tasktoExecute->toString());
    }
}