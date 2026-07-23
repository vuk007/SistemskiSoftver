
#include "linker.hpp"

string Linker::read_string(ifstream& in)
  {
      uint32_t size;
      in.read((char*)&size,sizeof(size));
      string s(size,'\0');
      in.read(&s[0],size);
      return s;
  }



void Linker::read_obj_files(){
    for(string file:input_files){
      objectFile obj;
      ifstream in(file,ios::binary);
      
      if(!in){
        cout<<"Nije moguce otvoriti file ili ne postoji: "<<file<<endl;
        return;
      }
      
      FileHeader header;
      in.read((char*)&header, sizeof(header));

      for(uint32_t i=0;i<header.symCount;i++){
          Symbol sym;
          
          sym.name=read_string(in);
          sym.section=read_string(in);
          in.read((char*)&sym.base,sizeof(sym.base));
          in.read((char*)&sym.size,sizeof(sym.size));
          in.read((char*)&sym.defined,sizeof(sym.defined));
          in.read((char*)&sym.global,sizeof(sym.global));
          in.read((char*)&sym.isextern,sizeof(sym.isextern));
          
          obj.symbols.push_back(sym);
      }
      for(uint32_t i=0;i<header.secCount;i++){
          
          Section sec;
         
          sec.name=read_string(in);
          in.read((char*)&sec.size, sizeof(sec.size));
          in.read((char*)&sec.base, sizeof(sec.base));
          uint32_t dataSize;
          in.read((char*)&dataSize, sizeof(dataSize));
          sec.data.resize(dataSize);
          in.read((char*)sec.data.data(), dataSize);
          obj.sections.push_back(sec);
      }
      for(uint32_t i=0;i<header.relocCount;i++){
          Relocation rel;
          
          rel.symbol=read_string(in);
          rel.section=read_string(in);
          in.read((char*)&rel.address,sizeof(rel.address));
          in.read((char*)&rel.size,sizeof(rel.size));
          in.read((char*)&rel.type,sizeof(rel.type));
          obj.relocations.push_back(rel);
      }
      in.close();
      objFiles.push_back(obj);
    } 
}

void Linker::join_sections(){
  uint32_t fileId = 0; 
  for (auto &obj:objFiles){
    for (auto &sec:obj.sections){
        auto it = outputSections.find(sec.name);
        if(it == outputSections.end()){
              placeTable.push_back({fileId,sec.name,0});
              outputSections[sec.name]=sec;
              sectionOrder.push_back(sec.name);
        }
        else {
                uint32_t offset=it->second.data.size();
                placeTable.push_back({fileId,sec.name,offset});
                it->second.data.insert(it->second.data.end(),sec.data.begin(),sec.data.end());
                it->second.size += sec.size;
        }
    }
    fileId++;
  }
}

void Linker::map_sections(){
    uint32_t current = 0;

    for (auto &kv : place){
        if (outputSections.find(kv.first) == outputSections.end()){
            cout << "Greska: -place navodi nepostojecu sekciju '" << kv.first << "'\n";
            continue;
        }
        outputSections[kv.first].base = kv.second;
        uint32_t end = kv.second + outputSections[kv.first].size;
        if (end > current) current = end;
    }

    for (auto &name : sectionOrder){
        if (place.find(name) == place.end()){
            outputSections[name].base = current;
            current += outputSections[name].size;
        }
    }
}
bool Linker::check_section_overlap(){
   vector<pair<string,Section*>> sections;

    for(auto &it:outputSections)
        sections.push_back({it.first,&it.second});

    for(long unsigned int i=0;i<sections.size();i++){
        uint32_t start1=sections[i].second->base;
        uint32_t end1=start1+sections[i].second->size;

        for(long unsigned int j=i+1;j<sections.size();j++){
            uint32_t start2=sections[j].second->base;
            uint32_t end2=start2+sections[j].second->size;

            if(start1<end2 && start2<end1){
                cout<<"Greska: sekcije "<<sections[i].first<<" i "<<sections[j].first<<" se preklapaju\n";
                cout<<hex<<"Prva: "<<start1<<" - "<<end1<<endl;
                cout<<hex<<"Druga: "<<start2<<" - "<<end2<<endl;
                return false;
            }
        }
    }

    return true;
}

Symbol* Linker::find_symbol(string name){

    for(Symbol &s : outputSymTable){
        if(s.name == name)
            return &s;
    }
    return nullptr;
}
uint32_t Linker::find_place(string sec , uint32_t id){
  for(auto p : placeTable){
    if(p.section == sec && p.fileId == id)return p.address;
  }
  return 0; 
}

bool Linker::create_sym_table(){
    uint32_t id = 0;
    bool ok = true;

    for (auto &file : objFiles){
        for (auto &sym : file.symbols){

            Symbol* existing = find_symbol(sym.name);
            
            if (existing == nullptr){
                outputSymTable.push_back(sym);
                existing = find_symbol(sym.name);

                if (sym.defined){
                    existing->base += outputSections[sym.section].base
                                     + find_place(sym.section, id);
                    existing->defined = true;
                }
            }
            else {
                if (sym.defined && existing->defined){
                    cout << "Greska: simbol '" << sym.name
                         << "' je visestruko definisan\n";
                    ok = false;
                    continue;
                }
                if (sym.defined && !existing->defined){
                    // ranije je bio samo extern najava, sad ga popunjavamo
                    existing->section = sym.section;
                    existing->base = sym.base
                                    + outputSections[sym.section].base
                                    + find_place(sym.section, id);
                    existing->defined = true;
                }
            }
        }
        id++;
    }
    return ok;
}


bool Linker::realloc_symbols(){
    uint32_t fileId = 0;
    bool ok = true;

    for (auto &file : objFiles){
        for (auto &rel : file.relocations){

            Symbol* sym = find_symbol(rel.symbol);
            if (sym == nullptr || !sym->defined){
                cout << "Greska: nerazresen simbol '" << rel.symbol
                     << "' (koriscen u sekciji " << rel.section << ")\n";
                ok = false;
                continue;
            }

            uint32_t placeOffset = find_place(rel.section, fileId);
            uint32_t patchAddr = placeOffset + rel.address;
            Section &sec = outputSections[rel.section];

            if (rel.type == 0 /* ABSOLUTE */){
                
              uint32_t value = (uint32_t)sym->base;
                sec.data[patchAddr + 0] = value & 0xFF;
                sec.data[patchAddr + 1] = (value >> 8) & 0xFF;
                sec.data[patchAddr + 2] = (value >> 16) & 0xFF;
                sec.data[patchAddr + 3] = (value >> 24) & 0xFF;
           
              } 
              else if (rel.type == 1) { /* PC_RELATIVE */
                
                int32_t value = sym->base - (int32_t)(sec.base + patchAddr + rel.size);
                
                if (value < -2048 || value > 2047){
                    cout << "Greska: PC-relativni pomeraj do simbola '" << rel.symbol
                         << "' ne staje u 12 bita (" << value << ")\n";
                    ok = false;
                    continue;
                }
                sec.data[patchAddr + 2] = (sec.data[patchAddr + 2] & 0xF0) | ((value >> 8) & 0xF);
                sec.data[patchAddr + 3] = value & 0xFF;
            }
            else{

                /*        DISP_ABS            */
                int32_t value = sym->base;
                if (value < -2048 || value > 2047){
                    cout << "Greska: PC-relativni pomeraj do simbola '" << rel.symbol
                         << "' ne staje u 12 bita (" << value << ")\n";
                    ok = false;
                    continue;
                }
                sec.data[patchAddr+2] = (sec.data[patchAddr + 2 ] & 0xF0)| ((value >> 8)& 0x0F);
                sec.data[patchAddr + 3] = value & 0xFF;
            } 
        }
        fileId++;
    }
    return ok;
}
    
/* PRINT ZA DEBUG */

void Linker::print_symbol_table(){
    cout << "================== LINKER SYMBOL TABLE ==================\n";
    for (auto &s : outputSymTable){
        cout << s.name << "\t\t" << s.section
             << "\t\tadr: 0x" << hex << s.base << dec
             << "\t\tdefined:" << s.defined
             << "\tglobal:" << s.global
             << "\textern:" << s.isextern
             << endl;
    }
}

void Linker::print_sections(){
    cout << "================== LINKER SECTIONS ==================\n";
    for (auto &name : sectionOrder){
        Section &s = outputSections[name];
        cout << s.name << "\t\tbase: 0x" << hex << s.base
             << "\t\tsize: 0x" << s.size << dec << endl;

        int offset = 0;
        for (auto b : s.data){
            if (offset % 16 == 0) cout << "\n" << hex << setw(4) << setfill('0') << offset << ": ";
            cout << hex << setw(2) << setfill('0') << (int)b << " ";
            offset++;
        }
        cout << dec << "\n\n";
    }
}

void Linker::print_place_table(){
    cout << "================== PLACE TABLE ==================\n";
    for (auto &p : placeTable){
        cout << "fileId:" << p.fileId
             << "\tsection:" << p.section
             << "\toffset: 0x" << hex << p.address << dec << endl;
    }
}

void Linker::print_relocations(){
    cout << "================== RELOCATIONS (ulazni fajlovi) ==================\n";
    uint32_t fileId = 0;
    for (auto &file : objFiles){
        for (auto &r : file.relocations){
            cout << "fileId:" << fileId
                 << "\tsymbol:" << r.symbol
                 << "\tsection:" << r.section
                 << "\taddress: 0x" << hex << r.address << dec
                 << "\tsize:" << r.size
                 << "\ttype:";
                if(r.type == 0)
                    cout<<"ABSOLUTE";
                else if(r.type == 1)
                    cout<<"PC_RELATIVE";
                else
                    cout<<"DISP_ABS";
                cout<<endl;
        }
        fileId++;
    }
}

/* DEBUG MODE */
bool Linker::link_print(){
    read_obj_files();
    join_sections();

    if (relocatableMode) place.clear();  // -relocatable u potpunosti ignorise -place
    map_sections();

    cout << "\n--- posle map_sections ---\n";
    print_sections();
    print_place_table();
    print_relocations();

    if (!create_sym_table()){
        cout << "Povezivanje neuspesno zbog greske u tabeli simbola.\n";
        return false;
    }

    cout << "\n--- posle create_sym_table ---\n";
    print_symbol_table();

    if (!relocatableMode){
        if (!realloc_symbols()){
            cout << "Povezivanje neuspesno zbog nerazresenih referenci.\n";
            return false;
        }
        cout << "\n--- posle realloc_symbols ---\n";
        print_sections();

        if (!check_section_overlap()){
            cout << "Povezivanje neuspesno zbog preklapanja sekcija.\n";
            return false;
        }
    }
    return true;
}

/* LINK */
bool Linker::link(){
    read_obj_files();
    join_sections();
 
    if (relocatableMode) place.clear();
    map_sections();
 
    cout << "\n--- posle map_sections ---\n";
    print_sections();
    print_place_table();
    print_relocations();
 
    if (!create_sym_table()){
        cout << "Povezivanje neuspesno zbog greske u tabeli simbola.\n";
        return false;
    }
 
    cout << "\n--- posle create_sym_table ---\n";
    print_symbol_table();
 
    if (!relocatableMode){
        if (!realloc_symbols()){
            cout << "Povezivanje neuspesno zbog nerazresenih referenci.\n";
            return false;
        }
        cout << "\n--- posle realloc_symbols ---\n";
        print_sections();
 
        if (!check_section_overlap()){
            cout << "Povezivanje neuspesno zbog preklapanja sekcija.\n";
            return false;
        }
    }
 
    if (!write_output_file()){
        cout << "Povezivanje neuspesno - greska pri upisu izlaznog fajla.\n";
        return false;
    }
 
    cout << "\nPovezivanje uspesno.\n";
    return true;
}

/* Upis u fajl naziv symbola, sekcije ( sve sto je string, kao u assembleru )*/
void Linker::write_string(ofstream& out, const string& s){
    uint32_t size = (uint32_t)s.size();
    out.write((char*)&size, sizeof(size));
    out.write(s.c_str(), size);
}


bool Linker::write_output_file(){
    if (isHex) return write_hex_output();
    else return write_relocatable_output();
}
 

/* -hex fajl, koji moze da se izvrsi u emulatoru pair <adresa , byte>*/
bool Linker::write_hex_output(){
    ofstream out(output_file);
    if (!out.is_open()){
        cout << "Ne mogu da otvorim izlazni fajl: " << output_file << "\n";
        return false;
    }

    vector<string> ordered = sectionOrder;
    sort(ordered.begin(), ordered.end(), [this](const string& a, const string& b){
        return outputSections[a].base < outputSections[b].base;
    });

    bool first = true;
    uint32_t lastAddr = 0; // adresa poslednjeg upisanog bajta
                            // provera poravnanja 

    for (auto &name : ordered){
        Section &sec = outputSections[name];
        for (size_t i = 0; i < sec.data.size(); i++){
            uint32_t addr = sec.base + (uint32_t)i;
            
            // da li treba poravnanje
            if (first || (addr % 8 == 0) || (addr != lastAddr + 1)){
                if (!first) out << "\n";
                out << hex << uppercase << setw(4) << setfill('0') << addr << ": " << dec << nouppercase;
                first = false;
            }
            out << hex << uppercase << setw(2) << setfill('0') << (int)sec.data[i] << " " << dec << nouppercase;
            lastAddr = addr;
        }
    }
    out << "\n";
    out.close();
    return true;
}


/* -relocatable resava sve sto moze da se spoji, pravi opet .o fajl i 
    sve simbole koji nisu reseni samo prenese .defined = false; prenosi dalje ne izbacuje gresku */
    
bool Linker::write_relocatable_output(){
    vector<Relocation> remainingRelocs;
    uint32_t fileId = 0;
 
    for (auto &file : objFiles){
        for (auto &rel : file.relocations){
            Symbol* sym = find_symbol(rel.symbol);
            uint32_t placeOffset = find_place(rel.section, fileId);
            uint32_t newAddr = placeOffset + rel.address;
 
            if (sym == nullptr || !sym->defined){
                Relocation r = rel;
                r.address = newAddr;         // azurirana adresa unutar SPOJENE sekcije
                remainingRelocs.push_back(r);
                continue;
            }
 
        
            Section &sec = outputSections[rel.section];
            if (rel.type == 0 ){
                
                /* ABSOLUTE */
                uint32_t value = (uint32_t)sym->base;
                sec.data[newAddr + 0] = value & 0xFF;
                sec.data[newAddr + 1] = (value >> 8) & 0xFF;
                sec.data[newAddr + 2] = (value >> 16) & 0xFF;
                sec.data[newAddr + 3] = (value >> 24) & 0xFF;
            } else if(rel.type == 1){ 
               
                /* PC_RELATIVE */
                int32_t value = sym->base - (int32_t)(sec.base + newAddr + rel.size);
                sec.data[newAddr + 2] = (sec.data[newAddr + 2] & 0xF0) | ((value >> 8) & 0xF);
                sec.data[newAddr + 3] = value & 0xFF;
            }
            else {
                int32_t value = sym->base;
                sec.data[newAddr + 2] = (sec.data[newAddr + 2 ] & 0xF0)| ((value >> 8)& 0x0F);
                sec.data[newAddr + 3] = value & 0xFF;
            }
        }
        fileId++;
    }
 
    ofstream out(output_file, ios::binary);
    if (!out.is_open()){
        cout << "Ne mogu da otvorim izlazni fajl: " << output_file << "\n";
        return false;
    }
    

    /* UPIS U FAJLOVE */
    
    FileHeader header;
    header.symCount = (uint32_t)outputSymTable.size();
    header.secCount = (uint32_t)sectionOrder.size();
    header.relocCount = (uint32_t)remainingRelocs.size();
    out.write((char*)&header, sizeof(header));
 
    for (auto &s : outputSymTable){
        write_string(out, s.name);
        write_string(out, s.section);
        out.write((char*)&s.base, sizeof(s.base));
        out.write((char*)&s.size, sizeof(s.size));
        out.write((char*)&s.defined, sizeof(s.defined));
        out.write((char*)&s.global, sizeof(s.global));
        out.write((char*)&s.isextern, sizeof(s.isextern));
    }
 
    for (auto &name : sectionOrder){
        Section &sec = outputSections[name];
        write_string(out, sec.name);
        out.write((char*)&sec.size, sizeof(sec.size));
        out.write((char*)&sec.base, sizeof(sec.base));
        uint32_t dataSize = (uint32_t)sec.data.size();
        out.write((char*)&dataSize, sizeof(dataSize));
        out.write((char*)sec.data.data(), dataSize);
    }
 
    for (auto &r : remainingRelocs){
        write_string(out, r.symbol);
        write_string(out, r.section);
        out.write((char*)&r.address, sizeof(r.address));
        out.write((char*)&r.size, sizeof(r.size));
        out.write((char*)&r.type, sizeof(r.type));
    }
 
    out.close();
    return true;
}
 

int main(int argc, char** argv){

    string outputFileName;
    vector<string> inputFiles;
    unordered_map<string,uint32_t> place;
    bool isHex = false;
    bool relocatable = false;

    if (argc < 2){
        cout << "Greska: nema ulaznih argumenata\n";
        return -1;
    }

    for (int i = 1; i < argc; i++){
        string cla = argv[i];

        if (cla == "-o"){
            if (++i >= argc){
                cout << "Greska: -o zahteva parametar\n";
                return -1;
            }
            outputFileName = argv[i];
        }
        else if (cla.rfind("-place", 0) == 0){
            cla = cla.substr(7); // preskace "-place="
            int delim = cla.find('@');
            if (delim == (int)string::npos){
                cout << "Greska: neispravan -place format: " << argv[i] << "\n";
                return -1;
            }
            string section = cla.substr(0, delim);
            string address = cla.substr(delim + 1);
            place[section] = stoul(address, nullptr, 0);
        }
        else if (cla == "-hex"){
            isHex = true;
        }
        else if (cla == "-relocatable"){
            relocatable = true;
        }
        else if (cla.size() >= 2 && cla.substr(cla.size() - 2) == ".o"){
            inputFiles.push_back(cla);
        }
        else {
            cout << "Greska: nepoznat argument '" << cla << "'\n";
            return -1;
        }
    }

    if (isHex == relocatable){
        cout << "Greska: mora se navesti tacno jedna od opcija -hex ili -relocatable\n";
        return -1;
    }

    if (inputFiles.empty()){
        cout << "Greska: nema ulaznih .o fajlova\n";
        return -1;
    }

    Linker linker(inputFiles, outputFileName, place, isHex, relocatable);

    if (!linker.link())
        return -1;

    return 0;
}