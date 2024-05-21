#include "BinWriter.h"
#include "BinReader.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>

using namespace std;

// Function to count frequencies of each byte in the file
vector<pair<unsigned char, unsigned int>> countFrequencies(string filePath, unsigned int (&frequencies)[256]) {
    BinReader reader(filePath.c_str());

    unsigned char byte;
    while (!reader.isEOF()) {
        byte = reader.readByte();
        if (reader.isEOF()) {
            break;
        }
        frequencies[byte]++;
    }

    vector<pair<unsigned char, unsigned int>> frequencyPairs;
    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) {
            frequencyPairs.push_back(make_pair(i, frequencies[i]));
        }
    }

    sort(frequencyPairs.begin(), frequencyPairs.end(), [](const pair<unsigned char, unsigned int>& a, const pair<unsigned char, unsigned int>& b) {
        return a.second > b.second;
    });

    return frequencyPairs;
}

// Recursive function to assign bits to symbols
void shannonFanoRecursive(vector<pair<unsigned char, unsigned int>>& frequencyPairs, int start, int end, string prefix, unordered_map<unsigned char, string>& encodingMap) {
    if (start == end) {
        encodingMap[frequencyPairs[start].first] = prefix;
        return;
    }

    // Calculate the total frequency sum
    unsigned int totalFrequency = 0;
    for (int i = start; i <= end; ++i) {
        totalFrequency += frequencyPairs[i].second;
    }

    // Find the point to split the array into two parts with approximately equal sums
    unsigned int halfFrequency = 0;
    int splitIndex = start;
    for (int i = start; i < end; ++i) {
        halfFrequency += frequencyPairs[i].second;
        if (halfFrequency >= totalFrequency / 2) {
            splitIndex = i;
            break;
        }
    }

    // Ensure the split index is not out of bounds
    if (splitIndex == end) {
        splitIndex = end - 1;
    }

    // Recursively assign bits
    shannonFanoRecursive(frequencyPairs, start, splitIndex, prefix + "0", encodingMap);
    shannonFanoRecursive(frequencyPairs, splitIndex + 1, end, prefix + "1", encodingMap);
}

// Function to write frequency table to file
void writeFrequencyTable(BinWriter& writer, const vector<pair<unsigned char, unsigned int>>& frequencyPairs) {
    int tableSize = frequencyPairs.size();
    writer.writeInt(tableSize); // Write the size of the frequency table
    for (const auto& p : frequencyPairs) {
        writer.writeByte(p.first);   // Write the character
        writer.writeInt(p.second);   // Write the frequency
    }
}

// Function to compress the file and print the binary code
void compressFile(const string& inputFilePath, const string& outputFilePath, const unordered_map<unsigned char, string>& encodingMap) {
    BinReader reader(inputFilePath.c_str());
    BinWriter writer(outputFilePath.c_str());

    // Write the frequency table first
    unsigned int frequencies[256] = {0};
    vector<pair<unsigned char, unsigned int>> frequencyPairs = countFrequencies(inputFilePath, frequencies);
    writeFrequencyTable(writer, frequencyPairs);

    string binaryCode;
    unsigned char byte;
    while (!reader.isEOF()) {
        byte = reader.readByte();
        if (reader.isEOF()) {
            break;
        }
        string encodedBits = encodingMap.at(byte);
        binaryCode += encodedBits;
        for (char bit : encodedBits) {
            writer.writeBit(bit == '1');
        }
    }

    // Print the binary code
    cout << "Binary code: " << binaryCode << endl;

    writer.close();
}

// Function to read frequency table from file
vector<pair<unsigned char, unsigned int>> readFrequencyTable(BinReader& reader) {
    int tableSize = reader.readInt(); // Read the size of the frequency table
    vector<pair<unsigned char, unsigned int>> frequencyPairs;
    for (int i = 0; i < tableSize; i++) {
        unsigned char character = reader.readByte();  // Read the character
        int frequency = reader.readInt();  // Read the frequency
        frequencyPairs.push_back(make_pair(character, frequency));
    }
    return frequencyPairs;
}

// Function to decompress the file
void decompressFile(const string& inputFilePath, const string& outputFilePath) {
    BinReader reader(inputFilePath.c_str());
    BinWriter writer(outputFilePath.c_str());

    // Read the frequency table
    vector<pair<unsigned char, unsigned int>> frequencyPairs = readFrequencyTable(reader);

    // Debug: Check the frequency table
    cout << "Read Frequency Table:" << endl;
    for (const auto& p : frequencyPairs) {
        cout << "Character: " << static_cast<char>(p.first) << " Frequency: " << p.second << endl;
    }

    // Recreate the encoding map
    unordered_map<unsigned char, string> encodingMap;
    shannonFanoRecursive(frequencyPairs, 0, frequencyPairs.size() - 1, "", encodingMap);

    // Debug: Print the recreated encoding map
    cout << "Recreated Encoding Map:" << endl;
    for (const auto& p : encodingMap) {
        cout << static_cast<char>(p.first) << ": " << p.second << endl;
    }

    // Create a reverse map for decoding
    unordered_map<string, unsigned char> decodingMap;
    for (const auto& p : encodingMap) {
        decodingMap[p.second] = p.first;
    }

    
    // Debug: Read the encoded data and decode it
    cout << "Starting bit reading after frequency table..." << endl;
    string currentBits;
    int bitCount = 0;
    while (!reader.isEOF()) {
        bool bit = reader.readBit();
        currentBits += (bit ? '1' : '0');
        bitCount++;
        cout << "Current bits: " << currentBits << " (Total bits read: " << bitCount << ")" << endl; // Debug output

        if (decodingMap.find(currentBits) != decodingMap.end()) {
            writer.writeByte(decodingMap[currentBits]);
            cout << "Decoded character: " << decodingMap[currentBits] << endl; // Debug output
            currentBits.clear();
        }
    }

    writer.close();
}



// Function to calculate file size
size_t getFileSize(const string& filePath) {
    ifstream in(filePath, ios::binary | ios::ate);
    return in.tellg();
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: ./Shannon <c = compress, d = decompress> <input file>" << endl;
        return 1;
    }

    string choice = argv[1];
    string inputFilePath = argv[2];
    string outputFilePath = (choice == "c") ? "out.bin" : "decompressed.txt";

    if (choice == "c") {
        // Compression
        unsigned int frequencies[256] = {0};
        vector<pair<unsigned char, unsigned int>> frequencyPairs = countFrequencies(inputFilePath, frequencies);

        cout << "Character frequencies:" << endl;
        for (const auto& p : frequencyPairs) {
            cout << static_cast<char>(p.first) << " frequency: " << p.second << endl;
        }

        unordered_map<unsigned char, string> encodingMap;
        shannonFanoRecursive(frequencyPairs, 0, frequencyPairs.size() - 1, "", encodingMap);

        cout << "Shannon-Fano encoding:" << endl;
        for (const auto& p : encodingMap) {
            cout << static_cast<char>(p.first) << ": " << p.second << endl;
        }

        // Compress the file and print binary code
        compressFile(inputFilePath, outputFilePath, encodingMap);

        // Calculate compression rate
        size_t originalSize = getFileSize(inputFilePath);
        size_t compressedSize = getFileSize(outputFilePath);
        double compressionRate = static_cast<double>(originalSize) / compressedSize;

        cout << "Compression rate: " << compressionRate << endl;
    } else if (choice == "d") {
        // Decompression
        decompressFile(inputFilePath, outputFilePath);
        cout << "File decompressed to " << outputFilePath << endl;
    } else {
        cerr << "Invalid choice. Use 'c' for compress or 'd' for decompress." << endl;
        return 1;
    }

    return 0;
}
