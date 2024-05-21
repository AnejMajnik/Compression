# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17

# Source files and executable name
SRCS = main.cpp BinReader.cpp BinWriter.cpp
EXEC = Shannon

# Default target
all: compile run

# Target to compile the program
compile:
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(SRCS)

# Target to run the program with specified arguments
run: compile
	./$(EXEC) c input.txt
	./$(EXEC) d out.bin

# Target to clean the build files
clean:
	rm -f $(EXEC)
	rm -f out.bin
	rm -f decompressed.txt

# Phony targets to avoid conflicts with files of the same name
.PHONY: all compile run clean
