%{
#include "tokeniser.h"
#include <iostream>
using namespace std;
%}

%option noyywrap
%option c++
%option yylineno

stringconst  \"[^\n"]+\"
charconst    \'[^\n\']\'
ws           [ \t\n\r]+
alpha        [A-Za-z]
digit        [0-9]
floatnumber  {digit}+\.{digit}+
number       {digit}+
id           {alpha}({alpha}|{digit})*
addop        (\+|\-|\|\|)
mulop        (\*|\/|%|\&\&)
relop        ("=="|"!="|"<="|">="|"<"|">")
unknown      .

%%

"VAR"       return VAR_T;
"INTEGER"   return INTEGER_T;
"BOOLEAN"   return BOOLEAN_T;
"DOUBLE"    return DOUBLE_T;
"CHAR"      return CHAR_T;

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

":="        return ASSIGN;
":"         return COLON;

{floatnumber} return FLOAT_NUMBER;
{number}      return NUMBER;
{charconst}   return CHARCONST;
{addop}       return ADDOP;
{mulop}       return MULOP;
{relop}       return RELOP;
{id}          return ID;
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