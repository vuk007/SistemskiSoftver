

#include "cpu.hpp"
#include "memory.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <cstdlib>

using namespace std;

struct termios origTermios;

void enableRawMode(){
    tcgetattr(STDIN_FILENO, &origTermios);
    struct termios raw = origTermios;
    raw.c_lflag &= ~(ICANON | ECHO);   // bez baferovanja po liniji, bez echo-a
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void disableRawMode(){
    tcsetattr(STDIN_FILENO, TCSANOW, &origTermios);
}

bool load_hex_file(const string& filename, Memory& memory){
    ifstream in(filename);
    if (!in.is_open()){
        cerr << "Ne mogu da otvorim fajl: " << filename << "\n";
        return false;
    }

    string line;
    while (getline(in, line)){
        if (line.empty()) continue;

        size_t colon = line.find(':');
        if (colon == string::npos) continue;

        uint32_t addr = (uint32_t)stoul(line.substr(0, colon), nullptr, 16);

        size_t pos = colon + 1;
        uint32_t offset = 0;

        while (pos < line.size()){

            while (pos < line.size() && line[pos] == ' ') pos++;
            if (pos >= line.size()) break;

            size_t end = pos;
            while (end < line.size() && line[end] != ' ') end++;

            uint8_t b = (uint8_t)stoul(line.substr(pos, end - pos), nullptr, 16);
            memory.write_byte(addr + offset, b);
            offset++;

            pos = end;
        }
    }
    in.close();
    return true;
}

int main(int argc, char** argv){
    if (argc != 2){
        cerr << "Upotreba: emulator <naziv_ulazne_datoteke>\n";
        return 1;
    }
    string inputFile = argv[1];

    Memory memory;
    if (!load_hex_file(inputFile, memory)){
        return 1;
    }

    CPU cpu(memory);
    cpu.reset();

    enableRawMode();
    atexit(disableRawMode);   

    cpu.run();
    cout<<"\n";
    cpu.print_state();
    cout<<"\n";
    memory.print_memory();
    return 0;
}