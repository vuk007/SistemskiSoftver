#include "assembler.hpp"
#include <iomanip>
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

void Assembler::output_file(string filename){
    ofstream out(filename,ios::binary);
    if(!out){
        cout<<"Fajl nije uspeo da se otvori \n";
        return;
    }

    FileHeader header;
    
    header.symCount = 0;
    for (auto &s : symbolTable)if (s.name.rfind("_pool_", 0) != 0) header.symCount++;
    
    header.secCount=sectionTable.size();
    header.relocCount=0;
    for(auto &r:forwardRefTable)
        header.relocCount+=r.second.size();

    out.write((char*)&header,sizeof(header));
    write_symbols(out);
    write_sections(out);
    write_relocations(out);
    out.close();
}

void Assembler::close_file() {
    if(!current_file)return;
    current_file.close();

}


void Assembler::write_string(ofstream& out,const string& s){
    uint32_t size=s.size();
    out.write((char*)&size,sizeof(size));
    out.write(s.c_str(),size);
}

void Assembler::write_symbols(ofstream& out){

    for(auto &sym:symbolTable){

        if (sym.name.rfind("_pool_", 0) == 0) continue;

        write_string(out,sym.name);
        write_string(out,sym.section);

        out.write((char*)&sym.base,sizeof(sym.base));
        out.write((char*)&sym.size,sizeof(sym.size));

        out.write((char*)&sym.defined,sizeof(sym.defined));
        out.write((char*)&sym.global,sizeof(sym.global));
        out.write((char*)&sym.isextern,sizeof(sym.isextern));
    }
}

void Assembler::write_sections(ofstream& out){
    // zajedno sa sekcijama upisujem i kod, ne moram da trazim offset vec
    // je kod odmah ispod imena i podataka za tu sekciju 
    for(auto &sec:sectionTable){
        write_string(out,sec.name);
        out.write((char*)&sec.size,sizeof(sec.size));
        out.write((char*)&sec.base,sizeof(sec.base));
        uint32_t dataSize=sec.data.size();
        out.write((char*)&dataSize,sizeof(dataSize));
        out.write((char*)sec.data.data(),dataSize );
    }
}

void Assembler::write_relocations(ofstream& out){

    for(auto &entry:forwardRefTable){
        for(auto &ref:entry.second){
            write_string(out,entry.first);
            write_string(out,ref.section);
            out.write((char*)&ref.address,sizeof(ref.address));
            out.write((char*)&ref.size,sizeof(ref.size));
            uint8_t type=ref.type;
            out.write((char*)&type,sizeof(type));
        }
    }
}
/// INSTRUKCIJE SA SIMBOLIMA (TABELOM SIMBOLA)///

Symbol* Assembler::symbolTable_find_symbol(string name){

    for(Symbol &s : symbolTable){
        if(s.name == name)
            return &s;
    }
    return nullptr;
}


int Assembler::symbolTable_add_symbol(string name) {

    if(symbolTable_find_symbol(name) != nullptr)
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


void Assembler::symbolTable_set_defined(string name){
    Symbol* s=symbolTable_find_symbol(name);
    if(s)
        s->defined=true;
}

void Assembler::symbolTable_set_extern(string name) {
    Symbol* s;
    if((s = symbolTable_find_symbol(name))) {
        s->isextern = true;
    }
}

void Assembler::symbolTable_set_global(string name){
    Symbol* s=symbolTable_find_symbol(name);
    if(s)
        s->global=true;
}

void Assembler::symbolTable_set_size(string name,uint32_t size){
    Symbol* s=symbolTable_find_symbol(name);
    if(s)
        s->size=size;
}

void Assembler::symbolTable_set_section(string name,string section){
    Symbol* s=symbolTable_find_symbol(name);
    if(s)
        s->section=section;
}

void Assembler::symbolTable_set_base(string name ,int32_t val){
    Symbol* s = symbolTable_find_symbol(name);
    if(s){
        s->base = val;
    }
}

void Assembler::resolvesymbols(){
    for (auto s : newsymbols){

        if(externsymbols == true){
            symbolTable_set_extern(s);
        }
        else symbolTable_set_global(s);
    }
    newsymbols.clear();
}



/// operacije sa section tabelom ///

int Assembler::sectionTable_add_section(string name){
    if(sectionTable_find_section(name))
        return 1;

    Section sec;

    sec.name=name;
    sec.size=0;
    sec.base=0;

    sectionTable.push_back(sec);
    return 0;
}

Section* Assembler::sectionTable_find_section(string name){
    for(Section &s:sectionTable){
        if(s.name==name)
            return &s;
    }
    return nullptr;
}

void Assembler::sectionTable_set_size(string name, uint32_t size){
    Section* section = sectionTable_find_section(name);
    if(section)
        section->size = size;
}

void Assembler::sectionTable_increase_size(string name, uint32_t value){
    Section* section = sectionTable_find_section(name);
    if(section)
        section->size += value;
}

void Assembler::sectionTable_set_offset(string name,uint32_t offset)
{
    Section* section=sectionTable_find_section(name);
    if(section)
        section->fileOffset=offset;
}

uint32_t Assembler::sectionTable_get_size(string name){
    Section* section=sectionTable_find_section(name);
    if(section)
        return section->size;
    return 0;
}

uint64_t Assembler::sectionTable_get_offset(string name){
    Section* section=sectionTable_find_section(name);
    if(section)
        return section->fileOffset;
    return 0;
}


// pisanje bajta instrukcije
void Assembler::write_byte(uint8_t b) {
    Section* sec = sectionTable_find_section(currentSection);
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

int Assembler::forwardRefTable_add_reference(string symbolName ,uint32_t address,string section,uint32_t size,forwardRefrence::Type type){
    forwardRefTable[symbolName].push_back({address, section, size, type});
    return 0;
}



void Assembler::backpatching(const string& name){
    auto it = forwardRefTable.find(name);
    if (it == forwardRefTable.end()) return;
    Symbol* sym = symbolTable_find_symbol(name);
    if (!sym || !sym->defined) return;

    auto &refs = it->second;

    for (auto refIt = refs.begin(); refIt != refs.end(); ) {
        forwardRefrence &ref = *refIt;
        if (ref.type == forwardRefrence::PC_RELATIVE && sym->section == ref.section) {
            Section* sec = sectionTable_find_section(ref.section);
            if (sec) {
                int32_t value = sym->base - (int32_t)(ref.address + ref.size);
                if (value < -2048 || value > 2047) {
                    cerr << "Greska: PC-relativni pomeraj do simbola '" << name
                         << "' ne staje u 12 bita (" << value << ")\n";
                }
                sec->data[ref.address + 2] = (sec->data[ref.address + 2] & 0xF0) | ((value >> 8) & 0xF);
                sec->data[ref.address + 3] = value & 0xFF;
            }
            refIt = refs.erase(refIt);
        }
        else {
            ++refIt;   // ABSOLUTE, ili PC_RELATIVE 
        }
    }

    if (refs.empty())
        forwardRefTable.erase(it);
}


/*literal pool */

string Assembler::literalPool_findOrAdd_PoolSlot(bool isSymbol, int32_t literalValue, const string& symbolName) {
    auto &pool = literalPool[currentSection];
    for (auto &e : pool) {
        if (e.isSymbol == isSymbol) {
            if (isSymbol && e.symbolName == symbolName) return e.poolKey;
            if (!isSymbol && e.literalValue == literalValue) return e.poolKey;
        }
    }
    
    string key = "_pool_" + to_string(poolCounter++);
    pool.push_back({key, isSymbol, literalValue, symbolName});
    return key;
}

void Assembler::literalPool_flush() {
    if (literalPool.empty()) return;

       for (auto &kv : literalPool) {
        const string &section = kv.first;
        for (auto &e : kv.second) {
            currentSection = section;
            locationCounter = sectionTable_get_size(section);

            symbolTable_add_symbol(e.poolKey);
            symbolTable_set_defined(e.poolKey);
            symbolTable_set_section(e.poolKey, section);
            symbolTable_set_base(e.poolKey, (int32_t)locationCounter);

            if (e.isSymbol) {
                forwardRefTable_add_reference(e.symbolName, locationCounter, section, 4,
                                               forwardRefrence::ABSOLUTE);
                write_word(0);
            } else {
                write_word((uint32_t)e.literalValue);
            }
            backpatching(e.poolKey);
            sectionTable_set_size(section, locationCounter);
        }
    }
    literalPool.clear();
}




void Assembler::symbolTable_print(){
    cout<<"================== SYMBOL TABLE ====================\n";
    for(auto &s:symbolTable){
        if (s.name.rfind("_pool_", 0) == 0) continue;   // preskoci interne pool zapise
        cout << s.name << "\t\t " << s.section << "\t\t "
             << "adr/val: " << s.base << "\t\t "
             << "defined:" << s.defined << " \t\t global:" << s.global
             << " \t\t extern:" << s.isextern << endl;
    }
}

void Assembler::sectionTable_print(){

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

void Assembler::forwardRefTable_print(){
    cout << "================== FORWARD REFERENCES ==================\n";
    for (auto &entry : forwardRefTable) {
        const string& symbolName = entry.first;
        for (auto &r : entry.second) {
            cout
            << "symbol:" << symbolName
            << " addr:" << r.address
            << " section:" << r.section
            << " size:" << r.size
            << " type:" << (r.type == forwardRefrence::PC_RELATIVE ? "PC_RELATIVE" : "ABSOLUTE")
            << endl;
        }
    }
}



void Assembler::print_byte_code()
{
    cout<<"================== CODE ==================\n";
    for (auto &sec : sectionTable)
    {
        cout << "Section: " << sec.name << endl;
        int offset = 0;
        for (auto &code : sec.data)
        {
            if (offset % 16 == 0)
            {
                cout << endl;
                cout << hex << setw(4) << setfill('0') << offset << ": ";
            }

            cout << hex
                 << setw(2)
                 << setfill('0')
                 << (int)code << " ";

            offset++;
        }
        cout << endl << endl;
    }
    cout << dec;
}