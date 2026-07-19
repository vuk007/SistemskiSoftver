%{
/* C i C++ kod */
  #include <iostream>
  #include "assembler.hpp"
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
      symbol colon  { cout << "[LABEL] " << $1 << "\n"; 
        ass.add_symbol($1);
        ass.set_defined_symbol($1);
        ass.set_sectionsymbol($1,ass.currentSection);
        ass.set_basesymbol($1 , ass.locationCounter);
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
                              if (!ass.currentSection.empty())
                                ass.set_section_size(ass.currentSection, ass.locationCounter);
                              ass.add_section($2);
                              ass.currentSection = $2;
                              ass.locationCounter = ass.get_section_size($2);
                              free($2); }
    | 
    WORD init_list            
    | 
    SKIP number                { for(long i = 0 ; i < $2 ; i++)ass.write_byte(0); }
    | 
    ASCII STRING               { for(long i = 1 ; i + 1 < strlen($2); i++)
                                                  ass.write_byte((uint8_t)$2[i]);/*preskacem navodnike*/  }
    | 
    EQU symbol COMMA expr      { cout << "[.equ] " << $2 << " = " << $4 << "\n"; free($2); } //TODO
    | 
    END                        { 
                                if(!ass.currentSection.empty()){
                                  ass.increase_section_size(ass.currentSection, ass.locationCounter);
                                }
                                ass.print_symbol_table(); 
                                ass.print_section_table();
                                ass.print_forward_reference_table();
                              }
    ;


symbol_list:
            symbol   {
              ass.add_symbol($1);
              newsymbols.push_back($1);
              free($1);
              }
            |
            symbol_list COMMA symbol   { 
              ass.add_symbol($3);
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
                ass.add_forward_reference(ass.locationCounter, ass.currentSection, 4,
                                            Assembler::forwardRefrence::ABSOLUTE);
                ass.write_word(0);
                free($1);
      }
    |
    number    { ass.write_word((uint32_t)$1); }
    ;


expr:
    number    { $$ = $1; }
    |
    symbol    { cout << "  [UPOZORENJE] simbol '" << $1 << "' u .equ izrazu - vrednost se racuna kasnije\n"; free($1); $$ = 0; }
    |
    expr PLUS expr    { $$ = $1 + $3; }
    | 
    expr '-' expr     { $$ = $1 - $3; }
    | 
    expr '*' expr     { $$ = $1 * $3; }
    | 
    expr '/' expr     { $$ = $1 / $3; }
    |
     '(' expr ')'     { $$ = $2; }
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
    IRET                                   { cout << "[IRET]\n"; }
    | 
    RET                                    { cout << "[RET]\n"; }
    |
    CALL jmp_operand    { 
      
      InstructionCode c = get_instruction_code(CALL);
        int mod = c.mod; 
        if ($2->symbol) {
            ass.add_forward_reference($2->symbol, ass.locationCounter, ass.currentSection, 4,
                                       Assembler::forwardRefrence::PC_RELATIVE);
            free($2->symbol);
        }
        ass.write_instruction(c.oc, mod, 15, $2->regB, $2->regC, $2->disp);
        delete $2;
    }
    | 
    JMP jmp_operand           { 
      
       InstructionCode c = get_instruction_code(JMP);
        if ($2->symbol) {
            ass.add_forward_reference($2->symbol,ass.locationCounter, ass.currentSection, 4,
                                       Assembler::forwardRefrence::PC_RELATIVE);
            free($2->symbol);
        }
        ass.write_instruction(c.oc, c.mod, 15, $2->regB, $2->regC, $2->disp);
        delete $2;
    }
    | 
    BEQ GPR COMMA GPR COMMA jmp_operand {
      InstructionCode c = get_instruction_code(BEQ);
        if ($6->symbol) {
            ass.add_forward_reference(ass.locationCounter, ass.currentSection, 4,
                                       Assembler::forwardRefrence::PC_RELATIVE);
            free($6->symbol);
        }
        /* jmp-format, MOD 0001 : if (gpr[B]==gpr[C]) pc<=gpr[A]+D; A=pc */
        ass.write_instruction(c.oc, c.mod, 15, gpr_index($2), gpr_index($4), $6->disp);
        free($2); free($4); delete $6;
    }
    |
    BNE GPR COMMA GPR COMMA jmp_operand {
      InstructionCode c = get_instruction_code(BNE);
        if ($6->symbol) {
            ass.add_forward_reference(ass.locationCounter, ass.currentSection, 4,
                                       Assembler::forwardRefrence::PC_RELATIVE);
            free($6->symbol);
        }
        ass.write_instruction(c.oc,c.mod, 15, gpr_index($2), gpr_index($4), $6->disp);
        free($2); free($4); delete $6;
    }
    |
    BGT GPR COMMA GPR COMMA jmp_operand {
        if ($6->symbol) {
            ass.add_forward_reference(ass.locationCounter, ass.currentSection, 4,
                                       Assembler::forwardRefrence::PC_RELATIVE);
            free($6->symbol);
        }
        ass.write_instruction(0b0011, 0b0011, 15, gpr_index($2), gpr_index($4), $6->disp);
        free($2); free($4); delete $6;
    }
    |
    PUSH GPR {
        /* sp<=sp-4; mem32[sp]<=gpr;  ->  store format, mod 0001, A=sp, D=-4, C=gpr */
        ass.write_instruction(0b1000, 0b0001, 14, 0, gpr_index($2), -4);
        free($2);
    }
    |
    POP GPR {
        /* gpr<=mem32[sp]; sp<=sp+4;  -> load format, mod 0011, A=gpr, B=sp, D=4 */
        ass.write_instruction(0b1001, 0b0011, gpr_index($2), 14, 0, 4);
        free($2);
    }
    |
    NOT GPR {
        InstructionCode c = get_instruction_code(NOT);
        int r = gpr_index($2);
        ass.write_instruction(c.oc, c.mod, r, r, 0, 0);
        free($2);
    }
    |
    XCHG GPR COMMA GPR {
        InstructionCode c = get_instruction_code(XCHG);
        ass.write_instruction(c.oc, c.mod, 0, gpr_index($2), gpr_index($4), 0);
        free($2); free($4);
    }
    |
    ADD GPR COMMA GPR {
        InstructionCode c = get_instruction_code(ADD);
        int d = gpr_index($4);
        ass.write_instruction(c.oc, c.mod, d, d, gpr_index($2), 0);
        free($2); free($4);
    }
    |
    SUB GPR COMMA GPR {
        InstructionCode c = get_instruction_code(SUB);
        int d = gpr_index($4);
        ass.write_instruction(c.oc, c.mod, d, d, gpr_index($2), 0);
        free($2); free($4);
    }
    |
    MUL GPR COMMA GPR {
        InstructionCode c = get_instruction_code(MUL);
        int d = gpr_index($4);
        ass.write_instruction(c.oc, c.mod, d, d, gpr_index($2), 0);
        free($2); free($4);
    }
    |
    DIV GPR COMMA GPR {
        InstructionCode c = get_instruction_code(DIV);
        int d = gpr_index($4);
        ass.write_instruction(c.oc, c.mod, d, d, gpr_index($2), 0);
        free($2); free($4);
    }
    |
    AND GPR COMMA GPR {
        InstructionCode c = get_instruction_code(AND);
        int d = gpr_index($4);
        ass.write_instruction(c.oc, c.mod, d, d, gpr_index($2), 0);
        free($2); free($4);
    }
    |
    OR GPR COMMA GPR {
        InstructionCode c = get_instruction_code(OR);
        int d = gpr_index($4);
        ass.write_instruction(c.oc, c.mod, d, d, gpr_index($2), 0);
        free($2); free($4);
    }
    |
    XOR GPR COMMA GPR {
        InstructionCode c = get_instruction_code(XOR);
        int d = gpr_index($4);
        ass.write_instruction(c.oc, c.mod, d, d, gpr_index($2), 0);
        free($2); free($4);
    }
    |
    SHL GPR COMMA GPR {
        InstructionCode c = get_instruction_code(SHL);
        int d = gpr_index($4);
        ass.write_instruction(c.oc, c.mod, d, d, gpr_index($2), 0);
        free($2); free($4);
    }
    |
    SHR GPR COMMA GPR {
        InstructionCode c = get_instruction_code(SHR);
        int d = gpr_index($4);
        ass.write_instruction(c.oc, c.mod, d, d, gpr_index($2), 0);
        free($2); free($4);
    }
    |
    LD operand COMMA GPR {
        int dest = gpr_index($4);
        if ($2->symbol) {
            ass.add_forward_reference(ass.locationCounter, ass.currentSection, 4,
                                       Assembler::forwardRefrence::PC_RELATIVE);
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
        int src = gpr_index($2);
        if ($4->symbol) {
            ass.add_forward_reference(ass.locationCounter, ass.currentSection, 4,
                                       Assembler::forwardRefrence::PC_RELATIVE);
            free($4->symbol);
        }
        ass.write_instruction(0b1000, $4->mod, $4->regB, $4->regC, src, $4->disp);
        free($2);
        delete $4;
    }
    |
    CSRRD CSR COMMA GPR {
        InstructionCode c = get_instruction_code(CSRRD);
        ass.write_instruction(c.oc, c.mod, gpr_index($4), csr_index($2), 0, 0);
        free($2); free($4);
    }
    |
    CSRWR GPR COMMA CSR {
        InstructionCode c = get_instruction_code(CSRWR);
        ass.write_instruction(c.oc, c.mod, csr_index($4), gpr_index($2), 0, 0);
        free($2); free($4);
    }
    ;
 /* operandi i skok labele */

jmp_operand:
            number    { 
                OperandInfo* o = new OperandInfo();
                o->symbol = nullptr; o->regB = -1; o->regC = 0;
                o->disp = (int32_t)$1;
                if ($1 < -2048 || $1 > 2047) yyerror("skok van dozvoljenog 12-bitnog opsega");
                $$ = o;
             }
            |
            symbol    { 
                OperandInfo* o = new OperandInfo();
                o->symbol = $1; o->regB = -1; o->regC = 0; o->disp = 0;
                $$ = o;
            }
            ;

operand:
    DOLLAR number {
        OperandInfo* o = new OperandInfo();
        o->symbol = nullptr; o->mod = 0b0001; o->regB = 0; o->regC = 0;
        o->disp = (int32_t)$2; o->isImmediateReg = false;
        $$ = o;
    }
    |
    DOLLAR symbol {
        OperandInfo* o = new OperandInfo();
        o->symbol = $2; o->mod = 0b0001; o->regB = 0; o->regC = 0;
        o->disp = 0; o->isImmediateReg = false;
        $$ = o;
    }
    |
    number {
        /* citanje iz memorije na apsolutnoj adresi -> PC-relativno */
        OperandInfo* o = new OperandInfo();
        o->symbol = nullptr; o->mod = 0b0010; o->regB = 15; o->regC = 0;
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
        o->symbol = nullptr; o->isImmediateReg = true; o->plainReg = gpr_index($1);
        free($1);
        $$ = o;
    }
    |
    LBRACKET GPR RBRACKET {
        OperandInfo* o = new OperandInfo();
        o->symbol = nullptr; o->mod = 0b0010; o->regB = gpr_index($2); o->regC = 0;
        o->disp = 0; o->isImmediateReg = false;
        free($2);
        $$ = o;
    }
    |
    LBRACKET GPR PLUS number RBRACKET {
        OperandInfo* o = new OperandInfo();
        o->symbol = nullptr; o->mod = 0b0010; o->regB = gpr_index($2); o->regC = 0;
        o->disp = (int32_t)$4; o->isImmediateReg = false;
        if ($4 < -2048 || $4 > 2047) yyerror("pomeraj van dozvoljenog 12-bitnog opsega");
        free($2);
        $$ = o;
    }
    |
    LBRACKET GPR PLUS symbol RBRACKET {
        OperandInfo* o = new OperandInfo();
        o->symbol = $4; o->mod = 0b0010; o->regB = gpr_index($2); o->regC = 0;
        o->disp = 0; o->isImmediateReg = false;
        free($2);
        $$ = o;
    }
    ;
colon: 
	  COLON  {cout<<":\n";}
	  ;
%%

int main(int argc, char **argv){
  extern FILE* yyin;
  if(argc > 1){
    yyin = fopen(argv[1] , "r");
    if (!yyin) { cerr << "Ne mogu da otvorim: " << argv[1] << "\n"; return 1; }
  }
  if (yyparse() == 0) cout << "Parsiranje uspesno.\n";
  return 0; 
}


void yyerror(const char *s) {
    cerr << "Greska (linija " << line << "): " << s << "\n";
}