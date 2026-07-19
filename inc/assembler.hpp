#pragma once
#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

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
    string currentSection;
    uint32_t locationCounter = 0;
private:
    
    ofstream current_file;
    vector<Symbol> symbolTable;
    vector<Section> sectionTable;
    vector<forwardRefrence> forwardRefrenceTable;

    Symbol* find_symbol(string name);
    Section* find_section(string name);
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
        string symbolName;
        string section;
        uint32_t size;
        enum Type {
            ABSOLUTE,
            PC_RELATIVE
        } type;
    };

    int open_create_file(string Name);
    void close_file();

    /* symbol table operations */
    int add_symbol(string name);
    void set_defined_symbol(string name);
    void set_globalsymbol(string name);
    void set_externsymbol(string name);
    void set_symbolsize(string name, uint32_t size);
    void set_basesymbol(string name, int32_t val);
    void set_sectionsymbol(string name, string section);


    /* section table funkcije */
    int add_section(string name);
    void set_section_size(string name, uint32_t size);
    void increase_section_size(string name, uint32_t value);
    void set_section_offset(string name, uint32_t offset);
    uint32_t get_section_size(string name);
    uint64_t get_section_offset(string name);
    void write_byte(uint8_t b);
    void write_word(uint32_t val);
    void write_instruction(uint8_t oc, uint8_t mod,uint8_t regA, uint8_t regB, uint8_t regC,int16_t disp);
    /* tabla obracanja unapred  funkcije */
    int add_forward_reference(string symbolName , uint32_t address,string section,uint32_t size,forwardRefrence::Type type);

    /* ispis tabela, za proveru */
    void print_symbol_table();
    void print_section_table();
    void print_forward_reference_table();

    // resavanje novih simbola //
    void resolvesymbols();

    //index registra//
    int8_t gpr_index(string s){
       if(s == "%sp")return 14;
       else if (s == "%pc") return 15;
       if (s.size() == 4) {         
            return s[2] - '0';
        } 
        else {                      
            return (s[2] - '0') * 10 + (s[3] - '0');
        }
    }
    int8_t csr_index(string s){
      if (s=="status") return 0;
      if (s== "%handler") return 1;
      if (s == "%cause") return 2;
    }
};



