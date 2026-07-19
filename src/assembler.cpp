
#include "assembler.hpp"
vector<string> newsymbols;
bool externsymbols;
int Assembler::open_create_file(string Name) {
    current_file.open(Name);
    if (!current_file) {
        cout<<"Kreiranje i otvaranje file-a neuspesno\n";
        return 1;
    }
    return 0;
}


void Assembler::close_file() {
    if(!current_file)return;
    current_file.close();

}

/// INSTRUKCIJE SA SIMBOLIMA (TABELOM SIMBOLA)///

Assembler::Symbol* Assembler::find_symbol(string name){

    for(Symbol &s : symbolTable){
        if(s.name == name)
            return &s;
    }
    return nullptr;
}


int Assembler::add_symbol(string name) {

    if(find_symbol(name) != nullptr)
        return 1; // vec postoji

    Symbol sym;

    sym.name = name;
    sym.section = "";
    sym.base = 0;
    sym.size = 0;
    sym.defined = false;
    sym.global = false;
    sym.isextern = false;

    symbolTable.push_back(sym);
    return 0;
}


void Assembler::set_defined_symbol(string name){
    Symbol* s=find_symbol(name);
    if(s)
        s->defined=true;
}

void Assembler::set_externsymbol(string name) {
    Symbol* s;
    if((s = find_symbol(name))) {
        s->isextern = true;
    }
}

void Assembler::set_globalsymbol(string name){
    Symbol* s=find_symbol(name);
    if(s)
        s->global=true;
}

void Assembler::set_symbolsize(string name,uint32_t size){
    Symbol* s=find_symbol(name);
    if(s)
        s->size=size;
}

void Assembler::set_sectionsymbol(string name,string section){
    Symbol* s=find_symbol(name);
    if(s)
        s->section=section;
}

void Assembler::set_basesymbol(string name ,int32_t val){
    Symbol* s = find_symbol(name);
    if(s){
        s->base = val;
    }
}

void Assembler::resolvesymbols(){
    for (auto s : newsymbols){
        
        if(externsymbols == true){
            set_externsymbol(s);
        }
        else set_globalsymbol(s);
    }
    cout<<externsymbols << "\n";
    newsymbols.clear();
}



/// operacije sa section tabelom ///

int Assembler::add_section(string name){
    if(find_section(name))
        return 1;

    Section sec;

    sec.name=name;
    sec.size=0;
    sec.base=0;

    sectionTable.push_back(sec);
    return 0;
}

Assembler::Section* Assembler::find_section(string name){
    for(Section &s:sectionTable){
        if(s.name==name)
            return &s;
    }
    return nullptr;
}

void Assembler::set_section_size(string name, uint32_t size){
    Section* section = find_section(name);
    if(section)
        section->size = size;
}

void Assembler::increase_section_size(string name, uint32_t value){
    Section* section = find_section(name);
    if(section)
        section->size += value;
}

void Assembler::set_section_offset(string name,uint32_t offset)
{
    Section* section=find_section(name);
    if(section)
        section->fileOffset=offset;
}

uint32_t Assembler::get_section_size(string name){
    Section* section=find_section(name);
    if(section)
        return section->size;
    return 0;
}

uint64_t Assembler::get_section_offset(string name){
    Section* section=find_section(name);
    if(section)
        return section->fileOffset;
    return 0;
}


// pisanje bajta instrukcije 
void Assembler::write_byte(uint8_t b) {
    Section* sec = find_section(currentSection);
    if (!sec) return; 
    sec->data.push_back(b);
    locationCounter++;
}
// pisanje .word direktive 
void Assembler::write_word(uint32_t val) { 
    write_byte(val & 0xFF);
    write_byte((val >> 8) & 0xFF);
    write_byte((val >> 16) & 0xFF);
    write_byte((val >> 24) & 0xFF);
}


void Assembler::write_instruction(uint8_t oc, 
                                 uint8_t mod,
                                 uint8_t regA, 
                                 uint8_t regB, 
                                 uint8_t regC,
                                 int16_t disp) {
    write_byte((oc << 4) | (mod & 0xF));
    write_byte((regA << 4) | (regB & 0xF));
    write_byte((regC << 4) | ((disp >> 8) & 0xF));
    write_byte(disp & 0xFF);
}

/// Tabela obracanja unapred ///

int Assembler::add_forward_reference(string symbolName ,uint32_t address,string section,uint32_t size,forwardRefrence::Type type){
    forwardRefrence ref;
    ref.symbolName = symbolName;
    ref.address=address;
    ref.section=section;
    ref.size=size;
    ref.type=type;

    forwardRefrenceTable.push_back(ref);
    return 0; 
}






void Assembler::print_symbol_table(){

    cout<<"================== SYMBOL TABLE ====================\n";
    for(auto &s:symbolTable){

        cout
        <<s.name<<"\t\t "
        <<s.section<<"\t\t "
        <<"adr/val: "<<s.base<<"\t\t "
        <<"defined:"<<s.defined
        <<" \t\t global:"<<s.global
        <<" \t\t extern:"<<s.isextern
        <<endl;
    }
}

void Assembler::print_section_table(){

    cout<<"================== SECTION TABLE ==================\n";
    for(auto &s:sectionTable){

        cout
        <<s.name
        <<" \t\tsize:"
        <<s.size
        <<" \t\t base:"
        <<s.base
        <<endl;
    }

}

void Assembler::print_forward_reference_table(){

    cout<<"================== FORWARD REFERENCES ==================\n";
    for(auto &r:forwardRefrenceTable){

        cout
        <<"addr:"
        <<r.address
        <<" section:"
        <<r.section
        <<" size:"
        <<r.size
        <<endl;
    }
}

