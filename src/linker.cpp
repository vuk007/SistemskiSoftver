
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

}
void Linker::create_sym_table(){}
void Linker::realloc_symbols(){}
    



int main (int argc , char** argv) {


  /*CITANJE ULAZA*/
  string output_file; 
  vector<string> input_files;
  bool hex = false;
  bool relocatable = false;  
  if(argc < 2)return -1;
   for (int i = 0; i < argc ; i++){

      string cla = argv[i];
      if ( cla == "-o" ){
        if( ++i > argc ) return -1;
        cla = argv[i];
        output_file = cla;
        cout<<"output_file:"<<output_file <<"\n";
      }
      else if (cla.rfind("-place") == 0 ){
        cla = cla.substr(7);
        int delim = cla.find('@');
        string section = cla.substr(0, delim);
        string address = cla.substr(delim+1);
        cout<<"section:"<<section<<" address:"<<address<<"\n";
      }
      else if (cla == "-hex"){
        hex = true;
        cout<<hex<<"\n";
      }
      else if (cla == "-relocatable"){
          relocatable = true;
          cout << relocatable << "\n"; 
      }
      else {
        if(cla.rfind(".o")){
          input_files.push_back(cla);
          cout<<"input_file:"<<cla<<"\n";
        }
      }
   }

   /* OBRADA LINKERA */
return 0; 

}