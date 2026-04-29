%{
#include "tokeniser.h"
#include <iostream>
using namespace std;
%}

%option noyywrap
%option c++
%option yylineno

ws          [ \t\n\r]+
digit       [0-9]
number      {digit}+
letter      [A-Za-z]
id          {letter}({letter}|{digit})*
stringconst \"[^\n"]+\"

addop       (\+|\-|\|\|)
mulop       (\*|\/|%|\&\&)
relop       ("=="|"!="|"<="|">="|"<"|">")
unknown     .

%%

"IF"        return IF_T;
"THEN"      return THEN_T;
"ELSE"      return ELSE_T;
"WHILE"     return WHILE_T;
"DO"        return DO_T;
"FOR"       return FOR_T;
"TO"        return TO_T;
"BEGIN"     return BEGIN_T;
"END"       return END_T;

":="        return ASSIGN;
{addop}     return ADDOP;
{mulop}     return MULOP;
{relop}     return RELOP;
{number}    return NUMBER;
{id}        return ID;
{stringconst} return STRINGCONST;

"["         return LBRACKET;
"]"         return RBRACKET;
"("         return LPARENT;
")"         return RPARENT;
","         return COMMA;
";"         return SEMICOLON;
"."         return DOT;
"!"         return NOT;

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
}

{ws}        ;
<<EOF>>     return FEOF;
{unknown}   return UNKNOWN;

%%
