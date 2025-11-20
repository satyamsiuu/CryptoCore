CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -g -DGL_SILENCE_DEPRECATION -pthread
INCLUDES = -I./src -I./src/app -I./src/app/fileHandling -I./src/app/processes \
          -I./vendor/imgui -I./vendor/imgui/backends -I/opt/homebrew/include

# ImGui source files
IMGUI_SRCS = vendor/imgui/imgui.cpp \
             vendor/imgui/imgui_draw.cpp \
             vendor/imgui/imgui_tables.cpp \
             vendor/imgui/imgui_widgets.cpp \
             vendor/imgui/backends/imgui_impl_glfw.cpp \
             vendor/imgui/backends/imgui_impl_opengl3.cpp

# Console version
CONSOLE_SRCS = src/main.cpp \
               src/app/processes/ProcessManagement.cpp \
               src/app/fileHandling/IO.cpp \
               src/app/fileHandling/ReadEnv.cpp
CONSOLE_TARGET = encrypt_decrypt.exe

# GUI version
GUI_SRCS = src/main_gui.cpp \
           src/gui/CryptoCoreGUI.cpp \
           src/app/processes/SyncStats.cpp \
           src/app/processes/ProcessManagement.cpp \
           src/app/processes/TaskManager.cpp \
           src/app/processes/BenchmarkManager.cpp \
           src/app/processes/GroqAnalyzer.cpp \
           src/app/fileHandling/IO.cpp \
           src/app/fileHandling/ReadEnv.cpp \
           $(IMGUI_SRCS)

GUI_TARGET = cryptocore_gui.exe
GUI_LIBS = -L/opt/homebrew/lib -lglfw -lcurl -framework OpenGL -framework Cocoa -framework IOKit

all: console gui

console: $(CONSOLE_TARGET)

gui: $(GUI_TARGET)

$(CONSOLE_TARGET): $(CONSOLE_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(CONSOLE_TARGET) $(CONSOLE_SRCS)

$(GUI_TARGET): $(GUI_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(GUI_TARGET) $(GUI_SRCS) $(GUI_LIBS)

clean:
	rm -f $(CONSOLE_TARGET) $(GUI_TARGET)

# Explicitly state dependencies
$(GUI_TARGET): src/gui/CryptoCoreGUI.hpp
$(CONSOLE_TARGET): src/app/processes/ProcessManagement.hpp src/app/fileHandling/IO.hpp

.PHONY: all console gui clean
