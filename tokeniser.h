#ifndef TOKENISER_H
#define TOKENISER_H

enum TOKEN {
	FEOF = 0,
	UNKNOWN,
	ID,
	NUMBER,
	STRINGCONST,

	ADDOP,
	MULOP,
	RELOP,
	ASSIGN,

	LBRACKET,
	RBRACKET,
	LPARENT,
	RPARENT,
	COMMA,
	SEMICOLON,
	DOT,
	NOT,

	IF_T,
	THEN_T,
	ELSE_T,
	WHILE_T,
	DO_T,
	FOR_T,
	TO_T,
	BEGIN_T,
	END_T
};

#endif