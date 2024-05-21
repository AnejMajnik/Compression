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
	./$(EXEC) c Input/lena512.bmp
	./$(EXEC) d lena512.bmp.bin

# Target to clean the build files
clean:
	rm -f $(EXEC) lorem_ipsum.txt.bin lorem_ipsum.txt alice.txt.bin alice.txt oscar_age_male.csv oscar_age_male.csv.bin lena512.bmp lena512.bmp.bin

# Phony targets to avoid conflicts with files of the same name
.PHONY: all compile run clean
