#include "BinWriter.h"

BinWriter::BinWriter(const char *p) : k(0), x(0) {
    f.open(p, std::ios::binary);
}

BinWriter::~BinWriter() {
    if (k > 0) writeByte(x);
    f.close();
}

void BinWriter::writeByte(char x) {
    f.write((char*)&x, 1);
}

void BinWriter::writeInt(int y) {
    f.write((char*)&y, 4);
}

void BinWriter::writeBit(bool b) {
    if (k == 8) {
        writeByte(x);
        k = 0;
    }
    x ^= (-b ^ x) & (1 << k);
    k++;
}

void BinWriter::close() { // Public method to close file
    if (f.is_open())
        f.close();
}

char BinWriter::getBuffer() const {
    return x; 
}

