#include <iostream>
#include <string>
#include "app/processes/ProcessManagement.hpp"
#include "app/processes/Task.hpp"
#include "app/fileHandling/IO.hpp"

int main()
{
    std::string filePath;
    std::cout << "Enter file path to encrypt/decrypt (e.g., test.txt): ";
    if (!std::getline(std::cin, filePath))
        return 1;

    ProcessManagement pm;

    // Create an encryption task
    IO io(filePath);
    std::fstream fileStream = io.getFileStream();
    if (!fileStream.is_open())
    {
        std::cout << "Failed to open file: " << filePath << std::endl;
        return 1;
    }

    // First encrypt
    auto encryptTask = std::make_unique<Task>(std::move(fileStream), Action::ENCRYPT, filePath);
    pm.submitToQueue(std::move(encryptTask));
    pm.executeTasks();

    std::cout << "\nFile is now encrypted. Checking file type...\n";

    // Run file command to show file type
    std::string cmd = "file " + filePath;
    std::cout << "File type after encryption:\n";
    system(cmd.c_str());

    std::cout << "\nPress Enter to decrypt the file...\n";
    std::cin.get(); // Now decrypt (need to reopen the file)
    IO io2(filePath);
    fileStream = io2.getFileStream();
    if (!fileStream.is_open())
    {
        std::cout << "Failed to open file for decryption: " << filePath << std::endl;
        return 1;
    }

    auto decryptTask = std::make_unique<Task>(std::move(fileStream), Action::DECRYPT, filePath);
    pm.submitToQueue(std::move(decryptTask));
    pm.executeTasks();

    return 0;
}
