#include "TaskManager.hpp"
#include "../fileHandling/IO.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>
#include <algorithm>

TaskManager::TaskManager() : semaphore_count(4)
{
    // Initialize mutexes
    if (pthread_mutex_init(&mutex, nullptr) != 0 ||
        pthread_mutex_init(&semaphore_mutex, nullptr) != 0)
    {
        throw std::runtime_error("Mutex initialization failed");
    }
    // Default to XOR encryption
    currentTechnique = std::make_unique<XOREncryption>();
}

TaskManager::~TaskManager()
{
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&semaphore_mutex);
}

// Encryption/decryption using current technique
void encryptDecryptChunk(char *buffer, size_t size, bool isEncryption, EncryptionTechnique *technique)
{
    if (technique)
    {
        if (isEncryption)
            technique->encryptChunk(buffer, size);
        else
            technique->decryptChunk(buffer, size);
    }
    else
    {
        // Fallback to XOR if no technique set
        const char key = 0x2A;
        for (size_t i = 0; i < size; ++i)
            buffer[i] ^= key;
    }
}

void *TaskManager::threadWorker(void *arg)
{
    auto *data = static_cast<ThreadData *>(arg);
    auto *manager = data->manager;

    // Initialize thread stats
    SyncStats::initThreadStats(data->threadId);

    // Wait for semaphore with stats tracking
    SyncStats::recordSemaphoreAcquire(data->threadId);
        pthread_mutex_lock(&manager->semaphore_mutex);
    while (manager->semaphore_count <= 0)
    {
        pthread_mutex_unlock(&manager->semaphore_mutex);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        pthread_mutex_lock(&manager->semaphore_mutex);
    }
    manager->semaphore_count--;
    pthread_mutex_unlock(&manager->semaphore_mutex);

    try
    {
        std::fstream file(data->filePath, std::ios::in | std::ios::out | std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Could not open file: " + data->filePath);
        }

        // Get file size
        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        // Calculate actual chunk size and offset
        size_t actualChunkSize = data->chunkSize;
        if (data->startOffset + actualChunkSize > static_cast<size_t>(fileSize))
        {
            actualChunkSize = static_cast<size_t>(fileSize) - data->startOffset;
        }

        // Read and process the chunk
        std::vector<char> buffer(actualChunkSize);

        SyncStats::recordMutexLock(data->threadId);
        pthread_mutex_lock(&manager->mutex);
        file.seekg(data->startOffset);
        file.read(buffer.data(), actualChunkSize);
        pthread_mutex_unlock(&manager->mutex);
        SyncStats::recordMutexUnlock(data->threadId);

        if (!file)
        {
            throw std::runtime_error("Error reading file chunk");
        }

        // Process the chunk
        encryptDecryptChunk(buffer.data(), actualChunkSize, data->isEncryption, manager->currentTechnique.get());

        // Write back the processed chunk
        SyncStats::recordMutexLock(data->threadId);
        pthread_mutex_lock(&manager->mutex);
        file.seekp(data->startOffset);
        file.write(buffer.data(), actualChunkSize);
        pthread_mutex_unlock(&manager->mutex);
        SyncStats::recordMutexUnlock(data->threadId);

        if (!file)
        {
            throw std::runtime_error("Error writing file chunk");
        }

        // Update progress to 100%
        *(data->progress) = 1.0f;
    }
    catch (const std::exception &e)
    {
        pthread_mutex_lock(&manager->mutex);
        manager->statusMessage = "Error in thread " + std::to_string(data->threadId) + ": " + e.what();
        pthread_mutex_unlock(&manager->mutex);
    }

    // Release semaphore
    SyncStats::recordSemaphoreRelease(data->threadId);
    pthread_mutex_lock(&manager->semaphore_mutex);
    manager->semaphore_count++;
    pthread_mutex_unlock(&manager->semaphore_mutex);
    delete data;
    return nullptr;
}

bool TaskManager::runWithThreads(const std::string &filePath, bool isEncryption, size_t numThreads)
{
    std::ifstream checkFile(filePath);
    if (!checkFile)
    {
        statusMessage = "File does not exist: " + filePath;
        return false;
    }

    checkFile.seekg(0, std::ios::end);
    std::streampos fileSize = checkFile.tellg();
    checkFile.close();

    if (fileSize == 0)
    {
        statusMessage = "File is empty: " + filePath;
        return false;
    }

    threadProgress.resize(numThreads, 0.0f);
    threadIds.resize(numThreads);

    // Calculate chunk size based on actual file size
    size_t chunkSize = (static_cast<size_t>(fileSize) + numThreads - 1) / numThreads;

    // Create threads
    for (size_t i = 0; i < numThreads; i++)
    {
        auto *data = new ThreadData{
            this,              // manager
            i,                 // threadId
            i * chunkSize,     // startOffset
            chunkSize,         // chunkSize
            filePath,          // filePath
            isEncryption,      // isEncryption
            &threadProgress[i] // progress pointer
        };

        int result = pthread_create(&threadIds[i], nullptr, threadWorker, data);
        if (result != 0)
        {
            statusMessage = "Failed to create thread " + std::to_string(i);
            return false;
        }
    }

    // Wait for all threads to complete
    for (size_t i = 0; i < numThreads; i++)
    {
        pthread_join(threadIds[i], nullptr);
    }

    return true;
}

bool TaskManager::runWithProcesses(const std::string &filePath, bool isEncryption, size_t numProcesses)
{
    std::ifstream checkFile(filePath);
    if (!checkFile)
    {
        statusMessage = "File does not exist: " + filePath;
        return false;
    }

    checkFile.seekg(0, std::ios::end);
    std::streampos fileSize = checkFile.tellg();
    checkFile.close();

    if (fileSize == 0)
    {
        statusMessage = "File is empty: " + filePath;
        return false;
    }

    // Create pipe for IPC
    if (pipe(pipefd) == -1)
    {
        statusMessage = "Failed to create pipe";
        return false;
    }

    // Calculate optimal number of processes based on file size
    // For large files (>10MB), use more processes for better performance
    size_t optimalProcesses = numProcesses;
    if (fileSize > 10 * 1024 * 1024) // 10MB
    {
        optimalProcesses = std::min(static_cast<size_t>(8), static_cast<size_t>(fileSize / (2 * 1024 * 1024))); // Max 8 processes, 1 per 2MB
        optimalProcesses = std::max(optimalProcesses, static_cast<size_t>(4));                                  // At least 4 processes
    }

    // Log the process creation info
    std::cout << "File size: " << fileSize << " bytes, Creating " << optimalProcesses << " processes" << std::endl;

    processIds.clear();
    processIds.resize(optimalProcesses);
    threadProgress.resize(optimalProcesses, 0.0f);

    size_t chunkSize = (static_cast<size_t>(fileSize) + optimalProcesses - 1) / optimalProcesses;

    // Create child processes
    for (size_t i = 0; i < optimalProcesses; i++)
    {
        pid_t pid = fork();

        if (pid == -1)
        {
            statusMessage = "Failed to create process " + std::to_string(i);
            return false;
        }

        if (pid == 0)
        {                     // Child process
            close(pipefd[0]); // Close read end

            try
            {
                std::fstream file(filePath, std::ios::in | std::ios::out | std::ios::binary);
                if (!file)
                {
                    throw std::runtime_error("Could not open file");
                }

                size_t startOffset = i * chunkSize;
                size_t actualChunkSize = chunkSize;
                if (startOffset + actualChunkSize > static_cast<size_t>(fileSize))
                {
                    actualChunkSize = static_cast<size_t>(fileSize) - startOffset;
                }

                // Read and process chunk
                std::vector<char> buffer(actualChunkSize);
                file.seekg(startOffset);
                file.read(buffer.data(), actualChunkSize);

                if (!file)
                {
                    throw std::runtime_error("Error reading file chunk");
                }

                // Process the chunk - Note: in child process, we need to use default XOR
                // as we can't share the technique pointer across processes
                const char key = 0x2A;
                for (size_t j = 0; j < actualChunkSize; ++j)
                    buffer[j] ^= key;

                // Write back
                file.seekp(startOffset);
                file.write(buffer.data(), actualChunkSize);

                if (!file)
                {
                    throw std::runtime_error("Error writing file chunk");
                }

                file.close();

                // Send 100% completion status
                std::string message = std::to_string(i) + ",100\n";
                write(pipefd[1], message.c_str(), message.length());
            }
            catch (const std::exception &e)
            {
                std::string errorMsg = std::to_string(i) + ",error:" + e.what() + "\n";
                write(pipefd[1], errorMsg.c_str(), errorMsg.length());
            }

            close(pipefd[1]);
            exit(0);
        }
        else
        { // Parent process
            processIds[i] = pid;
        }
    }

    // Update process hierarchy to include all related processes
    updateProcessHierarchy();

    // Parent process: read progress updates
    close(pipefd[1]); // Close write end in parent

    fd_set readfds;
    struct timeval tv;
    char buffer[256];
    bool processingDone = false;

    while (!processingDone)
    {
        FD_ZERO(&readfds);
        FD_SET(pipefd[0], &readfds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000; // 100ms timeout

        int result = select(pipefd[0] + 1, &readfds, NULL, NULL, &tv);

        if (result > 0)
        {
            ssize_t bytesRead = read(pipefd[0], buffer, sizeof(buffer));
            if (bytesRead > 0)
            {
                std::string message(buffer, bytesRead);
                size_t comma = message.find(',');
                if (comma != std::string::npos)
                {
                    size_t processIndex = std::stoul(message.substr(0, comma));
                    std::string progressStr = message.substr(comma + 1);

                    if (progressStr.find("error:") == 0)
                    {
                        statusMessage = "Process " + std::to_string(processIndex) + " error: " +
                                        progressStr.substr(6);
                    }
                    else
                    {
                        float progress = std::stof(progressStr) / 100.0f;
                        if (processIndex < threadProgress.size())
                        {
                            threadProgress[processIndex] = progress;
                        }
                    }
                }
            }
        }
        else if (result == 0)
        {
            // Check if all processes are done by verifying both progress and process state
            bool allProgressComplete = std::all_of(threadProgress.begin(), threadProgress.end(),
                                                   [](float progress)
                                                   { return progress >= 1.0f; });

            if (allProgressComplete)
            {
                // Verify all processes have actually finished
                processingDone = true;
                for (pid_t pid : processIds)
                {
                    int status;
                    pid_t result = waitpid(pid, &status, WNOHANG);
                    if (result == 0) // Process still running
                    {
                        processingDone = false;
                        break;
                    }
                    else if (result > 0) // Process finished
                    {
                        // Check if process exited normally
                        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
                        {
                            statusMessage = "Process " + std::to_string(pid) + " failed with status " + std::to_string(WEXITSTATUS(status));
                            processingDone = true; // Mark as done even if failed
                            break;
                        }
                    }
                }
            }
        }
    }

    close(pipefd[0]);

    // Wait for any remaining processes
    for (pid_t pid : processIds)
    {
        waitpid(pid, nullptr, 0);
    }

    if (statusMessage.empty() || statusMessage.find("Process") != 0)
    {
        statusMessage = "All processes completed successfully!";
    }

    return true;
}

float TaskManager::getProgress(size_t threadId) const
{
    if (threadId < threadProgress.size())
    {
        return threadProgress[threadId];
    }
    return 0.0f;
}

std::string TaskManager::getStatusMessage() const
{
    return statusMessage;
}

std::vector<pthread_t> TaskManager::getActiveThreadIds() const
{
    return threadIds;
}

std::vector<pid_t> TaskManager::getActiveProcessIds() const
{
    return processIds;
}

bool TaskManager::isProcessingComplete() const
{
    // Check if all progress values are at 100%
    for (float progress : threadProgress)
    {
        if (progress < 1.0f)
        {
            return false;
        }
    }

    // For processes, also check that no active processes remain
    if (!processIds.empty())
    {
        for (pid_t pid : processIds)
        {
            int status;
            pid_t result = waitpid(pid, &status, WNOHANG);
            if (result == 0) // Process still running
            {
                return false;
            }
        }
    }

    return !threadProgress.empty();
}

void TaskManager::clearCompletedProcesses()
{
    // Remove completed processes from the processIds vector
    processIds.erase(
        std::remove_if(processIds.begin(), processIds.end(),
                       [](pid_t pid)
                       {
                           int status;
                           pid_t result = waitpid(pid, &status, WNOHANG);
                           return result != 0; // Process is done (either completed or error)
                       }),
        processIds.end());
}

std::vector<pid_t> TaskManager::getAllChildProcesses() const
{
    return allChildProcesses;
}

void TaskManager::updateProcessHierarchy()
{
    allChildProcesses.clear();

    // Get all child processes of the current process
    // This includes direct children and their children
    for (pid_t pid : processIds)
    {
        allChildProcesses.push_back(pid);

        // Try to find child processes of this process
        // This is a simplified approach - in a real system you'd use /proc or similar
        // For now, we'll add some system processes that might be involved
        std::vector<pid_t> systemPids = {
            getpid(), // Current process
            getppid() // Parent process
        };

        for (pid_t sysPid : systemPids)
        {
            if (std::find(allChildProcesses.begin(), allChildProcesses.end(), sysPid) == allChildProcesses.end())
            {
                allChildProcesses.push_back(sysPid);
            }
        }
    }
}

std::string TaskManager::getProcessInfo(pid_t pid) const
{
    char info[256];
    snprintf(info, sizeof(info), "PID: %d", pid);
    return std::string(info);
}

void TaskManager::setEncryptionTechnique(std::unique_ptr<EncryptionTechnique> technique)
{
    currentTechnique = std::move(technique);
}

EncryptionType TaskManager::getCurrentTechniqueType() const
{
    return currentTechnique ? currentTechnique->getType() : EncryptionType::XOR;
}