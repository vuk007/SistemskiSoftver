

#include "cpu.hpp"

uint32_t CPU::read_sp(){
  return gpr[sp]; 
}
uint32_t CPU::read_pc(){
  return gpr[pc];
}
void CPU::write_sp(uint32_t value){
  gpr[sp] = value;
}
void CPU::write_pc(uint32_t value){
  gpr[pc] = value;
}

uint32_t CPU::read_csr(uint8_t index){
    switch(index){
        case 0: return status;
        case 1: return handler;
        case 2: return cause;
    }
    cout << "nepostojeci csr registar " << (int)index << "\n";
    return 0;
}

void CPU::write_csr(uint8_t index, uint32_t value){
    switch(index){
        case 0: status = value; return;
        case 1: handler = value; return;
        case 2: cause = value; return;
    }
    cout << "nepostojeci csr registar " << (int)index << "\n";
}

void CPU::trigger_interrupt(uint32_t causeVal){
    push(status);
    push(gpr[pc]);
    cause = causeVal;
    status &= ~0x4;      
    gpr[pc] = handler;
}

void CPU::execute_int(){
    trigger_interrupt(4);   // 4 = softverski prekid
}


uint32_t CPU::read_reg(uint8_t index){
      if(index==0)
        return 0;
    if(index < 16)return gpr[index];
    cout<<"nepostojaci registar "<< index; 
    return 0; 
}

void CPU::write_reg(uint8_t index, uint32_t value){
  if (index == 0) return; 
  if(index > 15){
    cout<<"nepostojaci registar " << index;
    return; 
  }
  gpr[index] = value;
}
    

void CPU::run(){
    while(!halted){
        uint32_t pc = gpr[15];

        uint8_t b0 = memory.read_byte(pc);
        uint8_t b1 = memory.read_byte(pc + 1);
        uint8_t b2 = memory.read_byte(pc + 2);
        uint8_t b3 = memory.read_byte(pc + 3);
       
        next_instruction();
        
        uint8_t oc = b0 >> 4;
        uint8_t mod = b0 & 0x0F;

        uint8_t regA = b1 >> 4;
        uint8_t regB = b1 & 0x0F;
        uint8_t regC = b2 >> 4;

        int32_t disp = ((b2 & 0x0F) << 8) | b3; //D - u postavci projekta
        disp = sign_extend12(disp);
        
        execute_instruction(oc, mod, regA, regB, regC, disp);
        
    }
}

void CPU::print_state(){

    cout << "-----------------------------------------------------------------\n";
    cout << "Emulated processor executed halt instruction\n";
    cout << "Emulated processor state:\n";

    for(int i = 0; i < 16; i++){
        cout << "r" << i << "=0x"
             << hex << uppercase
             << setw(8) << setfill('0')
             << read_reg(i)
             << dec;
        if(i == 3 || i == 7 || i == 11 || i == 15)
            cout << endl;
        else
            cout << "\t";
    }
}
  
void CPU::push(uint32_t value){
    gpr[sp] -= 4;
    memory.write_word(gpr[sp], value);
}
uint32_t CPU::pop(){
    uint32_t value = memory.read_word(gpr[sp]);
    gpr[sp] += 4;
    return value;
}
void CPU::execute_int(){
    push(status);
    push(gpr[15]);
    cause = 4;
    status &= ~0x4;
    gpr[15] = handler;
}
/* OBRADITI SVE SLUCAJEVE KOJI SU U POSTAVCI */
void CPU::execute_instruction(
        uint8_t oc,
        uint8_t mod,
        uint8_t A,
        uint8_t B,
        uint8_t C,
        int32_t D  
    )
    {
  switch (oc){
    case 0x0:{
        if(A == 0 && B == 0 && C == 0 && D == 0)halted = true;
        break;
      case 0x1:
      if ((mod|A|B|C|D) == 0) trigger_interrupt(4);
      else trigger_interrupt(1);
      break;
    }
    case 0x2:{
      if(mod == 0){  //push pc; pc<=gpr[A]+gpr[B]+D; 
          push(gpr[pc]);
          write_pc(read_reg(A) + read_reg(B) + D);
      }
      else if (mod == 1){ // push pc; pc<=mem32[gpr[A]+gpr[B]+D];
        push(gpr[pc]);
        write_pc(memory.read_word(read_reg(A) + read_reg(B) + D));
      }
      break;
    }

    
    case 0x3:{
      switch (mod){
        case 0b0000: // MMMM==0b0000: pc<=gpr[A]+D;
            write_pc( read_reg(A) + D);
            break;
        case 0b0001: //MMMM==0b0001: if (gpr[B] == gpr[C]) pc<=gpr[A]+D; 
            if (read_reg(B) == read_reg(C)) write_pc (read_reg(A) + D);
            break;
        case 0b0010: //MMMM==0b0010: if (gpr[B] != gpr[C]) pc<=gpr[A]+D; 
            if (read_reg(B) != read_reg(C)) write_pc  (read_reg(A) + D);
            break;
        case 0b0011: //MMMM==0b0011: if (gpr[B] signed> gpr[C]) pc<=gpr[A]+D;
            if ((int32_t)read_reg(B) > (int32_t)read_reg(C)) write_pc ( read_reg(A) + D);
            break;
        case 0b1000: //MMMM==0b1000: pc<=mem32[gpr[A]+D];
            write_pc ( memory.read_word(read_reg(A) + D));
            break;
        case 0b1001: //if (gpr[B] == gpr[C]) pc<=mem32[gpr[A]+D];
            if (read_reg(B) == read_reg(C)) write_pc ( memory.read_word(read_reg(A) + D));
            break;
        case 0b1010: //if (gpr[B] != gpr[C]) pc<=mem32[gpr[A]+D]
            if (read_reg(B) != read_reg(C)) write_pc (memory.read_word(read_reg(A) + D));
            break;
        case 0b1011: //if (gpr[B] signed> gpr[C]) pc<=mem32[gpr[A]+D]
            if ((int32_t)read_reg(B) > (int32_t)read_reg(C)) write_pc (memory.read_word(read_reg(A) + D));
            break;
        default:
            trigger_interrupt(1);
      }
      break;
    }
    
    case 0x4:{ /* xchg */
      if (mod == 0){
        uint32_t temp = read_reg(B);
        write_reg(B, read_reg(C));
        write_reg(C, temp);
      } 
      else trigger_interrupt(1);
      break;
    }
    case 0x5:{ /* aritmeticke operacije */
      switch (mod){
      case 0b0000: write_reg(A, read_reg(B) + read_reg(C)); break;
      case 0b0001: write_reg(A, read_reg(B) - read_reg(C)); break;
      case 0b0010: write_reg(A, read_reg(B) * read_reg(C)); break;
      case 0b0011:{
          if (read_reg(C) == 0){ trigger_interrupt(1); break; }
          write_reg(A, read_reg(B) / read_reg(C));
          break;
        }
      default:
          trigger_interrupt(1);
      }
      break;
    }
    case 0x6: /* logicke operacije */
    {    
      switch (mod){
      case 0b0000: write_reg(A, ~read_reg(B)); break;
      case 0b0001: write_reg(A, read_reg(B) & read_reg(C)); break;
      case 0b0010: write_reg(A, read_reg(B) | read_reg(C)); break;
      case 0b0011: write_reg(A, read_reg(B) ^ read_reg(C)); break;
      default:
          trigger_interrupt(1);
      }
      break;
    }

    case 0x7: /* pomerackle operacije */
    { 
      switch (mod){
      case 0b0000: write_reg(A, read_reg(B) << read_reg(C)); break;
      case 0b0001: write_reg(A, read_reg(B) >> read_reg(C)); break;
      default:
          trigger_interrupt(1);
      }
      break;
    }

      case 0x8: /* store */
    {    
      switch (mod){
        case 0b0000:
            memory.write_word(read_reg(A) + read_reg(B) + D, read_reg(C));
            break;
        case 0b0001: {
            uint32_t addr = read_reg(A) + D;
            write_reg(A, addr);
            memory.write_word(addr, read_reg(C));
            break;
        }
        case 0b0010:
            memory.write_word(memory.read_word(read_reg(A) + read_reg(B) + D), read_reg(C));
            break;
        default:
            trigger_interrupt(1);
        }
        break;
    }
    case 0x9:{ /* load */
      switch (mod){
      case 0b0000:
          write_reg(A, read_csr(B));
          break;
      case 0b0001:
          write_reg(A, read_reg(B) + D);
          break;
      case 0b0010:
          write_reg(A, memory.read_word(read_reg(B) + read_reg(C) + D));
          break;
      case 0b0011:
          write_reg(A, memory.read_word(read_reg(B)));
          write_reg(B, read_reg(B) + D);
          break;
      case 0b0100:
          write_csr(A, read_reg(B));
          break;
      case 0b0101:
          write_csr(A, read_csr(B) | D);
          break;
      case 0b0110:
          write_csr(A, memory.read_word(read_reg(B) + read_reg(C) + D));
          break;
      case 0b0111:
          write_csr(A, memory.read_word(read_reg(B)));
          write_reg(B, read_reg(B) + D);
          break;
      }
    }
    default:
      trigger_interrupt(1);
    }
  }