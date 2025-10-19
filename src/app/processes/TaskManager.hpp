#ifndef TASK_MANAGER_HPP
#define TASK_MANAGER_HPP

#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <queue>
#include <memory>
#include <string>
#include "Task.hpp"

class TaskManager; // Forward declaration

struct ThreadData
{
    TaskManager *manager;
    size_t threadId;
    size_t startOffset;
    size_t chunkSize;
    std::string filePath;
    bool isEncryption;
    float *progress;
};

class TaskManager
{
public:
    TaskManager();
    ~TaskManager();

    bool runWithThreads(const std::string &filePath, bool isEncryption, size_t numThreads = 4);
    bool runWithProcesses(const std::string &filePath, bool isEncryption, size_t numProcesses = 4);

    float getProgress(size_t threadId) const;
    std::string getStatusMessage() const;
    std::vector<pthread_t> getActiveThreadIds() const;
    std::vector<pid_t> getActiveProcessIds() const;
    std::vector<pid_t> getAllChildProcesses() const;
    bool isProcessingComplete() const;
    void clearCompletedProcesses();
    void updateProcessHierarchy();
    std::string getProcessInfo(pid_t pid) const;

    // These need to be public for the thread worker
    pthread_mutex_t mutex;
    pthread_mutex_t semaphore_mutex;
    int semaphore_count;

private:
    static void *threadWorker(void *arg);
    void processChunk(ThreadData *data);
    void initializeThreads(size_t count);
    void cleanupThreads();

    std::vector<float> threadProgress;
    std::vector<pthread_t> threadIds;
    std::vector<pid_t> processIds;
    std::vector<pid_t> allChildProcesses;
    std::string statusMessage;
    int pipefd[2];
};

#endif