CC = g++

CFLAGS = -Wall -g

TARGET = dash

# Source files
SRCS = src/main.cpp \
       src/execution.cpp \
       src/history/commandHistory.cpp \
	   src/prompt/prompt.cpp \
	   src/IO/input.cpp \
	   src/IO/tabCommandHandler.cpp \
	   src/IO/tabCdHandler.cpp \
	   src/IO/pathHelper.cpp \
	   src/IO/parse.cpp \
	   src/IO/commandColorWrap.cpp \
	   src/builtin/builtin.cpp \
	   src/builtin/shellCd.cpp \
	   src/builtin/shellExit.cpp \
	   src/builtin/shellHelp.cpp \
	   src/builtin/shellHistory.cpp \
	   src/builtin/shellLs.cpp \
	   src/builtin/shellWrite.cpp \
	   src/builtin/shellGrep.cpp \

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
