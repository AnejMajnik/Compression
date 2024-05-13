#ifndef BINWRITER_H
#define BINWRITER_H

#include <fstream>

class BinWriter {
public:
    BinWriter(const char *p);
    ~BinWriter();
    void writeByte(char x);
    void writeInt(int y);
    void writeBit(bool b);
    void close();
    char getBuffer() const;

private:
    int k;
    std::ofstream f;
    char x;
};

#endif // BINWRITER_H
