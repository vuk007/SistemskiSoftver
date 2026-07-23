#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <iostream>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
using namespace std;

class Memory{

private:
                  //adr  , data [2hex cifre]
    unordered_map<uint32_t, uint8_t> memory;
    uint32_t term_out = 0xFFFFFF00; //1 word sirine
    uint32_t term_in = 0xFFFFFF04; // sirina jedne reci 
    bool termInterruptPending = false; 
public:
    bool load_hex(string fileName);

    uint8_t read_byte(uint32_t address);
    uint32_t read_word(uint32_t address);

    void put_term(uint32_t val);
    uint32_t get_term();
    void write_byte(uint32_t address, uint8_t value);
    void write_word(uint32_t address, uint32_t value);
    bool terminal_interrupt_pending();
    void clear_terminal_interrupt();
    void poll_keyboard();
};
