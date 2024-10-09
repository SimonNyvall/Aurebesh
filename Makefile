CC = g++

CFLAGS = -Wall -g

TARGET = mudshell

# Source files
SRCS = src/main.cpp \
       src/builtin.cpp \
       src/execution.cpp \
       src/input.cpp \
       src/history/commandHistory.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Build the target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile .cpp files into .o files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
