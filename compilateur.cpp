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
#include <map>
#include <vector>
#include <cstring>
#include <FlexLexer.h>
#include "tokeniser.h"

using namespace std;

enum TYPE {
	UNSIGNED_INT,
	BOOLEAN
};

TOKEN current;
FlexLexer* lexer = new yyFlexLexer;

map<string, TYPE> declaredVariables;
unsigned long labelNumber = 0;

void Error(string message){
	cerr << "Ligne " << lexer->lineno()
	     << " : lu [" << lexer->YYText() << "] mais "
	     << message << endl;
	exit(-1);
}

void TypeError(string message){
	cerr << "Erreur de type ligne " << lexer->lineno()
	     << " : " << message << endl;
	exit(-1);
}

void NextToken(void){
	current = (TOKEN) lexer->yylex();
}

bool IsDeclared(string id){
	return declaredVariables.find(id) != declaredVariables.end();
}

TYPE TypeOfIdentifier(string id){
	if(!IsDeclared(id))
		Error("variable non declaree");

	return declaredVariables[id];
}

string TypeName(TYPE t){
	if(t == UNSIGNED_INT)
		return "INTEGER";
	return "BOOLEAN";
}

// Program := [VarDeclarationPart] StatementPart
// VarDeclarationPart := "VAR" VarDeclaration {";" VarDeclaration} "."
// VarDeclaration := Identifier {"," Identifier} ":" Type
// Type := "INTEGER" | "BOOLEAN"
// StatementPart := Statement {";" Statement} "."
// Statement := AssignementStatement | IfStatement | WhileStatement | ForStatement | BlockStatement | DisplayStatement
// AssignementStatement := Identifier ":=" Expression
// DisplayStatement := "DISPLAY" Expression
// IfStatement := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
// WhileStatement := "WHILE" Expression "DO" Statement
// ForStatement := "FOR" AssignementStatement "TO" Expression "DO" Statement
// BlockStatement := "BEGIN" Statement { ";" Statement } "END"
// Expression := SimpleExpression [RelationalOperator SimpleExpression]
// SimpleExpression := Term {AdditiveOperator Term}
// Term := Factor {MultiplicativeOperator Factor}
// Factor := Number | Identifier | "(" Expression ")" | "!" Factor

TYPE Expression(void);
void Statement(void);

TYPE ReadType(void){
	TYPE t;

	if(current == INTEGER_T){
		t = UNSIGNED_INT;
		NextToken();
		return t;
	}

	if(current == BOOLEAN_T){
		t = BOOLEAN;
		NextToken();
		return t;
	}

	Error("type INTEGER ou BOOLEAN attendu");
	return UNSIGNED_INT;
}

TYPE Identifier(void){
	string name = lexer->YYText();

	if(!IsDeclared(name))
		Error("variable non declaree");

	cout << "\tpush " << name << "(%rip)" << endl;
	NextToken();

	return TypeOfIdentifier(name);
}

TYPE Number(void){
	cout << "\tpush $" << lexer->YYText() << endl;
	NextToken();

	return UNSIGNED_INT;
}

TYPE Factor(void){
	TYPE factorType;

	if(current == LPARENT){
		NextToken();
		factorType = Expression();

		if(current != RPARENT)
			Error("')' attendu");

		NextToken();
		return factorType;
	}
	else if(current == NUMBER){
		return Number();
	}
	else if(current == ID){
		return Identifier();
	}
	else if(current == NOT){
		unsigned long n = ++labelNumber;

		NextToken();
		factorType = Factor();

		if(factorType != BOOLEAN)
			TypeError("l'operateur ! doit etre applique a un booleen");

		cout << "\tpop %rax" << endl;
		cout << "\tcmpq $0, %rax" << endl;
		cout << "\tje NotTrue" << n << endl;
		cout << "\tpush $0" << endl;
		cout << "\tjmp EndNot" << n << endl;
		cout << "NotTrue" << n << ":" << endl;
		cout << "\tpush $-1" << endl;
		cout << "EndNot" << n << ":" << endl;

		return BOOLEAN;
	}

	Error("facteur attendu");
	return UNSIGNED_INT;
}

TYPE Term(void){
	string op;
	TYPE leftType;
	TYPE rightType;

	leftType = Factor();

	while(current == MULOP){
		op = lexer->YYText();
		NextToken();

		rightType = Factor();

		if(op == "&&"){
			if(leftType != BOOLEAN || rightType != BOOLEAN)
				TypeError("l'operateur && attend deux booleens");
			leftType = BOOLEAN;
		}
		else{
			if(leftType != UNSIGNED_INT || rightType != UNSIGNED_INT)
				TypeError("les operateurs * / % attendent deux INTEGER");
			leftType = UNSIGNED_INT;
		}

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

	return leftType;
}

TYPE SimpleExpression(void){
	string op;
	TYPE leftType;
	TYPE rightType;

	leftType = Term();

	while(current == ADDOP){
		op = lexer->YYText();
		NextToken();

		rightType = Term();

		if(op == "||"){
			if(leftType != BOOLEAN || rightType != BOOLEAN)
				TypeError("l'operateur || attend deux booleens");
			leftType = BOOLEAN;
		}
		else{
			if(leftType != UNSIGNED_INT || rightType != UNSIGNED_INT)
				TypeError("les operateurs + et - attendent deux INTEGER");
			leftType = UNSIGNED_INT;
		}

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

	return leftType;
}

TYPE Expression(void){
	string op;
	unsigned long n;
	TYPE leftType;
	TYPE rightType;

	leftType = SimpleExpression();

	if(current == RELOP){
		op = lexer->YYText();
		NextToken();

		rightType = SimpleExpression();

		if(leftType != rightType)
			TypeError("comparaison entre " + TypeName(leftType) + " et " + TypeName(rightType));

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

		return BOOLEAN;
	}

	return leftType;
}

void OneVarDeclaration(void){
	vector<string> names;
	TYPE declaredType;

	if(current != ID)
		Error("identificateur attendu dans la declaration");

	names.push_back(lexer->YYText());
	NextToken();

	while(current == COMMA){
		NextToken();

		if(current != ID)
			Error("identificateur attendu apres ','");

		names.push_back(lexer->YYText());
		NextToken();
	}

	if(current != COLON)
		Error("':' attendu dans la declaration");

	NextToken();

	declaredType = ReadType();

	for(unsigned int i=0; i<names.size(); i++){
		if(IsDeclared(names[i]))
			Error("variable deja declaree");

		declaredVariables[names[i]] = declaredType;
		cout << names[i] << ":\t.quad 0\t# " << TypeName(declaredType) << endl;
	}
}

void VarDeclarationPart(void){
	if(current != VAR_T)
		return;

	NextToken();

	OneVarDeclaration();

	while(current == SEMICOLON){
		NextToken();

		if(current == DOT)
			break;

		OneVarDeclaration();
	}

	if(current != DOT)
		Error("'.' attendu apres les declarations VAR");

	NextToken();
}

string AssignementStatement(void){
	if(current != ID)
		Error("identificateur attendu a gauche de l'affectation");

	string variable = lexer->YYText();
	TYPE variableType = TypeOfIdentifier(variable);

	NextToken();

	if(current != ASSIGN)
		Error("':=' attendu");

	NextToken();

	TYPE expressionType = Expression();

	if(variableType != expressionType)
		TypeError("affectation impossible : " + variable + " est " + TypeName(variableType)
		          + " mais l'expression est " + TypeName(expressionType));

	cout << "\tpop " << variable << "(%rip)" << endl;

	return variable;
}

void DisplayStatement(void){
	NextToken();

	TYPE expressionType = Expression();

	if(expressionType != UNSIGNED_INT)
		TypeError("DISPLAY ne peut afficher qu'un INTEGER");

	cout << "\tpop %rdx\t# The value to be displayed" << endl;
	cout << "\tleaq FormatString1(%rip), %rcx\t# \"%llu\\n\"" << endl;
	cout << "\tsubq $40, %rsp\t# shadow space + align stack for Windows printf" << endl;
	cout << "\tcall printf" << endl;
	cout << "\taddq $40, %rsp\t# restore stack" << endl;
}

void IfStatement(void){
	unsigned long n = ++labelNumber;

	NextToken();

	TYPE conditionType = Expression();

	if(conditionType != BOOLEAN)
		TypeError("la condition du IF doit etre BOOLEAN");

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

	TYPE conditionType = Expression();

	if(conditionType != BOOLEAN)
		TypeError("la condition du WHILE doit etre BOOLEAN");

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

	if(TypeOfIdentifier(variable) != UNSIGNED_INT)
		TypeError("la variable du FOR doit etre INTEGER");

	if(current != TO_T)
		Error("TO attendu");

	cout << "ForBegin" << n << ":" << endl;

	NextToken();

	TYPE limitType = Expression();

	if(limitType != UNSIGNED_INT)
		TypeError("la borne du FOR doit etre INTEGER");

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
	else if(current == DISPLAY_T){
		DisplayStatement();
	}
	else{
		Error("instruction attendue");
	}
}

void StatementPart(void){
	cout << "\t.text" << endl;
	cout << "\t.globl main" << endl;
	cout << "\t.extern printf" << endl;
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
	cout << "\t.data" << endl;
	cout << "\t.align 8" << endl;
	cout << "FormatString1:\t.string \"%llu\\n\"" << endl;

	VarDeclarationPart();
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