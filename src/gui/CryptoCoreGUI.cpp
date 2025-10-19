#include "CryptoCoreGUI.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <unistd.h>

CryptoCoreGUI::CryptoCoreGUI()
    : window(nullptr), showFileDialog(false), showProcessingPanel(false),
      progress(0.0f), useThreads(true), isProcessing(false), isCompleted(false),
      completionTime(std::chrono::microseconds(0)) {}
CryptoCoreGUI::~CryptoCoreGUI()
{
    if (ImGui::GetCurrentContext() != nullptr)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    if (window)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

bool CryptoCoreGUI::init()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // GL 2.1 + GLSL 120 for macOS legacy compatibility
    const char *glsl_version = "#version 120";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // Create window with graphics context
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup custom modern style
    setupModernStyle();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    return true;
}

void CryptoCoreGUI::run()
{
    if (!window)
    {
        std::cerr << "Cannot run without initialized window" << std::endl;
        return;
    }

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render GUI elements
        renderMainWindow();
        if (showFileDialog)
            renderFileDialog();
        if (showProcessingPanel)
            renderProcessingPanel();
        renderStatusBar();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                     clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
}

void CryptoCoreGUI::setupModernStyle()
{
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;

    // Modern dark theme colors
    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.13f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.1f, 0.1f, 0.1f, 0.50f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.28f, 0.28f, 0.35f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.40f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.45f, 0.45f, 0.50f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.35f, 0.40f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.45f, 0.45f, 0.50f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.35f, 0.35f, 0.40f, 0.35f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.2f, 0.5f, 0.7f, 1.0f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 0.55f, 0.75f, 1.0f);

    // Modern style settings
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.FramePadding = ImVec2(8, 6);
    style.ItemSpacing = ImVec2(8, 6);
    style.WindowPadding = ImVec2(12, 8);
    style.ScrollbarSize = 16.0f;
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
}

void CryptoCoreGUI::renderMainWindow()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
    ImGui::Begin("CryptoCore", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Title and description
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::Text("CryptoCore File Encryption");
    ImGui::PopFont();
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    ImGui::Text("Secure your files with parallel processing");
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();

    // Processing mode selection
    ImGui::Text("Processing Mode:");
    ImGui::SameLine();
    static int mode = 0;
    ImGui::RadioButton("Threads", &mode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Processes", &mode, 1);
    useThreads = (mode == 0);
    ImGui::Spacing();

    // File selection area
    ImGui::BeginChild("FileSelection", ImVec2(0, 120), true);
    ImGui::Text("Selected File:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f),
                       "%s", selectedFile.empty() ? "None" : selectedFile.c_str());

    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 10));
    if (ImGui::Button("Select File", ImVec2(150, 40)))
        showFileDialog = true;
    ImGui::PopStyleVar();
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Spacing();

    // Action buttons
    ImGui::BeginGroup();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(30, 15));

    bool fileSelected = !selectedFile.empty();
    if (!fileSelected)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    }

    if (ImGui::Button("Encrypt File", ImVec2(200, 50)) && fileSelected)
        processFile(selectedFile, Action::ENCRYPT);

    ImGui::SameLine(250);

    if (ImGui::Button("Decrypt File", ImVec2(200, 50)) && fileSelected)
        processFile(selectedFile, Action::DECRYPT);

    if (!fileSelected)
    {
        ImGui::PopStyleVar();
    }
    ImGui::PopStyleVar();
    ImGui::EndGroup();

    // Progress bar
    if (progress > 0.0f && progress < 1.0f)
    {
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.5f, 0.7f, 1.0f));
        ImGui::ProgressBar(progress, ImVec2(-1, 8));
        ImGui::PopStyleColor();
    }

    ImGui::PopStyleVar();
    ImGui::End();
}

void CryptoCoreGUI::renderFileDialog()
{
    const float DIALOG_WIDTH = 500;
    const float DIALOG_HEIGHT = 200;

    ImGui::SetNextWindowPos(ImVec2((WINDOW_WIDTH - DIALOG_WIDTH) * 0.5f, (WINDOW_HEIGHT - DIALOG_HEIGHT) * 0.5f),
                            ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(DIALOG_WIDTH, DIALOG_HEIGHT), ImGuiCond_Appearing);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
    if (ImGui::Begin("Select File", &showFileDialog,
                     ImGuiWindowFlags_NoCollapse))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
        ImGui::Text("Enter the path to the file you want to process:");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        static char buf[512] = "";
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 8));

        // Simple input text without callbacks
        ImGui::InputText("##filepath", buf, sizeof(buf));

        ImGui::PopStyleVar();

        // Show file status if path is not empty
        if (buf[0] != '\0')
        {
            std::ifstream checkFile(buf);
            if (checkFile.good())
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
                ImGui::Text("File exists and is readable");
                ImGui::PopStyleColor();
                checkFile.close();
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                ImGui::Text("File not found or not accessible");
                ImGui::PopStyleColor();
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Center buttons
        float buttons_width = 280; // Total width of both buttons + spacing
        ImGui::SetCursorPosX((DIALOG_WIDTH - buttons_width) * 0.5f);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 8));
        bool fileExists = false;
        {
            std::ifstream checkFile(buf);
            fileExists = checkFile.good();
        }

        if (ImGui::Button("Open File", ImVec2(130, 35)) && fileExists)
        {
            selectedFile = buf;
            showFileDialog = false;
            statusMessage = "File selected: " + selectedFile;
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(130, 35)))
        {
            showFileDialog = false;
            statusMessage = "";
        }
        ImGui::PopStyleVar();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void CryptoCoreGUI::renderStatusBar()
{
    ImGui::SetNextWindowPos(ImVec2(0, WINDOW_HEIGHT - 40));
    ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH, 40));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 12));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::Begin("##StatusBar", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

    if (!statusMessage.empty())
    {
        ImVec4 statusColor = ImVec4(0.2f, 0.8f, 0.2f, 1.0f); // Success color
        if (statusMessage.find("Failed") != std::string::npos)
        {
            statusColor = ImVec4(0.8f, 0.2f, 0.2f, 1.0f); // Error color
        }
        ImGui::PushStyleColor(ImGuiCol_Text, statusColor);
        ImGui::Text("%s", statusMessage.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::End();
    ImGui::PopStyleVar(2);
}

void CryptoCoreGUI::renderProcessingPanel()
{
    const float PANEL_WIDTH = 600;
    const float PANEL_HEIGHT = 400;

    ImGui::SetNextWindowPos(ImVec2((WINDOW_WIDTH - PANEL_WIDTH) * 0.5f, (WINDOW_HEIGHT - PANEL_HEIGHT) * 0.5f),
                            ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(PANEL_WIDTH, PANEL_HEIGHT), ImGuiCond_Appearing);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
    if (ImGui::Begin("Processing Status", &showProcessingPanel))
    {
        // Check completion status
        if (taskManager && !isCompleted) // Check even if not isProcessing in case worker finished
        {
            // Update process hierarchy for real-time monitoring
            taskManager->updateProcessHierarchy();
            // Clear completed processes to get accurate status
            taskManager->clearCompletedProcesses();
            bool done = taskManager->isProcessingComplete();

            if (done)
            {
                std::lock_guard<std::mutex> lk(logMutex);
                if (!isCompleted) // Avoid duplicate updates
                {
                    isCompleted = true;
                    isProcessing = false;

                    // If we still have a dynamic line, clean it up and add completion message
                    if (hasDynamicLog && !logMessages.empty())
                    {
                        logMessages.pop_back();
                        hasDynamicLog = false;
                        // Add completion message based on the actual operation
                        std::string completedMsg = (statusMessage.find("Encrypting") != std::string::npos) ? "File encrypted!" : "File decrypted!";
                        statusMessage = completedMsg; // Update the status message for the status bar
                        logMessages.push_back(completedMsg);
                        // Add final time
                        auto finalTime = std::chrono::steady_clock::now();
                        completionTime = std::chrono::duration_cast<std::chrono::microseconds>(finalTime - startTime);
                        char timeMsg[128];
                        long long us = completionTime.count();
                        double secs = us / 1e6;
                        snprintf(timeMsg, sizeof(timeMsg), "[Done] Total Execution Time: %lld µs (%.6f s)", us, secs);
                        logMessages.push_back(timeMsg);
                    }
                }
            }
        } // Show execution time
        // Log messages (now at the top)
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Log Messages:");
        ImGui::BeginChild("LogMessages", ImVec2(0, 120), true);
        // Render log messages. If the last message is a dynamic "is encrypting/decrypting"
        // message and the process is still running, render that last line with a live timer
        // Snapshot log messages under lock to avoid races with worker thread
        std::vector<std::string> logsSnapshot;
        bool snapshotHasDynamic = false;
        {
            std::lock_guard<std::mutex> lk(logMutex);
            logsSnapshot = logMessages;
            snapshotHasDynamic = hasDynamicLog;
        }

        int logCount = (int)logsSnapshot.size();

        // Print all messages except the dynamic last one (we'll render it specially)
        for (int i = 0; i < logCount; ++i)
        {
            if (i == logCount - 1 && snapshotHasDynamic && isProcessing)
                continue;
            ImGui::TextWrapped("%s", logsSnapshot[i].c_str());
        }

        // If processing, show dynamic line with timer (do NOT mutate logMessages here)
        if (isProcessing)
        {
            auto currentTime = std::chrono::steady_clock::now();
            auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime).count();
            double secs = duration_us / 1e6;
            if (snapshotHasDynamic && logCount > 0)
            {
                // show the last dynamic message with timer
                ImGui::TextWrapped("%s [Running] Execution Time: %lld µs (%.6f s)", logsSnapshot.back().c_str(), duration_us, secs);
            }
            else
            {
                ImGui::TextWrapped("[Running] Execution Time: %lld µs (%.6f s)", duration_us, secs);
            }
        }
        ImGui::EndChild();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Show active threads/processes (now at the bottom)
        if (taskManager)
        {
            if (useThreads)
            {
                auto threadIds = taskManager->getActiveThreadIds();
                ImGui::Text("Active Threads:");

                // Add thread info
                char tidInfo[256];
                pthread_t mainThread = pthread_self();
                snprintf(tidInfo, sizeof(tidInfo), "Main Thread ID: %p", (void *)mainThread);
                ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "%s", tidInfo);
                ImGui::Spacing();

                if (threadIds.empty())
                {
                    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "No active worker threads");
                }
                else
                {
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.8f, 1.0f), "Worker Threads:");
                    for (size_t i = 0; i < threadIds.size(); i++)
                    {
                        float threadProgress = taskManager->getProgress(i);
                        ImGui::Text("Thread %zu (ID: %p)", i, (void *)threadIds[i]);
                        std::string overlay = std::to_string(static_cast<int>(threadProgress * 100)) + "%";
                        ImGui::ProgressBar(threadProgress, ImVec2(-1, 8), overlay.c_str());
                        ImGui::Spacing();
                    }

                    // Add verification commands
                    ImGui::Separator();
                    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Verify in terminal (macOS):");
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "sudo spindump %d 1 -arch arm64 -quiet", getpid());
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "or: sudo sample %d 1", getpid());
                }
            }
            else
            {
                auto processIds = taskManager->getActiveProcessIds();
                ImGui::Text("Active Processes:");

                // Add system info
                char pidInfo[256];
                snprintf(pidInfo, sizeof(pidInfo), "Parent Process (GUI) PID: %d", getpid());
                ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "%s", pidInfo);
                ImGui::Spacing();

                if (processIds.empty())
                {
                    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "No active child processes");
                }
                else
                {
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.8f, 1.0f), "Main Worker Processes (%zu):", processIds.size());
                    for (size_t i = 0; i < processIds.size(); i++)
                    {
                        float processProgress = taskManager->getProgress(i);
                        ImGui::Text("Worker Process %zu (PID: %d)", i, processIds[i]);
                        std::string overlay = std::to_string(static_cast<int>(processProgress * 100)) + "%";
                        ImGui::ProgressBar(processProgress, ImVec2(-1, 8), overlay.c_str());
                        ImGui::Spacing();
                    }
                    
                    // Show all related processes
                    auto allProcesses = taskManager->getAllChildProcesses();
                    if (!allProcesses.empty())
                    {
                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.2f, 1.0f), "All Related Processes:");
                        for (size_t i = 0; i < allProcesses.size(); i++)
                        {
                            pid_t pid = allProcesses[i];
                            std::string processType = "Unknown";
                            
                            // Determine process type
                            if (pid == getpid())
                                processType = "GUI Process";
                            else if (pid == getppid())
                                processType = "Parent Process";
                            else if (std::find(processIds.begin(), processIds.end(), pid) != processIds.end())
                                processType = "Worker Process";
                            else
                                processType = "System Process";
                            
                            ImGui::Text("PID: %d (%s)", pid, processType.c_str());
                        }
                    }

                    // Add verification commands
                    ImGui::Separator();
                    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Process Monitoring Commands:");
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "ps -M -p %d", getpid());
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "ps aux | grep cryptocore");
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "pstree -p %d", getpid());
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "lsof -p %d", getpid());
                    
                    // Show process hierarchy
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.6f, 0.8f, 0.2f, 1.0f), "Process Hierarchy:");
                    ImGui::Text("GUI Process (PID: %d)", getpid());
                    ImGui::Text("  └── Worker Processes:");
                    for (size_t i = 0; i < processIds.size(); i++)
                    {
                        ImGui::Text("      └── Worker %zu (PID: %d)", i, processIds[i]);
                    }
                }
            }
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void CryptoCoreGUI::processFile(const std::string &path, Action action)
{
    // Validate file exists and is accessible
    std::ifstream checkFile(path);
    if (!checkFile)
    {
        statusMessage = "Error: File not found or not accessible - " + path;
        return;
    }
    checkFile.close();

    if (!taskManager)
    {
        taskManager = std::make_unique<TaskManager>();
    }

    // Fully reset all state and log for new operation
    {
        std::lock_guard<std::mutex> lk(logMutex);
        logMessages.clear();
        startTime = std::chrono::steady_clock::now();
        showProcessingPanel = true;
        isCompleted = false;
        isProcessing = true;
        completionTime = std::chrono::microseconds(0);
    }

    std::string actionStr = (action == Action::ENCRYPT) ? "Encrypting" : "Decrypting";
    std::string completedStr = (action == Action::ENCRYPT) ? "File encrypted!" : "File decrypted!";
    statusMessage = actionStr + " file: " + path;
    // Push initial status + dynamic line under lock
    {
        std::lock_guard<std::mutex> lk(logMutex);
        logMessages.push_back(statusMessage);
        std::string dynamicMsg;
        if (action == Action::ENCRYPT)
        {
            dynamicMsg = "File is encrypting...";
        }
        else
        {
            dynamicMsg = "File is decrypting...";
        }
        logMessages.push_back(dynamicMsg);
        hasDynamicLog = true;
    }

    // Start processing in a separate thread to keep GUI responsive
    std::thread([this, path, action]()
                {
        bool success = false;
        try
        {
            if (useThreads)
            {
                success = taskManager->runWithThreads(path, action == Action::ENCRYPT);
            }
            else
            {
                success = taskManager->runWithProcesses(path, action == Action::ENCRYPT);
                // For process mode, ensure we wait for processes to complete
                // Wait until all processes are done and progress is complete
                while (!taskManager->isProcessingComplete())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
        }
        catch (const std::exception &e)
        {
            std::lock_guard<std::mutex> lk(logMutex);
            // Update status safely
            statusMessage = std::string("Error: ") + e.what();
            logMessages.push_back(statusMessage);
            isProcessing = false;
            hasDynamicLog = false;
            return;
        }

        // Update shared state after completion (under lock when modifying logs)
        auto endTime = std::chrono::steady_clock::now();
        completionTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        
        // Always update the status message with completion time, regardless of panel state
        std::string timeInfo = "Completed in " + std::to_string(completionTime.count() / 1000) + " ms";

        {
            std::lock_guard<std::mutex> lk(logMutex);
            // First clear the dynamic line
            if (hasDynamicLog && !logMessages.empty())
            {
                logMessages.pop_back();
                hasDynamicLog = false;
            }

            isProcessing = false;
            isCompleted = success;

            if (success)
            {
                std::string doneMsg = (action == Action::ENCRYPT) ? "File encrypted!" : "File decrypted!";
                statusMessage = doneMsg + " " + timeInfo;
                logMessages.push_back(doneMsg);

                char timeMsg[128];
                long long us = completionTime.count();
                double secs = us / 1e6;
                snprintf(timeMsg, sizeof(timeMsg), "[Done] Total Execution Time: %lld µs (%.6f s)", us, secs);
                logMessages.push_back(timeMsg);
            }
            else
            {
                std::string error = taskManager->getStatusMessage();
                statusMessage = "Error: " + error;
                logMessages.push_back(statusMessage);
            }
        } })
        .detach();
}