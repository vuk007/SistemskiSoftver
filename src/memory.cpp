#include "memory.hpp"

uint8_t Memory::read_byte(uint32_t address){
    auto it = memory.find(address);
    if(it == memory.end())
        return 0;
    return it->second;
}
 // 0: 0h i 1h  1: 2h i 3h ... 
uint32_t Memory::read_word(uint32_t address){
    return (uint32_t)read_byte(address) |
           ((uint32_t)read_byte(address + 1) << 8) |
           ((uint32_t)read_byte(address + 2) << 16) |
           ((uint32_t)read_byte(address + 3) << 24);
}

void Memory::write_byte(uint32_t address, uint8_t value){
    memory[address] = value;
}

void Memory::write_word(uint32_t address, uint32_t value){
  write_byte(address, value & 0xFF);
  write_byte(address + 1, (value >> 8) & 0xFF);
  write_byte(address + 2, (value >> 16) & 0xFF);
  write_byte(address + 3, (value >> 24) & 0xFF);
}
