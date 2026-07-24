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
    if (address == term_out){
        put_term(value);   
        return;
    }
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

void Memory::print_memory(){
    cout << "================== MEMORY (nenulti bajtovi) ==================\n";

    if (memory.empty()){
        cout << "(prazno)\n";
        return;
    }
    vector<uint32_t> addrs;
    addrs.reserve(memory.size());
    for (auto &kv : memory) addrs.push_back(kv.first);
    sort(addrs.begin(), addrs.end());

    bool first = true;
    uint32_t lastAddr = 0;

    for (uint32_t addr : addrs){
        bool newLine = first || (addr % 8 == 0) || (addr != lastAddr + 1);

        if (newLine){
            if (!first) cout << "\n";
            cout << hex << uppercase << setw(8) << setfill('0') << addr << ": " << dec << nouppercase;
            first = false;
        }

        cout << hex << uppercase << setw(2) << setfill('0') << (int)memory[addr] << " " << dec << nouppercase;
        lastAddr = addr;
    }
    cout << "\n";
}