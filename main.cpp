#include "BinWriter.h"
#include "BinReader.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>

using namespace std;

vector<pair<unsigned char, unsigned int>> countFrequencies(string filePath, unsigned int (&frequencies)[256]){
    BinReader reader(filePath.c_str());
    
    // branje byte po byte
    unsigned char byte;
    while(!reader.isEOF()){
        byte = reader.readByte();
        if(reader.isEOF()){
            break;
        }
        frequencies[byte]++;
    }

    // vektor parov (da si zapomnim indekse po sortiranju)
    vector<pair<unsigned char, unsigned int>> frequencyPairs;
    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) {  // samo frekvence, ki niso 0
            frequencyPairs.push_back(make_pair(i, frequencies[i]));
        }
    }

    // sortiranje frekvenc
    sort(frequencyPairs.begin(), frequencyPairs.end(), [](const pair<unsigned char, unsigned int>& a, const pair<unsigned char, unsigned int>& b) {
        return a.second > b.second; // sortiranje od največjega proti najmanjšemu
    });

    

    return frequencyPairs;
}

int main(int argc, char *argv[]) {
    if(argc != 3){
        cerr << "Usage: ./Shannon <c = compress, d = decompress> <input file>" << endl;
    }

    string choice = argv[1];
    string filePath = argv[2];

    unsigned int frequencies[256] = {0};
    
    vector<pair<unsigned char, unsigned int>> frequencyPairs;

    frequencyPairs = countFrequencies(filePath, frequencies);

    // izpis znakov in frekvenc
    for (const auto& p : frequencyPairs) {
        cout << static_cast<char>(p.first) << " frequency: " << p.second << endl;
    }

    return 0;
}
