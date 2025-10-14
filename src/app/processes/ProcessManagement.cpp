#include "ProcessManagement.hpp"
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <vector>
#include <sstream>
#include <fstream>

// Simple XOR encryption/decryption key
const char CRYPTO_KEY = 0x42; // You can change this key

static void executeCryption(const std::string &taskStr)
{
    std::istringstream iss(taskStr);
    std::string filePath;
    std::string actionStr;

    if (std::getline(iss, filePath, ',') && std::getline(iss, actionStr))
    {
        std::fstream file(filePath, std::ios::in | std::ios::out | std::ios::binary);
        if (!file)
        {
            std::cout << "Failed to open file: " << filePath << std::endl;
            return;
        }

        // Read entire file
        std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});

        // XOR each byte with the key
        for (char &byte : buffer)
        {
            byte ^= CRYPTO_KEY;
        }

        // Write back to file
        file.seekp(0);
        file.write(buffer.data(), buffer.size());
        file.flush();

        std::cout << "Successfully " << (actionStr == "ENCRYPT" ? "encrypted" : "decrypted")
                  << " file: " << filePath << std::endl;
    }
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