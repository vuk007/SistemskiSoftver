#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

using namespace std;

class Memory{

private:
                  //adr  , data [2hex cifre]
    unordered_map<uint32_t, uint8_t> memory;

public:
    bool load_hex(string fileName);

    uint8_t read_byte(uint32_t address);
    uint32_t read_word(uint32_t address);

    void write_byte(uint32_t address, uint8_t value);
    void write_word(uint32_t address, uint32_t value);
};
