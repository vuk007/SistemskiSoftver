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

/* REGISTRI*/
%token GPR CSR

/* ADRESIRANJA I CONSTANTE*/

%token COLON LBRACKET RBRACKET PLUS  COMMA DOLLAR

/* Nazivi simobla i vrednosti broja*/

%token <str> SYMBOL
%token <val> NUMBER HEX

%%
program
      :
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