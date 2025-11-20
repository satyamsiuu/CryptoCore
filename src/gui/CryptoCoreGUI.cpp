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

    logMessages.clear();
    startTime = std::chrono::steady_clock::now();
    showProcessingPanel = true;
    isCompleted = false;
    isProcessing = true;
    completionTime = std::chrono::microseconds(0);

    std::string actionStr = (action == Action::ENCRYPT) ? "Encrypting" : "Decrypting";
    statusMessage = actionStr + " file: " + path;
    logMessages.push_back(statusMessage);

    // Start processing in a separate thread to keep GUI responsive
    std::thread([this, path, action, actionStr]()
                {
        bool success = false;
        try
        {
            if (useThreads)
            {
                std::string msg = std::string("Starting thread-based ") +
                                  (action == Action::ENCRYPT ? "encryption..." : "decryption...");
                logMessages.push_back(msg);
                success = taskManager->runWithThreads(path, action == Action::ENCRYPT);
            }
            else
            {
                std::string msg = std::string("Starting process-based ") +
                                  (action == Action::ENCRYPT ? "encryption..." : "decryption...");
                logMessages.push_back(msg);
                success = taskManager->runWithProcesses(path, action == Action::ENCRYPT);
            }

            if (success)
            {
                auto endTime = std::chrono::steady_clock::now();
                completionTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
                isCompleted = true;
                isProcessing = false;
                statusMessage = actionStr + " completed successfully!";
                logMessages.push_back(statusMessage);
            }
            else
            {
                isProcessing = false;
                std::string error = taskManager->getStatusMessage();
                statusMessage = "Error: " + error;
                logMessages.push_back(statusMessage);
            }
        }
        catch (const std::exception &e)
        {
            isProcessing = false;
            statusMessage = "Error: " + std::string(e.what());
            logMessages.push_back(statusMessage);
        } })
        .detach();
}

void CryptoCoreGUI::renderProcessingPanel()
{
    const float PANEL_WIDTH = 600;
    const float PANEL_HEIGHT = 400;

    ImGui::SetNextWindowPos(ImVec2((WINDOW_WIDTH - PANEL_WIDTH) * 0.5f, (WINDOW_HEIGHT - PANEL_HEIGHT) * 0.5f),
                            ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(PANEL_WIDTH, PANEL_HEIGHT), ImGuiCond_Appearing);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
    if (ImGui::Begin("Processing Status", &showProcessingPanel,
                     ImGuiWindowFlags_NoCollapse))
    {
        // Header: execution time / status
        if (isCompleted)
        {
            float secs = completionTime.count() / 1000000.0f; // microseconds -> seconds
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Total Execution Time: %.2f s", secs);
        }
        else if (isProcessing)
        {
            auto now = std::chrono::steady_clock::now();
            float secs = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count() / 1000000.0f;
            ImGui::Text("Execution Time: %.2f s", secs);
        }
        else
        {
            ImGui::Text("Waiting to start...");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Show threads or processes depending on mode
        if (useThreads)
        {
            ImGui::Text("Active Threads:");
            char tidInfo[64];
            pthread_t mainThread = pthread_self();
            snprintf(tidInfo, sizeof(tidInfo), "Main Thread ID: %p", (void *)mainThread);
            ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f), "%s", tidInfo);

            auto threadIds = taskManager ? taskManager->getActiveThreadIds() : std::vector<pthread_t>{};
            if (threadIds.empty())
            {
                ImGui::TextDisabled("No worker threads active");
            }
            else
            {
                for (size_t i = 0; i < threadIds.size(); ++i)
                {
                    float p = taskManager ? taskManager->getProgress(i) : 0.0f;
                    ImGui::Text("Thread %zu (ID: %p)", i, (void *)threadIds[i]);
                    std::string overlay = std::to_string(static_cast<int>(p * 100)) + "%";
                    ImGui::ProgressBar(p, ImVec2(-1, 8), overlay.c_str());
                }
            }
        }
        else
        {
            ImGui::Text("Active Processes:");
            auto pids = taskManager ? taskManager->getActiveProcessIds() : std::vector<pid_t>{};
            if (pids.empty())
            {
                ImGui::TextDisabled("No worker processes active");
            }
            else
            {
                for (size_t i = 0; i < pids.size(); ++i)
                {
                    float p = taskManager ? taskManager->getProgress(i) : 0.0f;
                    ImGui::Text("Process %zu (PID: %d)", i, pids[i]);
                    std::string overlay = std::to_string(static_cast<int>(p * 100)) + "%";
                    ImGui::ProgressBar(p, ImVec2(-1, 8), overlay.c_str());
                }
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Log
        ImGui::Text("Log Messages:");
        ImGui::BeginChild("LogMessages", ImVec2(0, 120), true);
        for (const auto &m : logMessages)
            ImGui::TextWrapped("%s", m.c_str());
        ImGui::EndChild();

        ImGui::End();
    }
    ImGui::PopStyleVar();
}