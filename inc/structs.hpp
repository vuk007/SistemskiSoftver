#pragma once
#include <string>
#include <cstdint>
#include <vector>
using namespace std;
   // polje symbol tabele
struct Symbol {
    string name;
    string section;
    int32_t base; //za .equ predstavlja vrednost izraza
                  // za sve ostalo adresu
    uint32_t size;
    bool defined;
    bool global;
    bool isextern;
};
struct Section {
    string name;
    uint32_t size;
    uint32_t base;
    uint64_t fileOffset;
    vector<uint8_t> data; // lista instrukcija sekcije koja se menja,
                                //ona se prepisuje u output fajl nakon svega
};
struct forwardRefrence {
    uint32_t address;
    string section;
    uint32_t size;
    enum Type {
        ABSOLUTE, // 0
        PC_RELATIVE, // 1
        DISP_ABS // 2
    } type;
};
struct PoolEntry {
    string poolKey;
    bool isSymbol;  //false za literale (direktno upisane vrednosti)
    int32_t literalValue;
    string symbolName;
};
struct OperandInfo {
    int mod;
    int regB;      /* bazni registar; -1 ako se ne koristi */
    int regC;      /*  r0 kad se ne koristi */
    int32_t disp;
    char* symbol;  /* nenulti ako disp jos nije poznat (forward ref) */
    bool isImmediateReg; /* true za %gpr  */
    int plainReg;
    bool isIndirect = false;
    enum RelocKind { NONE, POOL_PCREL, DISP_ABS } relocKind = NONE;
  };

struct FileHeader{
    uint32_t symCount;
    uint32_t secCount;
    uint32_t relocCount;
};

struct placeEntry {
  uint32_t fileId; 
  string section;
  uint32_t address;
};

struct Relocation  //assembler ne koristi zapravo ovo jer je uvedeno tek kod linkera
{
    string symbol;
    string section;
    uint32_t address;
    uint32_t size;
    uint8_t type; 
 };

struct objectFile{
    vector<Symbol> symbols;
    vector<Section> sections;
    vector<Relocation> relocations;
};