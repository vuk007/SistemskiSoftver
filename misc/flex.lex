%{
#include <iostream>
#include "../inc/parser.hpp"
#include <cstring>
using namespace std;

/*kopira vrednost stringa - koristi ko yylvel(), treba free() da ne dodje do memmoryleak*/
static char* dupstr(const char* s) { return strdup(s); }

/*funkcije za prebacivanje u vrednosti radi lakse implementacije i rada*/
static void hex_to_num(const char* yytext ,long* val ){*val = strtol(yytext, nullptr, 16);}
static void str_to_num(const char* yytext ,long* val ){*val = strtol(yytext, nullptr, 10);}

int line = 0;
int character_in_line = 0;
%}

DIGIT [0-9]
SYMBOL [a-zA-Z][a-zA-Z0-9_]*
HEX 0[xX][0123456789abcdefABCDEF]+

%%

".global"   {return GLOBAL;}
".extern"   {return EXTERN;}
".section"  {return SECTION;}
".word"     { return WORD; }
".end"      { return END; }
".equ"      { return EQU; }
".skip"     { return SKIP; }
".ascii"    { return ASCII; }
"halt"      { return HALT; }
"int"       { return INT; }
"iret"      { return IRET; }
"call"      { return CALL; }
"ret"       { return RET; }
"jmp"       { return JMP; }
"beq"       { return BEQ; }
"bne"       { return BNE; }
"bgt"       { return BGT; }
"push"      { return PUSH; }
"pop"       { return POP; }
"xchg"      { return XCHG; }
"add"       { return ADD; }
"sub"       { return SUB; }
"mul"       { return MUL; }
"div"       { return DIV; }
"not"       { return NOT; }
"and"       { return AND; }
"or"        { return OR; }
"xor"       { return XOR; }
"shl"       { return SHL; }
"shr"       { return SHR; }
"ld"        { return LD; }
"st"        { return ST; }
"csrrd"     { return CSRRD; }
"csrwr"     { return CSRWR; }

 /* registri*/

"%status"   { yylval.str = dupstr(yytext); return CSR; }
"%handler"  { yylval.str = dupstr(yytext); return CSR; }
"%cause"    { yylval.str = dupstr(yytext); return CSR; }

 /* opstenamenski registri*/
"%r"({DIGIT}|1[0-5])   { yylval.str = dupstr(yytext); return GPR; }
"%sp"                  { yylval.str = dupstr(yytext); return GPR; }
"%pc"                  { yylval.str = dupstr(yytext); return GPR; }


":"          { return COLON; }
"["          { return LBRACKET; }
"]"          { return RBRACKET; }
"+"          { return PLUS; }
","          { return COMMA; }

"$"          { return DOLLAR; }
"#".*        {}

{SYMBOL}    { yylval.str = dupstr(yytext); return SYMBOL;     }
{HEX}       { hex_to_num(yytext, &yylval.val); return NUMBER; }
{DIGIT}+    { str_to_num(yytext, &yylval.val); return NUMBER; }

\n { 
    line++; 
    character_in_line=0;
    cout << "\n";
    return NEWLINE;
}

. { character_in_line++; }

%%
int yywrap() {
    return 1;
}

