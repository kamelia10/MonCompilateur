//  A compiler from a very simple Pascal-like structured language LL(k)
//  to 64-bit 80x86 Assembly langage
//  Copyright (C) 2019 Pierre Jourlin
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Build with "make compilateur"

#include <string>
#include <iostream>
#include <cstdlib>
#include <set>
#include <cstring>
#include <FlexLexer.h>
#include "tokeniser.h"

using namespace std;

TOKEN current;
FlexLexer* lexer = new yyFlexLexer;

set<string> declaredVariables;
unsigned long labelNumber = 0;

void Error(string message){
	cerr << "Ligne " << lexer->lineno()
	     << " : lu [" << lexer->YYText() << "] mais "
	     << message << endl;
	exit(-1);
}

void NextToken(void){
	current = (TOKEN) lexer->yylex();
}

bool IsDeclared(string id){
	return declaredVariables.find(id) != declaredVariables.end();
}

// Program := [DeclarationPart] StatementPart
// DeclarationPart := "[" Identifier {"," Identifier} "]"
// StatementPart := Statement {";" Statement} "."
// Statement := AssignementStatement | IfStatement | WhileStatement | ForStatement | BlockStatement
// AssignementStatement := Identifier ":=" Expression
// IfStatement := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
// WhileStatement := "WHILE" Expression "DO" Statement
// ForStatement := "FOR" AssignementStatement "TO" Expression "DO" Statement
// BlockStatement := "BEGIN" Statement { ";" Statement } "END"
// Expression := SimpleExpression [RelationalOperator SimpleExpression]
// SimpleExpression := Term {AdditiveOperator Term}
// Term := Factor {MultiplicativeOperator Factor}
// Factor := Number | Identifier | "(" Expression ")" | "!" Factor

void Expression(void);
void Statement(void);

void Identifier(void){
	string name = lexer->YYText();

	if(!IsDeclared(name))
		Error("variable non declaree");

	cout << "\tpush " << name << "(%rip)" << endl;
	NextToken();
}

void Number(void){
	cout << "\tpush $" << lexer->YYText() << endl;
	NextToken();
}

void Factor(void){
	if(current == LPARENT){
		NextToken();
		Expression();

		if(current != RPARENT)
			Error("')' attendu");

		NextToken();
	}
	else if(current == NUMBER){
		Number();
	}
	else if(current == ID){
		Identifier();
	}
	else if(current == NOT){
		unsigned long n = ++labelNumber;

		NextToken();
		Factor();

		cout << "\tpop %rax" << endl;
		cout << "\tcmpq $0, %rax" << endl;
		cout << "\tje NotTrue" << n << endl;
		cout << "\tpush $0" << endl;
		cout << "\tjmp EndNot" << n << endl;
		cout << "NotTrue" << n << ":" << endl;
		cout << "\tpush $-1" << endl;
		cout << "EndNot" << n << ":" << endl;
	}
	else{
		Error("facteur attendu");
	}
}

void Term(void){
	string op;

	Factor();

	while(current == MULOP){
		op = lexer->YYText();
		NextToken();

		Factor();

		cout << "\tpop %rbx" << endl;
		cout << "\tpop %rax" << endl;

		if(op == "*"){
			cout << "\tmulq %rbx" << endl;
			cout << "\tpush %rax" << endl;
		}
		else if(op == "/"){
			cout << "\tmovq $0, %rdx" << endl;
			cout << "\tdivq %rbx" << endl;
			cout << "\tpush %rax" << endl;
		}
		else if(op == "%"){
			cout << "\tmovq $0, %rdx" << endl;
			cout << "\tdivq %rbx" << endl;
			cout << "\tpush %rdx" << endl;
		}
		else if(op == "&&"){
			cout << "\tandq %rbx, %rax" << endl;
			cout << "\tpush %rax" << endl;
		}
		else{
			Error("operateur multiplicatif inconnu");
		}
	}
}

void SimpleExpression(void){
	string op;

	Term();

	while(current == ADDOP){
		op = lexer->YYText();
		NextToken();

		Term();

		cout << "\tpop %rbx" << endl;
		cout << "\tpop %rax" << endl;

		if(op == "+")
			cout << "\taddq %rbx, %rax" << endl;
		else if(op == "-")
			cout << "\tsubq %rbx, %rax" << endl;
		else if(op == "||")
			cout << "\torq %rbx, %rax" << endl;
		else
			Error("operateur additif inconnu");

		cout << "\tpush %rax" << endl;
	}
}

void Expression(void){
	string op;
	unsigned long n;

	SimpleExpression();

	if(current == RELOP){
		op = lexer->YYText();
		NextToken();

		SimpleExpression();

		n = ++labelNumber;

		cout << "\tpop %rbx" << endl;
		cout << "\tpop %rax" << endl;
		cout << "\tcmpq %rbx, %rax" << endl;

		if(op == "==")
			cout << "\tje BoolTrue" << n << endl;
		else if(op == "!=")
			cout << "\tjne BoolTrue" << n << endl;
		else if(op == "<")
			cout << "\tjb BoolTrue" << n << endl;
		else if(op == ">")
			cout << "\tja BoolTrue" << n << endl;
		else if(op == "<=")
			cout << "\tjbe BoolTrue" << n << endl;
		else if(op == ">=")
			cout << "\tjae BoolTrue" << n << endl;
		else
			Error("operateur relationnel inconnu");

		cout << "\tpush $0" << endl;
		cout << "\tjmp BoolEnd" << n << endl;
		cout << "BoolTrue" << n << ":" << endl;
		cout << "\tpush $-1" << endl;
		cout << "BoolEnd" << n << ":" << endl;
	}
}

void DeclarationPart(void){
	if(current != LBRACKET)
		return;

	cout << "\t.data" << endl;
	cout << "\t.align 8" << endl;

	NextToken();

	if(current != ID)
		Error("identificateur attendu dans la declaration");

	while(true){
		if(current != ID)
			Error("identificateur attendu");

		string name = lexer->YYText();
		declaredVariables.insert(name);

		cout << name << ":\t.quad 0" << endl;

		NextToken();

		if(current == COMMA){
			NextToken();
		}
		else if(current == RBRACKET){
			NextToken();
			break;
		}
		else{
			Error("',' ou ']' attendu");
		}
	}
}

string AssignementStatement(void){
	if(current != ID)
		Error("identificateur attendu a gauche de l'affectation");

	string variable = lexer->YYText();

	if(!IsDeclared(variable))
		Error("variable non declaree");

	NextToken();

	if(current != ASSIGN)
		Error("':=' attendu");

	NextToken();

	Expression();

	cout << "\tpop " << variable << "(%rip)" << endl;

	return variable;
}

void IfStatement(void){
	unsigned long n = ++labelNumber;

	NextToken();

	Expression();

	if(current != THEN_T)
		Error("THEN attendu");

	cout << "\tpop %rax" << endl;
	cout << "\tcmpq $0, %rax" << endl;
	cout << "\tje ElsePart" << n << endl;

	NextToken();
	Statement();

	if(current == ELSE_T){
		cout << "\tjmp EndIf" << n << endl;
		cout << "ElsePart" << n << ":" << endl;

		NextToken();
		Statement();

		cout << "EndIf" << n << ":" << endl;
	}
	else{
		cout << "ElsePart" << n << ":" << endl;
	}
}

void WhileStatement(void){
	unsigned long n = ++labelNumber;

	cout << "WhileBegin" << n << ":" << endl;

	NextToken();
	Expression();

	if(current != DO_T)
		Error("DO attendu");

	cout << "\tpop %rax" << endl;
	cout << "\tcmpq $0, %rax" << endl;
	cout << "\tje WhileEnd" << n << endl;

	NextToken();
	Statement();

	cout << "\tjmp WhileBegin" << n << endl;
	cout << "WhileEnd" << n << ":" << endl;
}

void ForStatement(void){
	unsigned long n = ++labelNumber;

	NextToken();

	string variable = AssignementStatement();

	if(current != TO_T)
		Error("TO attendu");

	cout << "ForBegin" << n << ":" << endl;

	NextToken();
	Expression();

	if(current != DO_T)
		Error("DO attendu");

	cout << "\tpop %rbx" << endl;
	cout << "\tmovq " << variable << "(%rip), %rax" << endl;
	cout << "\tcmpq %rbx, %rax" << endl;
	cout << "\tja ForEnd" << n << endl;

	NextToken();
	Statement();

	cout << "\taddq $1, " << variable << "(%rip)" << endl;
	cout << "\tjmp ForBegin" << n << endl;
	cout << "ForEnd" << n << ":" << endl;
}

void BlockStatement(void){
	NextToken();

	Statement();

	while(current == SEMICOLON){
		NextToken();

		if(current == END_T)
			break;

		Statement();
	}

	if(current != END_T)
		Error("END attendu");

	NextToken();
}

void Statement(void){
	if(current == ID){
		AssignementStatement();
	}
	else if(current == IF_T){
		IfStatement();
	}
	else if(current == WHILE_T){
		WhileStatement();
	}
	else if(current == FOR_T){
		ForStatement();
	}
	else if(current == BEGIN_T){
		BlockStatement();
	}
	else{
		Error("instruction attendue");
	}
}

void StatementPart(void){
	cout << "\t.text" << endl;
	cout << "\t.globl main" << endl;
	cout << "main:" << endl;
	cout << "\tmovq %rsp, %rbp" << endl;

	Statement();

	while(current == SEMICOLON){
		NextToken();

		if(current == DOT)
			break;

		Statement();
	}

	if(current != DOT)
		Error("'.' attendu a la fin du programme");

	NextToken();
}

void Program(void){
	DeclarationPart();
	StatementPart();
}

int main(void){
	cout << "\t\t\t# This code was produced by the CERI Compiler" << endl;

	NextToken();
	Program();

	cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top" << endl;
	cout << "\tret\t\t\t# Return from main function" << endl;

	if(current != FEOF)
		Error("caracteres en trop a la fin du programme");

	return 0;
}