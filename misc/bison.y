%{
/* C i C++ kod */
  #include <iostream>
  using namespace std;
  void yyerror(const char *s);
  extern int yylex(); 
  extern int line; 
  
%}

%union {
  char* str;
  long val;
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
      symbol colon  {cout<<"{label:}";}
      ;

symbol:
      SYMBOL  {cout<<"Symbol\n";}
      ;

number: 
      NUMBER   {cout<<"number\n";}
      ;  

statement:
        instruction
        |
        directive
        ;




 /* DIREKTIVE */
directive: 
    GLOBAL symbol_list        { cout << "[.global]\n"; }
    | 
    EXTERN symbol_list        { cout << "[.extern]\n"; }
    | 
    SECTION name            { cout << "[.section] "  << "\n"; }
    | 
    WORD init_list            { cout << "[.word]\n"; }
    | 
    SKIP number                { cout << "[.skip] "  << "\n"; }
    | 
    ASCII STRING               { cout << "[.ascii] "  << "\n";  }
    | 
    EQU symbol COMMA expr      { cout << "[.equ] "  << "\n"; }
    | 
    END                        { cout << "[.end]\n"; }
    ;


symbol_list:
            symbol
            |
            symbol_list COMMA symbol
            ;

name: 
    symbol
    ;


init_list:
          init_list COMMA init
          |
          init
          ;

init:
    symbol
    |
    number
    ;


expr:
    number
    |
    symbol
    |
    expr PLUS expr
    | 
    expr '-' expr
    | 
    expr '*' expr
    | 
    expr '/' expr
    |
     '(' expr ')'
    ;


 /* instrukcije */
instruction:
      HALT                                   { cout << "[HALT]\n"; }
    | 
    INT                                    { cout << "[INT]\n"; }
    | 
    IRET                                   { cout << "[IRET]\n"; }
    | 
    RET                                    { cout << "[RET]\n"; }
    |
    CALL jmp_operand                      { cout << "[CALL]\n"; }
    | 
    JMP jmp_operand           { cout << "[JMP]\n"; }
    | 
    BEQ GPR COMMA GPR COMMA jmp_operand    { cout << "[BEQ]\n";  }
    | 
    BNE GPR COMMA GPR COMMA jmp_operand    { cout << "[BNE]\n";  }
    | 
    BGT GPR COMMA GPR COMMA jmp_operand    { cout << "[BGT]\n";  }
    | 
    PUSH GPR                               { cout << "[PUSH] " << "\n";  }
    | 
    POP GPR                                { cout << "[POP] "  << "\n";  }
    | 
    NOT GPR                                { cout << "[NOT] "   << "\n";  }
    | 
    XCHG GPR COMMA GPR                     { cout << "[XCHG]\n";  }
    | 
    ADD  GPR COMMA GPR                     { cout << "[ADD]\n";   }
    | 
    SUB  GPR COMMA GPR                     { cout << "[SUB]\n";   }
    | 
    MUL  GPR COMMA GPR                     { cout << "[MUL]\n";   }
    | 
    DIV  GPR COMMA GPR                     { cout << "[DIV]\n";   }
    | 
    AND  GPR COMMA GPR                     { cout << "[AND]\n";   }
    | 
    OR   GPR COMMA GPR                     { cout << "[OR]\n";    }
    | 
    XOR  GPR COMMA GPR                     { cout << "[XOR]\n";   }
    | 
    SHL  GPR COMMA GPR                     { cout << "[SHL]\n";   }
    | 
    SHR  GPR COMMA GPR                     { cout << "[SHR]\n";   }
    | 
    LD operand COMMA GPR                   { cout << "[LD]\n";    }
    | 
    ST GPR COMMA operand                   { cout << "[ST]\n";    }
    | 
    CSRRD CSR COMMA GPR                    { cout << "[CSRRD]\n"; }
    | 
    CSRWR GPR COMMA CSR                    { cout << "[CSRWR]\n"; }
    ;

 /* operandi i skok labele */

jmp_operand:
            number
            |
            symbol
            ;

operand:
    DOLLAR number                            
    | 
    DOLLAR symbol              
    | 
    number                                    
    | 
    symbol                     
    | 
    GPR                     
    | 
    LBRACKET GPR RBRACKET                          /* [%<reg>] */
    | 
    LBRACKET GPR PLUS number RBRACKET               /* [%<reg> + <literal>] */
    | 
    LBRACKET GPR PLUS symbol RBRACKET               /* [%<reg> + <simbol>] */
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