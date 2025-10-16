#include <iostream>
#include <string>
#include <limits>
#include "app/processes/ProcessManagement.hpp"
#include "app/processes/Task.hpp"
#include "app/fileHandling/IO.hpp"

void clearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void displayMenu()
{
    std::cout << "\n╔════════════════════════════════╗\n";
    std::cout << "║       CryptoCore Menu          ║\n";
    std::cout << "╠════════════════════════════════╣\n";
    std::cout << "║ 1. Encrypt a File              ║\n";
    std::cout << "║ 2. Decrypt a File              ║\n";
    std::cout << "║ 3. View File Status            ║\n";
    std::cout << "║ 4. Exit                        ║\n";
    std::cout << "╚════════════════════════════════╝\n";
    std::cout << "Enter your choice (1-4): ";
}

void processFile(const std::string &filePath, Action action)
{
    ProcessManagement pm;
    IO io(filePath);
    std::fstream fileStream = io.getFileStream();

    if (!fileStream.is_open())
    {
        std::cout << "\n❌ Failed to open file: " << filePath << std::endl;
        return;
    }

    auto task = std::make_unique<Task>(std::move(fileStream), action, filePath);
    pm.submitToQueue(std::move(task));
    pm.executeTasks();

    if (action == Action::ENCRYPT)
    {
        std::cout << "\n🔒 File is now encrypted. Checking file type...\n";
    }
    else
    {
        std::cout << "\n🔓 File is now decrypted. Checking file type...\n";
    }

    std::string cmd = "file " + filePath;
    system(cmd.c_str());
}

void viewFileStatus(const std::string &filePath)
{
    std::cout << "\n📊 File Information:\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "📂 Path: " << filePath << "\n";

    // Check if file exists
    std::ifstream file(filePath);
    if (!file)
    {
        std::cout << "❌ File does not exist!\n";
        return;
    }

    // Get file size
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::cout << "📦 Size: " << size << " bytes\n";

    // Show file type
    std::string cmd = "file " + filePath;
    std::cout << "📋 Type: ";
    system(cmd.c_str());
}

int main()
{
    std::string filePath;
    int choice;

    while (true)
    {
        clearScreen();
        displayMenu();

        if (!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 4)
        {
            clearScreen();
            std::cout << "\n👋 Thank you for using CryptoCore!\n";
            break;
        }

        if (choice >= 1 && choice <= 3)
        {
            std::cout << "\nEnter file path: ";
            if (!std::getline(std::cin, filePath))
                continue;

            switch (choice)
            {
            case 1:
                processFile(filePath, Action::ENCRYPT);
                break;
            case 2:
                processFile(filePath, Action::DECRYPT);
                break;
            case 3:
                viewFileStatus(filePath);
                break;
            }
        }

        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }

    return 0;
}
