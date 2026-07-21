#pragma once
#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
using namespace  std;
//pomocne stvari
struct OperandInfo {
    int mod;
    int regB;      /* bazni registar; -1 ako se ne koristi */
    int regC;      /*  r0 kad se ne koristi */
    int32_t disp;
    char* symbol;  /* nenulti ako disp jos nije poznat (forward ref) */
    bool isImmediateReg; /* true za %gpr  */
    int plainReg;
  };
extern vector<string> newsymbols;
extern bool externsymbols;
class Assembler {
public:
    struct Symbol;
    struct Section;
    struct forwardRefrence;
    struct PoolEntry; 
    string currentSection;
    uint32_t locationCounter = 0;
private:

    ofstream current_file;
    vector<Symbol> symbolTable;
    vector<Section> sectionTable;
    vector<PoolEntry> literalPool; // bazen literala
    unordered_map<string, vector<forwardRefrence>> forwardRefTable;
    Symbol* symbolTable_find_symbol(string name);
    Section* sectionTable_find_section(string name);
public:
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
        std::vector<uint8_t> data; // lista instrukcija sekcije koja se menja,
                                    //ona se prepisuje u output fajl nakon svega
    };
    struct forwardRefrence {
        uint32_t address;
        string section;
        uint32_t size;
        enum Type {
            ABSOLUTE,
            PC_RELATIVE
        } type;
    };
    struct PoolEntry {
        string poolKey;
        bool isSymbol;  //false za literale (direktno upisane vrednosti)
        int32_t literalValue;
        string symbolName;
    };

    int open_create_file(string Name);
    void output_file(string filename);
    void close_file();
    /* symbol table operations */
    int symbolTable_add_symbol(string name);
    void symbolTable_set_defined(string name);
    void symbolTable_set_global(string name);
    void symbolTable_set_extern(string name);
    void symbolTable_set_size(string name, uint32_t size);
    void symbolTable_set_base(string name, int32_t val);
    void symbolTable_set_section(string name, string section);
    /* section table funkcije */
    int sectionTable_add_section(string name);
    void sectionTable_set_size(string name, uint32_t size);
    void sectionTable_increase_size(string name, uint32_t value);
    void sectionTable_set_offset(string name, uint32_t offset);
    uint32_t sectionTable_get_size(string name);
    uint64_t sectionTable_get_offset(string name);
    void write_byte(uint8_t b);
    void write_word(uint32_t val);
    void write_instruction(uint8_t oc, uint8_t mod,uint8_t regA, uint8_t regB, uint8_t regC,int16_t disp);


    /* tabla obracanja unapred  funkcije */
    int forwardRefTable_add_reference(string symbolName , uint32_t address,string section,uint32_t size,forwardRefrence::Type type);
    
    /*Literal pool operations*/

    string literalPool_findOrAdd_PoolSlot(bool isSymbol, int32_t literalValue, const string& symbolName);
    void literalPool_flush();

    /* ispis tabela, za proveru */
    void symbolTable_print();
    void sectionTable_print();
    void forwardRefTable_print();
    void print_byte_code();
    void backpatching(const string& name);
    // resavanje novih simbola //
    void resolvesymbols();
    //index registra//
    int8_t gpr_index(string s){
       if(s == "%sp")return 14;
       else if (s == "%pc") return 15;
       if (s.size() == 3) {
            return s[2] - '0';
        }
        else {
            return (s[2] - '0') * 10 + (s[3] - '0');
        }
    }
    int8_t csr_index(string s){
      if (s=="%status") return 0;
      if (s== "%handler") return 1;
      if (s == "%cause") return 2;
      return -1;
    }
};