%{
/* C i C++ kod */
  #include <iostream>
  #include <cstring>
  #include "assembler.hpp"
  #include "Instructions.hpp"
  using namespace std;
  void yyerror(const char *s);
  extern int yylex(); 
  extern int line;
  Assembler ass; 
  extern vector<string> newsymbols;
  extern bool externsymbols;
  
%}

%code requires{#include "assembler.hpp"} // za samu strukturu OperandInfo

%union {
  char* str;
  long val;
  OperandInfo* opinfo;
}

 /* direktive */
%token GLOBAL EXTERN SECTION WORD SKIP ASCII
%token EQU END 

 /* instrukcije */
%token HALT INT IRET CALL RET JMP
%token BEQ BNE BGT
%token PUSH POP XCHG
%token ADD SUB MUL DIV NOT AND OR XOR SHL SHR
%token LD ST
%token CSRRD CSRWR  


/* ADRESIRANJA I CONSTANTE*/

%token COLON LBRACKET RBRACKET PLUS  COMMA DOLLAR NEWLINE

/* Nazivi simobla i vrednosti broja*/

%token <str> SYMBOL GPR  CSR STRING
%token <val> NUMBER HEX

%type <str> symbol name
%type <val> number expr 

%type <opinfo> operand jmp_operand

 /* levo asocijativne operacije */


%left PLUS '-'
%left '*' '/'

%%
program
      :    
      |
      program line  
      ;


line:
    NEWLINE
    |
    label statement NEWLINE
    |
    statement NEWLINE
    |
    label NEWLINE
    ;

label: 
      symbol colon  { 
        ass.symbolTable_add_symbol($1);
        ass.symbolTable_set_defined($1);
        ass.symbolTable_set_section($1,ass.currentSection);
        ass.symbolTable_set_base($1 , ass.locationCounter);
        ass.backpatching($1);
        free($1); }
      ;

symbol:
      SYMBOL  { $$ = $1; }
      ;

number: 
      NUMBER   { $$ = $1; }
      ;  

statement:
        instruction 
        |
        directive    
        ;




 /* DIREKTIVE */
directive: 
    GLOBAL symbol_list        { 
                                externsymbols = false;
                                ass.resolvesymbols();
                              }
    | 
    EXTERN symbol_list          { 
                                  externsymbols = true;
                                  ass.resolvesymbols();
                                }
    | 
    SECTION name            {  
                              if (!ass.currentSection.empty()){
                                ass.sectionTable_set_size(ass.currentSection, ass.locationCounter);
                              }
                              ass.sectionTable_add_section($2);
                              ass.currentSection = $2;
                              ass.locationCounter = ass.sectionTable_get_size($2);
                              free($2); }
    | 
    WORD init_list            
    | 
    SKIP number                { for(long i = 0 ; i < $2 ; i++)ass.write_byte(0); }
    | 
    ASCII STRING               { for(ulong i = 1 ; i + 1 < strlen($2); i++)
                                                  ass.write_byte((uint8_t)$2[i]);/*preskacem navodnike*/  }
    | 
    EQU symbol COMMA expr      { free($2); } //TODO
    | 
    END                        { 
                                if(!ass.currentSection.empty()){
                                  ass.sectionTable_increase_size(ass.currentSection, ass.locationCounter);
                                }
                                ass.literalPool_flush();
                                ass.symbolTable_print();
                                ass.sectionTable_print();
                                ass.forwardRefTable_print();
                              }
    ;


symbol_list:
            symbol   {
              ass.symbolTable_add_symbol($1);
              newsymbols.push_back($1);
              free($1);
              }
            |
            symbol_list COMMA symbol   { 
              ass.symbolTable_add_symbol($3);
              newsymbols.push_back($3);
              free($3); 
              }
            ;

name: 
    symbol  { $$ = $1; }
    ;


init_list:
          init_list COMMA init    
          |
          init
          ;

init:
    symbol    {         
                /* vrednost simbola jos mozda nije poznata - registruj forward ref
                na adresu koju upravo pisemo, pa upisi 0 kao placeholder */
                ass.forwardRefTable_add_reference($1,ass.locationCounter, ass.currentSection, 4,
                                            Assembler::forwardRefrence::ABSOLUTE);
                ass.write_word(0);
                free($1);
      }
    |
    number    { ass.write_word((uint32_t)$1); }
    ;

 /* izostavljen za B nivo, treba dodatno razraditi ako nisu poznati simboli*/
expr: 
    number    { $$ = 0; }
    |
    symbol    { free($1); $$ = 0; /*OVO TREBA RAZRADITI ZA .EQU*/}
    |
    expr PLUS expr    { $$ = 0; }
    | 
    expr '-' expr     { $$ = 0; }
    | 
    expr '*' expr     { $$ = 0; }
    | 
    expr '/' expr     { $$ = 0; }
    |
     '(' expr ')'     { $$ = 0; }
    ;


 /* instrukcije */
instruction:
    HALT  { 
            InstructionCode c = get_instruction_code(HALT);
            ass.write_instruction(c.oc, c.mod, 0, 0, 0, 0);
          }
    | 
    INT { 
        InstructionCode c = get_instruction_code(INT);
        ass.write_instruction(c.oc, c.mod, 0, 0, 0, 0); 
    }
    | 
    IRET { 
        ass.write_instruction(0b1001, 0b0011, 15, 14, 0, 4);
        ass.write_instruction(0b1001, 0b0111, 0, 14, 0, 4);
    }                                   
    | 
    RET     {ass.write_instruction(0b1001, 0b0011, 15, 14, 0, 4);}                               
    |
    CALL jmp_operand    { 
      
        InstructionCode c = get_instruction_code(CALL);
        int mod = $2->isIndirect ? 0b0001 : c.mod; 
        if ($2->symbol) {
            ass.forwardRefTable_add_reference($2->symbol, ass.locationCounter, ass.currentSection, 4,
                                       Assembler::forwardRefrence::PC_RELATIVE);
            free($2->symbol);
        }
        ass.write_instruction(c.oc, mod, 15, $2->regB, $2->regC, $2->disp);
        delete $2;
    }
    | 
    JMP jmp_operand           { 
      
       InstructionCode c = get_instruction_code(JMP);
       int mod = $2->isIndirect ? 0b1000 : c.mod;
        if ($2->symbol) {
            ass.forwardRefTable_add_reference($2->symbol,ass.locationCounter, ass.currentSection, 4,
                                       Assembler::forwardRefrence::PC_RELATIVE);
            free($2->symbol);
        }
        ass.write_instruction(c.oc, mod, 15, $2->regB, $2->regC, $2->disp);
        delete $2;
    }
    | 
    BEQ GPR COMMA GPR COMMA jmp_operand {
        InstructionCode c = get_instruction_code(BEQ);
        int mod = $6->isIndirect ? 0b1001 : c.mod;
        if ($6->symbol) {
            ass.forwardRefTable_add_reference($6->symbol, ass.locationCounter, ass.currentSection, 4,
                                       Assembler::forwardRefrence::PC_RELATIVE);
            free($6->symbol);
        }
        /* jmp-format, MOD 0001 : if (gpr[B]==gpr[C]) pc<=gpr[A]+D; A=pc */
        ass.write_instruction(c.oc, mod, 15, ass.gpr_index($2), ass.gpr_index($4), $6->disp);
        free($2); free($4); delete $6;
    }
    |
    BNE GPR COMMA GPR COMMA jmp_operand {
        InstructionCode c = get_instruction_code(BNE);
        int mod = $6->isIndirect ? 0b1010 : c.mod;
        if ($6->symbol) {
            ass.forwardRefTable_add_reference($6->symbol , ass.locationCounter, ass.currentSection, 4,
                                       Assembler::forwardRefrence::PC_RELATIVE);
            free($6->symbol);
        }
        ass.write_instruction(c.oc,mod, 15, ass.gpr_index($2), ass.gpr_index($4), $6->disp);
        free($2); free($4); delete $6;
    }
    |
    BGT GPR COMMA GPR COMMA jmp_operand {
        int mod = $6->isIndirect ? 0b1011 : 0b0011;
        if ($6->symbol) {
            ass.forwardRefTable_add_reference($6->symbol, ass.locationCounter, ass.currentSection, 4,
                                       Assembler::forwardRefrence::PC_RELATIVE);
            free($6->symbol);
        }
        ass.write_instruction(0b0011, mod, 15, ass.gpr_index($2), ass.gpr_index($4), $6->disp);
        free($2); free($4); delete $6;
    }
    |
    PUSH GPR {
        /* sp<=sp-4; mem32[sp]<=gpr;  ->  store format, mod 0001, A=sp, D=-4, C=gpr */
        ass.write_instruction(0b1000, 0b0001, 14, 0, ass.gpr_index($2), -4);
        free($2);
    }
    |
    POP GPR {
        /* gpr<=mem32[sp]; sp<=sp+4;  -> load format, mod 0011, A=gpr, B=sp, D=4 */
        ass.write_instruction(0b1001, 0b0011, ass.gpr_index($2), 14, 0, 4);
        free($2);
    }
    |
    NOT GPR {
        InstructionCode c = get_instruction_code(NOT);
        int r = ass.gpr_index($2);
        ass.write_instruction(c.oc, c.mod, r, r, 0, 0);
        free($2);
    }
    |
    XCHG GPR COMMA GPR {
        InstructionCode c = get_instruction_code(XCHG);
        ass.write_instruction(c.oc, c.mod, 0, ass.gpr_index($2), ass.gpr_index($4), 0);
        free($2); free($4);
    }
    |
    ADD GPR COMMA GPR {
        InstructionCode c = get_instruction_code(ADD);
        int d = ass.gpr_index($4);
        ass.write_instruction(c.oc, c.mod, d, d, ass.gpr_index($2), 0);
        free($2); free($4);
    }
    |
    SUB GPR COMMA GPR {
        InstructionCode c = get_instruction_code(SUB);
        int d = ass.gpr_index($4);
        ass.write_instruction(c.oc, c.mod, d, d, ass.gpr_index($2), 0);
        free($2); free($4);
    }
    |
    MUL GPR COMMA GPR {
        InstructionCode c = get_instruction_code(MUL);
        int d = ass.gpr_index($4);
        ass.write_instruction(c.oc, c.mod, d, d, ass.gpr_index($2), 0);
        free($2); free($4);
    }
    |
    DIV GPR COMMA GPR {
        InstructionCode c = get_instruction_code(DIV);
        int d = ass.gpr_index($4);
        ass.write_instruction(c.oc, c.mod, d, d, ass.gpr_index($2), 0);
        free($2); free($4);
    }
    |
    AND GPR COMMA GPR {
        InstructionCode c = get_instruction_code(AND);
        int d = ass.gpr_index($4);
        ass.write_instruction(c.oc, c.mod, d, d, ass.gpr_index($2), 0);
        free($2); free($4);
    }
    |
    OR GPR COMMA GPR {
        InstructionCode c = get_instruction_code(OR);
        int d = ass.gpr_index($4);
        ass.write_instruction(c.oc, c.mod, d, d, ass.gpr_index($2), 0);
        free($2); free($4);
    }
    |
    XOR GPR COMMA GPR {
        InstructionCode c = get_instruction_code(XOR);
        int d = ass.gpr_index($4);
        ass.write_instruction(c.oc, c.mod, d, d, ass.gpr_index($2), 0);
        free($2); free($4);
    }
    |
    SHL GPR COMMA GPR {
        InstructionCode c = get_instruction_code(SHL);
        int d = ass.gpr_index($4);
        ass.write_instruction(c.oc, c.mod, d, d, ass.gpr_index($2), 0);
        free($2); free($4);
    }
    |
    SHR GPR COMMA GPR {
        InstructionCode c = get_instruction_code(SHR);
        int d = ass.gpr_index($4);
        ass.write_instruction(c.oc, c.mod, d, d, ass.gpr_index($2), 0);
        free($2); free($4);
    }
    |
    LD operand COMMA GPR {
        int dest = ass.gpr_index($4);
        if ($2->symbol) {
            auto relocType = ($2->mod == 0b0001)
                ? Assembler::forwardRefrence::ABSOLUTE     // $simbol - direktna vrednost, bez PC
                : Assembler::forwardRefrence::PC_RELATIVE; // goli simbol/[reg+simbol] - PC baza
            ass.forwardRefTable_add_reference($2->symbol, ass.locationCounter, ass.currentSection, 4, relocType);
            free($2->symbol);
        }
        if ($2->isImmediateReg) {
            /* ld %gprS, %gprD  =>  gpr[A]<=gpr[B]+0 */
            ass.write_instruction(0b1001, 0b0001, dest, $2->plainReg, 0, 0);
        } else {
            ass.write_instruction(0b1001, $2->mod, dest, $2->regB, $2->regC, $2->disp);
        }
        free($4);
        delete $2;
    }
    |
    ST GPR COMMA operand {
        int src = ass.gpr_index($2);
        if ($4->symbol) {
            auto relocType = ($4->mod == 0b0001)
            ? Assembler::forwardRefrence::ABSOLUTE
            : Assembler::forwardRefrence::PC_RELATIVE;
            ass.forwardRefTable_add_reference($4->symbol, ass.locationCounter, ass.currentSection, 4, relocType);
            free($4->symbol);
        }
        ass.write_instruction(0b1000, $4->mod, $4->regB, $4->regC, src, $4->disp);
        free($2);
        delete $4;
    }
    |
    CSRRD CSR COMMA GPR {
        InstructionCode c = get_instruction_code(CSRRD);
        ass.write_instruction(c.oc, c.mod, ass.gpr_index($4), ass.csr_index($2), 0, 0);
        free($2); free($4);
    }
    |
    CSRWR GPR COMMA CSR {
        InstructionCode c = get_instruction_code(CSRWR);
        ass.write_instruction(c.oc, c.mod, ass.csr_index($4), ass.gpr_index($2), 0, 0);
        free($2); free($4);
    }
    ;
 /* operandi i skok labele */

jmp_operand:
            number    { 
                OperandInfo* o = new OperandInfo();
                o->symbol = nullptr; o->regB = 0; o->regC = 0;
                o->disp = (int32_t)$1;
                if ($1 >= -2048 && $1 <= 2047) {
                   o->symbol = nullptr; o->disp = (int32_t)$1; o->isIndirect = false;
                }
                else{
                    string key = ass.literalPool_findOrAdd_PoolSlot(false, (int32_t)$1, "");
                    o->symbol = strdup(key.c_str()); o->disp = 0; o->isIndirect = true;
                }
                $$ = o;
             }
            |
            symbol    { 
                OperandInfo* o = new OperandInfo();
                o->symbol = $1; o->regB = 0; o->regC = 0; o->disp = 0; o->isIndirect = false;
                $$ = o;
            }
            ;

operand:
    DOLLAR number {
        OperandInfo* o = new OperandInfo();
        o->isImmediateReg = false;
        if ($2 >= -2048 && $2 <= 2047) {
            o->symbol = nullptr; o->mod = 0b0001; o->regB = 0; o->regC = 0;
            o->disp = (int32_t)$2;
        } else {
            string key = ass.literalPool_findOrAdd_PoolSlot(false, (int32_t)$2, "");
            o->symbol = strdup(key.c_str()); 
            o->mod = 0b0010; o->regB = 15; o->regC = 0; o->disp = 0;
        }
        $$ = o;
    }
    |
    DOLLAR symbol {
        OperandInfo* o = new OperandInfo();
        string key = ass.literalPool_findOrAdd_PoolSlot(true, 0, $2);
        o->symbol = strdup(key.c_str());
        o->mod = 0b0010; o->regB = 15; o->regC = 0; o->disp = 0;
        o->isImmediateReg = false;
        free($2);
        $$ = o;
    }
    |
    number {
        /* citanje iz memorije na apsolutnoj adresi -> PC-relativno */
        OperandInfo* o = new OperandInfo();
        o->symbol = nullptr; o->mod = 0b0010; o->regB = 0; o->regC = 0;
        o->disp = (int32_t)$1; o->isImmediateReg = false;
        $$ = o;
    }
    |
    symbol {
        OperandInfo* o = new OperandInfo();
        o->symbol = $1; o->mod = 0b0010; o->regB = 15; o->regC = 0;
        o->disp = 0; o->isImmediateReg = false;
        $$ = o;
    }
    |
    GPR {
        OperandInfo* o = new OperandInfo();
        o->symbol = nullptr; o->isImmediateReg = true; o->plainReg = ass.gpr_index($1);
        free($1);
        $$ = o;
    }
    |
    LBRACKET GPR RBRACKET {
        OperandInfo* o = new OperandInfo();
        o->symbol = nullptr; o->mod = 0b0010; o->regB = ass.gpr_index($2); o->regC = 0;
        o->disp = 0; o->isImmediateReg = false;
        free($2);
        $$ = o;
    }
    |
    LBRACKET GPR PLUS number RBRACKET {
        OperandInfo* o = new OperandInfo();
        o->symbol = nullptr; o->mod = 0b0010; o->regB = ass.gpr_index($2); o->regC = 0;
        o->disp = (int32_t)$4; o->isImmediateReg = false;
        if ($4 < -2048 || $4 > 2047) yyerror("pomeraj van dozvoljenog 12-bitnog opsega");
        free($2);
        $$ = o;
    }
    |
    LBRACKET GPR PLUS symbol RBRACKET {
        OperandInfo* o = new OperandInfo();
        o->symbol = $4; o->mod = 0b0010; o->regB = ass.gpr_index($2); o->regC = 0;
        o->disp = 0; o->isImmediateReg = false;
        free($2);
        $$ = o;
    }
    ;
colon: 
	  COLON  
	  ;
%%

#include <iostream>
#include <string>

using namespace std;



int main(int argc, char **argv){
extern FILE* yyin;
    string input_file;
    string output_file = "output.hex"; // podrazumevani izlaz
    if(argc < 2){
        cerr << "Upotreba: assembler [-o izlaz] ulaz\n";
        return 1;
    }

    int index = 1;
    if(string(argv[index]) == "-o"){
        if(argc < 4){
            cerr << "Nedostaje ulazni fajl\n";
            return 1;
        }
        output_file = argv[index + 1];
        input_file = argv[index + 2];
    }
    else {
        input_file = argv[index];
    }
    yyin = fopen(input_file.c_str(), "r");
    if(!yyin){
        cerr << "Ne mogu da otvorim ulazni fajl: "
             << input_file << endl;
        return 1;
    }
    if(yyparse() == 0){
        if(ass.check_undefind_sym() == -1 )return 1;
        cout << "Parsiranje uspesno.\n";
    }
    else{
        cerr << "Greska pri parsiranju.\n";
        return 1;
    }
    
    ass.output_file(output_file);
    fclose(yyin);
    return 0;
}


void yyerror(const char *s) {
    cerr << "Greska (linija " << line << "): " << s << "\n";
}