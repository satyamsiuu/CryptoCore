#pragma once
#include <string>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <GLFW/glfw3.h>
#include "../app/processes/ProcessManagement.hpp"
#include "../app/processes/TaskManager.hpp"

class CryptoCoreGUI
{
public:
    CryptoCoreGUI();
    ~CryptoCoreGUI();

    void run();
    bool init();

private:
    void renderMainWindow();
    void renderFileDialog();
    void renderStatusBar();
    void renderProcessingPanel();
    void processFile(const std::string &path, Action action);
    void setupModernStyle();
    void updateProgress();

    GLFWwindow *window;
    std::string selectedFile;
    std::string statusMessage;
    bool showFileDialog;
    bool showProcessingPanel;
    float progress;
    ProcessManagement processManager;
    std::unique_ptr<TaskManager> taskManager;
    bool useThreads; // true for threads, false for processes
    bool isProcessing;
    bool isCompleted;
    std::vector<std::string> logMessages;
    std::mutex logMutex;
    bool hasDynamicLog = false;
    std::chrono::steady_clock::time_point startTime;
    // Store completion time in microseconds for higher precision display
    std::chrono::microseconds completionTime;

    // Window settings
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;
    const char *WINDOW_TITLE = "CryptoCore GUI";
};