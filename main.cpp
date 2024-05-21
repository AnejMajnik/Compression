#include "BinWriter.h"
#include "BinReader.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

// funkcija, ki prešteje frekvence znakov in vrne vektor parov znak in frekvenca
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

// rekurzivna funkcija za izvajanje shannon fano nad frekvencami in dodeljevanje bitov
void shannonFanoRecursive(vector<pair<unsigned char, unsigned int>>& frequencyPairs, int start, int end, string prefix, unordered_map<unsigned char, string>& encodingMap) {
    if (start == end) {
        encodingMap[frequencyPairs[start].first] = prefix;
        return;
    }

    // seštevek vseh frekvenc
    unsigned int totalFrequency = 0;
    for (int i = start; i <= end; ++i) {
        totalFrequency += frequencyPairs[i].second;
    }

    // iskanje sredine glede na seštevek strani
    unsigned int halfFrequency = 0;
    int splitIndex = start;
    for (int i = start; i < end; ++i) {
        halfFrequency += frequencyPairs[i].second;
        if (halfFrequency >= totalFrequency / 2) {
            splitIndex = i;
            break;
        }
    }

    if (splitIndex == end) {
        splitIndex = end - 1;
    }

    // rekurzivno naslavljanje bitov
    shannonFanoRecursive(frequencyPairs, start, splitIndex, prefix + "0", encodingMap);
    shannonFanoRecursive(frequencyPairs, splitIndex + 1, end, prefix + "1", encodingMap);
}

// zapis frekvenčne tabele v datoteko
void writeFrequencyTable(BinWriter& writer, const vector<pair<unsigned char, unsigned int>>& frequencyPairs) {
    int tableSize = frequencyPairs.size();
    writer.writeInt(tableSize); // zapis velikosti frekvenčne tabele
    for (const auto& p : frequencyPairs) {
        writer.writeByte(p.first);   // zapis znaka
        writer.writeInt(p.second);   // zapis frekvence
    }
}

// stiskanje datoteke
void compressFile(const string& inputFilePath, const string& outputFilePath, const unordered_map<unsigned char, string>& encodingMap) {
    BinReader reader(inputFilePath.c_str());
    BinWriter writer(outputFilePath.c_str());

    // najprej zapis frekvenčne tabele
    unsigned int frequencies[256] = {0};
    vector<pair<unsigned char, unsigned int>> frequencyPairs = countFrequencies(inputFilePath, frequencies);
    writeFrequencyTable(writer, frequencyPairs);

    // pretvorba datoteke v binarni zapis in pisanje v datoteko
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
    writer.close();
}

// branje frekvenčne tabele
vector<pair<unsigned char, unsigned int>> readFrequencyTable(BinReader& reader) {
    int tableSize = reader.readInt(); // velikost frekvenčne tabele
    vector<pair<unsigned char, unsigned int>> frequencyPairs;
    for (int i = 0; i < tableSize; i++) {
        unsigned char character = reader.readByte();  // branje znak po znak
        int frequency = reader.readInt();  // branje frekvence
        frequencyPairs.push_back(make_pair(character, frequency));
    }
    return frequencyPairs;
}

// razširjevanje datoteke
void decompressFile(const string& inputFilePath, const string& outputFilePath) {
    BinReader reader(inputFilePath.c_str());
    BinWriter writer(outputFilePath.c_str());

    // branje frekvenčne tabele
    vector<pair<unsigned char, unsigned int>> frequencyPairs = readFrequencyTable(reader);

    // rekreacija encodingMap
    unordered_map<unsigned char, string> encodingMap;
    shannonFanoRecursive(frequencyPairs, 0, frequencyPairs.size() - 1, "", encodingMap);

    // decodingMap
    unordered_map<string, unsigned char> decodingMap;
    for (const auto& p : encodingMap) {
        decodingMap[p.second] = p.first;
    }

    reader.readByte();

    // branje in zapis bajtov
    string binaryCodeDuringDecompression;
    string currentBits;
    while (!reader.isEOF()) {
        bool bit = reader.readBit();
        binaryCodeDuringDecompression += (bit ? '1' : '0');
        currentBits += (bit ? '1' : '0');

        if (decodingMap.find(currentBits) != decodingMap.end()) {
            writer.writeByte(decodingMap[currentBits]);
            currentBits.clear();
        }
    }
    writer.close();
}

// velikost datoteke
size_t getFileSize(const string& filePath) {
    ifstream in(filePath, ios::binary | ios::ate);
    return in.tellg();
}

// ime datoteke iz poti
string getOutputFilePath(const string& inputFilePath, bool isCompress) {
    fs::path inputPath(inputFilePath);
    fs::path outputPath;

    if (isCompress) {
        // dodajanje .bin končnice
        outputPath = fs::current_path() / (inputPath.filename().string() + ".bin");
    } else {
        // odstranjevanje .bin končnice
        if (inputPath.extension() == ".bin") {
            // ime datoteke iz poti brez končnice
            outputPath = fs::current_path() / inputPath.stem().string();
        } else {
            cerr << "Error: Compressed file should have a .bin extension" << endl;
            exit(1);
        }
    }

    return outputPath.string();
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: ./Shannon <c = compress, d = decompress> <input file>" << endl;
        return 1;
    }

    string choice = argv[1];
    string inputFilePath = argv[2];
    bool isCompress = (choice == "c");

    string outputFilePath = getOutputFilePath(inputFilePath, isCompress);
    cout << outputFilePath << "\n";

    if (choice == "c") {
        unsigned int frequencies[256] = {0};
        vector<pair<unsigned char, unsigned int>> frequencyPairs = countFrequencies(inputFilePath, frequencies);

        unordered_map<unsigned char, string> encodingMap;
        shannonFanoRecursive(frequencyPairs, 0, frequencyPairs.size() - 1, "", encodingMap);

        // stiskanje datoteke
        compressFile(inputFilePath, outputFilePath, encodingMap);

        // izračun kompresijskega razmerja
        size_t originalSize = getFileSize(inputFilePath);
        size_t compressedSize = getFileSize(outputFilePath);
        double compressionRate = static_cast<double>(originalSize) / compressedSize;

        cout << "Compression rate: " << compressionRate << endl;
    } else if (choice == "d") {
        // razširjanje datoteke
        decompressFile(inputFilePath, outputFilePath);
        cout << "File decompressed to " << outputFilePath << endl;
    } else {
        cerr << "Invalid choice. Use 'c' for compress or 'd' for decompress." << endl;
        return 1;
    }

    return 0;
}
