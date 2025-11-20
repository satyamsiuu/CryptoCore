#pragma once
#include <string>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <GLFW/glfw3.h>
#include "../app/processes/ProcessManagement.hpp"
#include "../app/processes/TaskManager.hpp"
// Need Action enum
#include "../app/processes/Task.hpp"
// Thread synchronization stats
#include "../app/processes/SyncStats.hpp"
#include "../app/processes/BenchmarkManager.hpp"
#include "../app/processes/GroqAnalyzer.hpp"
#include "../app/processes/EncryptionTechnique.hpp"

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
    void renderBenchmarkPanel();
    void processFile(const std::string &path, Action action);
    void runBenchmark();
    void setupModernStyle();
    void updateProgress();

    GLFWwindow *window;
    std::string selectedFile;
    std::string statusMessage;
    bool showFileDialog;
    bool showProcessingPanel;
    bool showBenchmarkPanel;
    float progress;
    ProcessManagement processManager;
    std::unique_ptr<TaskManager> taskManager;
    std::unique_ptr<BenchmarkManager> benchmarkManager;
    std::unique_ptr<GroqAnalyzer> groqAnalyzer;
    bool useThreads; // true for threads, false for processes
    std::atomic<bool> isProcessing;
    std::atomic<bool> isCompleted;
    std::atomic<bool> isBenchmarking;
    std::vector<std::string> logMessages;
    std::mutex logMutex;
    bool hasDynamicLog = false;
    std::chrono::steady_clock::time_point startTime;
    // Store completion time in microseconds for higher precision display
    std::chrono::microseconds completionTime;
    std::chrono::steady_clock::time_point endTime;
    
    // Benchmark results
    std::vector<BenchmarkResult> benchmarkResults;
    GroqRecommendation groqRecommendation;
    EncryptionType selectedTechnique;

    // Window settings
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;
    const char *WINDOW_TITLE = "CryptoCore GUI";
};