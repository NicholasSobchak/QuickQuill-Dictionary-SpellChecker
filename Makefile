# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -Iinclude -I. -Ithird_party/Crow/include
LIBS = -lsqlite3

# Catch2 (system-installed). If pkg-config available, append flags.
PKG_CATCH_CFLAGS := $(shell pkg-config --cflags catch2-with-main 2>/dev/null)
PKG_CATCH_LIBS   := $(shell pkg-config --libs catch2-with-main 2>/dev/null)

CXXFLAGS += $(PKG_CATCH_CFLAGS)
LIBS     += $(PKG_CATCH_LIBS)

# Executable name
TARGET = dict
CROW_TARGET = dict_crow
TEST_TARGET = tests

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

# Test sources (Catch2 single-header lives at third_party/catch2/catch.hpp)
UNIT_TEST_SRCS = $(wildcard tests/unit/*.cpp)
UNIT_TEST_OBJS = $(UNIT_TEST_SRCS:.cpp=.o)
TEST_LIB_OBJS = src/core/Dictionary.o src/core/Trie.o src/core/SpellChecker.o src/data/Database.o

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

# Unit tests
run_unit_tests: $(UNIT_TEST_OBJS) $(TEST_LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(UNIT_TEST_OBJS) $(TEST_LIB_OBJS) $(LIBS)
	./run_unit_tests

# Aggregate test target
test: run_unit_tests

.PHONY: all clean test run_unit_tests

# Clean build files
clean:
	rm -f $(OBJS) $(CROW_OBJS) $(TARGET) $(CROW_TARGET)
