#pragma once
#include <iostream>
#include <fstream>
#include <unordered_map>
#include "structs.hpp"
using namespace  std;
//pomocne stvari

extern vector<string> newsymbols;
extern bool externsymbols;
class Assembler {
public:
    string currentSection;
    uint32_t locationCounter = 0;
private:

    ofstream current_file;
    vector<Symbol> symbolTable;
    vector<Section> sectionTable;
    unordered_map<string, vector<PoolEntry>> literalPool;   // kljuc = ime sekcije
    int poolCounter = 0;                              
    unordered_map<string, vector<forwardRefrence>> forwardRefTable;
    Symbol* symbolTable_find_symbol(string name);
    Section* sectionTable_find_section(string name);
public:
    int open_create_file(string Name);
    void output_file(string filename);
    void close_file();
    void write_string(ofstream& out,const string& s);
    void write_symbols(ofstream& out);
    void write_sections(ofstream& out);
    void write_relocations(ofstream& out);
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

    int check_undefind_sym(){
        for (auto &s : symbolTable){
            if(s.defined != 1 && s.isextern != true){
                cout<< "simbol "<< s.name <<" nije definisan niti uvezen\n";
                return -1; 
            }
        }
        for (auto &r : forwardRefTable){
            auto sym  =symbolTable_find_symbol(r.first);
            if(sym == nullptr){
                cout<< "simbol "<< r.first <<" nije definisan niti uvezen\n";
                return -1; 
            }
            if( sym->defined == false && sym->isextern == false){
                cout<< "simbol "<< sym->name <<" nije definisan niti uvezen\n";
                return -1; 
            }
        }
        return 0;
    }
};