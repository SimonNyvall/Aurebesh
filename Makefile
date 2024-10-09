# Compiler
CC = g++

# Compiler flags
CFLAGS = -Wall -g

# Target executable
TARGET = mudshell

# Source files
SRCS = src/main.cpp src/builtin.cpp src/execution.cpp src/input.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Build the target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile .cpp files into .o files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and the target executable
clean:
	rm -f $(OBJS) $(TARGET)
