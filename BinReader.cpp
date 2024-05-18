#include "BinReader.h"

BinReader::BinReader(const char *p) : k(0), x(0) {
    f.open(p, std::ios::binary);
}

char BinReader::readByte() {
    f.read((char*)&x, 1);
    return x;
}

bool BinReader::readBit() {
    if (k == 8) {
        readByte();
        k = 0;
    }
    bool b = (x >> k) & 1;
    k++;
    return b;
}

void BinReader::close() { // Public method to close file
    if (f.is_open())
        f.close();
}

char BinReader::getBuffer() const {
    return x; 
}

bool BinReader::isEOF() const {
    return f.eof();
}

int BinReader::readInt() {
    int y = 0;
    f.read((char*)&y, sizeof(y));
    return y;
}