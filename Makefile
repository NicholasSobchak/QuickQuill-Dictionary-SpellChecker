# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -Iinclude -I. -Ithird_party/Crow/include
LIBS = -lsqlite3

# Executable name
TARGET = dict
CROW_TARGET = dict_crow

# Source files
SRCS = src/app/main.cpp \
       src/core/Dictionary.cpp \
       src/core/Trie.cpp \
	   src/core/SpellChecker.cpp \
       src/data/Database.cpp

CROW_SRCS = src/app/main_crow.cpp \
            src/http/Server.cpp \
            src/http/routes/WordRoutes.cpp \
            src/http/handlers/WordHandler.cpp \
            src/http/dto/WordResponse.cpp \
            src/core/Dictionary.cpp \
            src/core/Trie.cpp \
            src/core/SpellChecker.cpp \
            src/data/Database.cpp

# Object files (auto-generated from SRCS)
OBJS = $(SRCS:.cpp=.o)
CROW_OBJS = $(CROW_SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Link step
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

$(CROW_TARGET): $(CROW_OBJS)
	$(CXX) $(CXXFLAGS) -o $(CROW_TARGET) $(CROW_OBJS) $(LIBS) -pthread

# Compile step (this is the magic)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJS) $(CROW_OBJS) $(TARGET) $(CROW_TARGET)
