#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <fstream>
#include "structs.hpp"
#include <unordered_map>
using namespace std;

vector<placeEntry> placeTable;

class Linker {

  vector<objectFile> objFiles;
  vector<string> input_files;
  string output_file;
  unordered_map<string, Section> outputSections;
  
  private:
    void read_obj_files();
    void join_sections();
    void map_sections();
    void create_sym_table();
    void realloc_symbols();
    string read_string(ifstream& in);

  public:
    Linker(vector<string> input , string output): input_files(input), output_file(output){};

};