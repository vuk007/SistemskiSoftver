//
// Created by vuk00 on 7/18/2026.
//

#pragma once
#include <string>
#include <unordered_map>
#include "parser.hpp"
using namespace std;


struct InstructionCode {
    int oc;
    int mod;
};


inline unordered_map<yytokentype, InstructionCode> instruction_table = {
    {HALT,   {0b0000, 0b0000}},

    {INT,    {0b0001, 0b0000}},

    {CALL,   {0b0010, 0b0000}},   // push pc; pc<=gpr[A]+gpr[B]+D  (registarski oblik)

    {JMP,    {0b0011, 0b0000}},   // pc<=gpr[A]+D
    {BEQ,    {0b0011, 0b0001}},   // if (gpr[B]==gpr[C]) pc<=gpr[A]+D
    {BNE,    {0b0011, 0b0010}},   // if (gpr[B]!=gpr[C]) pc<=gpr[A]+D
    {BGT,    {0b0011, 0b0011}},   // if (gpr[B] signed> gpr[C]) pc<=gpr[A]+D

    {XCHG,   {0b0100, 0b0000}},

    {ADD,    {0b0101, 0b0000}},
    {SUB,    {0b0101, 0b0001}},
    {MUL,    {0b0101, 0b0010}},
    {DIV,    {0b0101, 0b0011}},

    {NOT,    {0b0110, 0b0000}},
    {AND,    {0b0110, 0b0001}},
    {OR,     {0b0110, 0b0010}},
    {XOR,    {0b0110, 0b0011}},

    {SHL,    {0b0111, 0b0000}},
    {SHR,    {0b0111, 0b0001}},

    {ST,     {0b1000, 0b0000}},
    {PUSH,   {0b1000, 0b0001}},   // gpr[A]<=gpr[A]+D; mem32[gpr[A]]<=gpr[C];  (A=sp, D=-4)

    {LD,     {0b1001, 0b0010}},   // gpr[A]<=mem32[gpr[B]+gpr[C]+D]  (memorijski oblik)
    {POP,    {0b1001, 0b0011}},   // gpr[A]<=mem32[gpr[B]]; gpr[B]<=gpr[B]+D;  (B=sp, D=4)
    {RET,    {0b1001, 0b0011}},   // isto kao pop, ali A=pc(15), B=sp(14), D=4


    {CSRRD,  {0b1001, 0b0000}},   // gpr[A]<=csr[B]
    {CSRWR,  {0b1001, 0b0100}},   // csr[A]<=gpr[B]
};

InstructionCode get_instruction_code(yytokentype i)
{
    return instruction_table[i];
}