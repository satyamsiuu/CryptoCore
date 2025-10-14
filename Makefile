CC = g++
CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -g -I./src -I./src/app -I./src/app/fileHandling -I./src/app/processes

SRCS := $(shell find src -name "*.cpp")
OBJS := $(SRCS:.cpp=.o)

TARGET = encrypt_decrypt.exe

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean
