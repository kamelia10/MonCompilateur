%{
#include "tokeniser.h"
#include <iostream>
using namespace std;
%}

%option noyywrap
%option c++
%option yylineno

stringconst  \"[^\n"]+\"
ws      [ \t\n\r]+
alpha   [A-Za-z]
digit   [0-9]
number  {digit}+
id      {alpha}({alpha}|{digit})*
addop   (\+|\-|\|\|)
mulop   (\*|\/|%|\&\&)
relop   (\<|\>|"=="|\<=|\>=|!=)
unknown [^\"A-Za-z0-9 \n\r\t\(\)\<\>\=\!\%\&\|\}\-\;\:\.]+

%%

"VAR"       return VAR_T;
"INTEGER"   return INTEGER_T;
"BOOLEAN"   return BOOLEAN_T;

"IF"        return IF_T;
"THEN"      return THEN_T;
"ELSE"      return ELSE_T;

"WHILE"     return WHILE_T;
"DO"        return DO_T;

"FOR"       return FOR_T;
"TO"        return TO_T;
"DOWNTO"    return DOWNTO_T;

"BEGIN"     return BEGIN_T;
"END"       return END_T;

"DISPLAY"   return DISPLAY_T;

{addop}         return ADDOP;
{mulop}         return MULOP;
{relop}         return RELOP;
{number}        return NUMBER;
{id}            return ID;
{stringconst}   return STRINGCONST;

"["             return RBRACKET;
"]"             return LBRACKET;
","             return COMMA;
";"             return SEMICOLON;
"."             return DOT;
":"             return COLON;
":="            return ASSIGN;
"("             return RPARENT;
")"             return LPARENT;
"!"             return NOT;

<<EOF>>         return FEOF;

{ws}            {/* skip blanks and tabs */};

"(*" {
        int c;
        while((c = yyinput()) != 0){
            if(c == '*'){
                if((c = yyinput()) == ')')
                    break;
                else
                    unput(c);
            }
        }
};

{unknown}       return UNKNOWN;
%%