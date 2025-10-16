#pragma once
#include <string>
#include <memory>
#include <GLFW/glfw3.h>
#include "../app/processes/ProcessManagement.hpp"

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
    void processFile(const std::string &path, Action action);
    void setupModernStyle();

    GLFWwindow *window;
    std::string selectedFile;
    std::string statusMessage;
    bool showFileDialog;
    float progress;
    ProcessManagement processManager;

    // Window settings
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;
    const char *WINDOW_TITLE = "CryptoCore GUI";
};