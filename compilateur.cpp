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

using namespace std;

char current;
char nextcar;
int hasNext = 0;

set<char> declaredVariables;
int labelNumber = 0;

void Error(string s){
	cerr << "Erreur : " << s << " avec le caractere [" << current << "]" << endl;
	exit(-1);
}

// Read character and skip spaces until non space character is read
void ReadChar(void){
	if(hasNext){
		current = nextcar;
		hasNext = 0;
	}
	else{
		while(cin.get(current) && (current==' ' || current=='\t' || current=='\n'));
		if(cin.eof())
			current = '\0';
	}
}

// look ahead : read one character in advance
void LookAhead(void){
	if(!hasNext){
		while(cin.get(nextcar) && (nextcar==' ' || nextcar=='\t' || nextcar=='\n'));
		if(cin.eof())
			nextcar = '\0';
		hasNext = 1;
	}
}

// Program := [DeclarationPart] StatementPart
// DeclarationPart := "[" Letter {"," Letter} "]"
// StatementPart := Statement {";" Statement} "."
// Statement := AssignementStatement
// AssignementStatement := Letter "=" Expression
// Expression := SimpleExpression [RelationalOperator SimpleExpression]
// SimpleExpression := Term {AdditiveOperator Term}
// Term := Factor {MultiplicativeOperator Factor}
// Factor := Number | Letter | "(" Expression ")" | "!" Factor
// Number := Digit{Digit}
// AdditiveOperator := "+" | "-" | "||"
// MultiplicativeOperator := "*" | "/" | "%" | "&&"
// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="
// Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
// Letter := "a"|...|"z"

bool IsDeclared(char c){
	return declaredVariables.find(c) != declaredVariables.end();
}

void Expression(void);

void Number(void){
	unsigned long long value = 0;

	if(current < '0' || current > '9')
		Error("nombre attendu");

	while(current >= '0' && current <= '9'){
		value = value * 10 + (current - '0');
		ReadChar();
	}

	cout << "\tpush $" << value << endl;
}

void Letter(void){
	char variable = current;

	if(variable < 'a' || variable > 'z')
		Error("lettre attendue");

	if(!IsDeclared(variable))
		Error("variable non declaree");

	cout << "\tpush " << variable << "(%rip)" << endl;
	ReadChar();
}

void Factor(void){
	if(current == '('){
		ReadChar();
		Expression();

		if(current != ')')
			Error("')' etait attendu");

		ReadChar();
	}
	else if(current >= '0' && current <= '9'){
		Number();
	}
	else if(current >= 'a' && current <= 'z'){
		Letter();
	}
	else if(current == '!'){
		int n = ++labelNumber;

		ReadChar();
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
	char op;

	Factor();

	while(current == '*' || current == '/' || current == '%' || current == '&'){
		op = current;

		if(current == '&'){
			ReadChar();
			if(current != '&')
				Error("l'operateur ET s'ecrit &&");
			ReadChar();
		}
		else{
			ReadChar();
		}

		Factor();

		cout << "\tpop %rbx" << endl;
		cout << "\tpop %rax" << endl;

		if(op == '*'){
			cout << "\tmulq %rbx" << endl;
			cout << "\tpush %rax" << endl;
		}
		else if(op == '/'){
			cout << "\tmovq $0, %rdx" << endl;
			cout << "\tdivq %rbx" << endl;
			cout << "\tpush %rax" << endl;
		}
		else if(op == '%'){
			cout << "\tmovq $0, %rdx" << endl;
			cout << "\tdivq %rbx" << endl;
			cout << "\tpush %rdx" << endl;
		}
		else if(op == '&'){
			cout << "\tandq %rbx, %rax" << endl;
			cout << "\tpush %rax" << endl;
		}
	}
}

void SimpleExpression(void){
	char op;

	Term();

	while(current == '+' || current == '-' || current == '|'){
		op = current;

		if(current == '|'){
			ReadChar();
			if(current != '|')
				Error("l'operateur OU s'ecrit ||");
			ReadChar();
		}
		else{
			ReadChar();
		}

		Term();

		cout << "\tpop %rbx" << endl;
		cout << "\tpop %rax" << endl;

		if(op == '+')
			cout << "\taddq %rbx, %rax" << endl;
		else if(op == '-')
			cout << "\tsubq %rbx, %rax" << endl;
		else if(op == '|')
			cout << "\torq %rbx, %rax" << endl;

		cout << "\tpush %rax" << endl;
	}
}

string RelationalOperator(void){
	string op;

	if(current == '='){
		LookAhead();
		if(nextcar == '='){
			ReadChar();
			ReadChar();
			op = "==";
		}
		else{
			Error("utilisez == pour tester l'egalite");
		}
	}
	else if(current == '!'){
		LookAhead();
		if(nextcar == '='){
			ReadChar();
			ReadChar();
			op = "!=";
		}
		else{
			Error("operateur != attendu");
		}
	}
	else if(current == '<'){
		LookAhead();
		if(nextcar == '='){
			ReadChar();
			ReadChar();
			op = "<=";
		}
		else{
			ReadChar();
			op = "<";
		}
	}
	else if(current == '>'){
		LookAhead();
		if(nextcar == '='){
			ReadChar();
			ReadChar();
			op = ">=";
		}
		else{
			ReadChar();
			op = ">";
		}
	}

	return op;
}

void Expression(void){
	string op;
	int n;

	SimpleExpression();

	if(current == '=' || current == '!' || current == '<' || current == '>'){
		op = RelationalOperator();
		SimpleExpression();

		n = ++labelNumber;

		cout << "\tpop %rbx" << endl;
		cout << "\tpop %rax" << endl;
		cout << "\tcmpq %rbx, %rax" << endl;

		if(op == "==")
			cout << "\tje True" << n << endl;
		else if(op == "!=")
			cout << "\tjne True" << n << endl;
		else if(op == "<")
			cout << "\tjb True" << n << endl;
		else if(op == ">")
			cout << "\tja True" << n << endl;
		else if(op == "<=")
			cout << "\tjbe True" << n << endl;
		else if(op == ">=")
			cout << "\tjae True" << n << endl;

		cout << "\tpush $0" << endl;
		cout << "\tjmp EndCompare" << n << endl;
		cout << "True" << n << ":" << endl;
		cout << "\tpush $-1" << endl;
		cout << "EndCompare" << n << ":" << endl;
	}
}

void DeclarationPart(void){
	if(current != '[')
		return;

	cout << "\t.data" << endl;
	cout << "\t.align 8" << endl;

	ReadChar();

	while(true){
		if(current < 'a' || current > 'z')
			Error("lettre minuscule attendue dans la declaration");

		declaredVariables.insert(current);
		cout << current << ":\t.quad 0" << endl;

		ReadChar();

		if(current == ','){
			ReadChar();
		}
		else if(current == ']'){
			ReadChar();
			break;
		}
		else{
			Error("',' ou ']' attendu");
		}
	}
}

void AssignementStatement(void){
	char variable;

	if(current < 'a' || current > 'z')
		Error("instruction d'affectation attendue");

	variable = current;

	if(!IsDeclared(variable))
		Error("variable non declaree a gauche de l'affectation");

	ReadChar();

	if(current != '=')
		Error("'=' attendu dans l'affectation");

	ReadChar();

	Expression();

	cout << "\tpop " << variable << "(%rip)" << endl;
}

void Statement(void){
	AssignementStatement();
}

void StatementPart(void){
	cout << "\t.text" << endl;
	cout << "\t.globl main" << endl;
	cout << "main:" << endl;
	cout << "\tmovq %rsp, %rbp" << endl;

	Statement();

	while(current == ';'){
		ReadChar();
		Statement();
	}

	if(current != '.')
		Error("'.' attendu a la fin du programme");

	ReadChar();
}

void Program(void){
	DeclarationPart();
	StatementPart();
}

int main(void){
	// Header for gcc assembler / linker
	cout << "\t\t\t# This code was produced by the CERI Compiler" << endl;

	// Let's proceed to the analysis and code production
	ReadChar();
	Program();

	// Trailer for the gcc assembler / linker
	cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top" << endl;
	cout << "\tret\t\t\t# Return from main function" << endl;

	return 0;
}