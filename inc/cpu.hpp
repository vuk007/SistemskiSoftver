#pragma once
#include <cstdint>
#include "memory.hpp"
#include <iostream>
#include <iomanip>
using namespace std;
//debug sa cerr ne cout jer je cout bufferovan
inline const int pc = 15; 
inline const int sp = 14;

class CPU {
private:

    uint32_t gpr[16]; //gde je znak potreban castovati//

    uint32_t status;
    uint32_t handler;
    uint32_t cause;

    Memory &memory;

    bool halted;

    void push(uint32_t value);
    uint32_t pop (); 
    uint32_t read_reg(uint8_t index);
    uint32_t read_sp();
    uint32_t read_pc();
    void write_sp(uint32_t value);
    void write_pc(uint32_t value);
    uint32_t read_csr(uint8_t index);
    void write_csr(uint8_t index, uint32_t value);
    void trigger_interrupt(uint32_t causeVal);

    inline void next_instruction(){
      gpr[pc] +=4; 
    }
    void write_reg(uint8_t index, uint32_t value);
    
    inline int32_t sign_extend12(uint32_t x){
      x &= 0x0FFF;
      if(x & 0x0800){
          x |= 0xFFFFF000;
      }
      return (int32_t)x;
    }
    
    void execute_instruction(
        uint8_t oc,
        uint8_t mod,
        uint8_t A,
        uint8_t B,
        uint8_t C,
        int32_t D
        
    );


    void execute_int();
    void check_interrupts();

public:

    CPU(Memory &mem):memory(mem){gpr[pc] = 0x40000000;};
    void reset();
    void run();
    void print_state();
};