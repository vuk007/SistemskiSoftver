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
void Memory::put_term(uint32_t val){
    char c = (char)(val & 0xFF);
    std::cout << c;
    cout.flush();  //da ne bude baferisanje, odmah gurni       
}
uint32_t Memory::get_term(){
    return read_word(term_in);
}
bool Memory::terminal_interrupt_pending(){
    return termInterruptPending;
}
void Memory::clear_terminal_interrupt(){
    termInterruptPending = false;
}
void Memory::poll_keyboard(){
    
    struct timeval tv{0, 0}; //odmah proverava nema cekanja
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    if (select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0){
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1){
            write_word(term_in, (uint32_t)(unsigned char)c);
            termInterruptPending = true;
        }
    }
}