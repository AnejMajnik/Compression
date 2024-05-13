#ifndef BINREADER_H
#define BINREADER_H

#include <fstream>

class BinReader {
public:
    BinReader(const char *p);
    char readByte();
    bool readBit();
    void close();
    char getBuffer() const;
    bool isEOF() const;

private:
    int k;
    std::ifstream f;
    char x;
};

#endif // BINREADER_H
