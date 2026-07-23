#pragma once
#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <fstream>
#include "structs.hpp"
#include <unordered_map>
#include <iomanip>
#include <algorithm>
using namespace std;

class Linker {
  
  vector<objectFile> objFiles;
  vector<Symbol> outputSymTable;
  vector<string> input_files;
  vector<placeEntry> placeTable; // gde krece koja sekcija i iz kog fajla 
                                //npr sekcija code iz fajla 1 na offsetu 0 sekcije code u output
                                //fajlu a iz fajla 1 na offsetu 15 npr itd...
  string output_file;
  unordered_map<string, Section> outputSections;
  vector<string> sectionOrder;
  unordered_map<string, uint32_t> place;
  bool isHex;
  bool relocatableMode;

  private:
    void read_obj_files();
    void join_sections();
    void map_sections();
    bool create_sym_table();
    bool realloc_symbols();
    bool check_section_overlap();
    string read_string(ifstream& in);
    uint32_t find_place(string sec , uint32_t id);
    Symbol* find_symbol(string name);
    void write_string(ofstream& out, const string& s);    
    bool write_relocatable_output();
    bool write_hex_output();
  public:
    Linker(vector<string> input, string output,unordered_map<string,uint32_t> placeArg,bool hexMode, bool relocMode)
        : input_files(input), output_file(output),
          place(placeArg), isHex(hexMode), relocatableMode(relocMode) {};

    bool link_print(); //pokrece ali ne zapisuje u fajl 
    bool link();  // pokrece ceo pipeline redom
    bool write_output_file();
    void print_symbol_table();
    void print_sections();
    void print_place_table();
    void print_relocations();
};