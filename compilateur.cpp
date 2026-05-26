//  A compiler from a very simple Pascal-like structured language LL(k)
//  to 64-bit 80x86 Assembly langage
//  Copyright (C) 2019 Pierre Jourlin
//  Modified by Kamelia Bouzourene (2026) - Université d'Avignon - L2 Informatique
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

// Types supportes par le compilateur
enum TYPE {
	UNSIGNED_INT,
	BOOLEAN,
	DOUBLE_TYPE,
	CHAR_TYPE
};

TOKEN current;
FlexLexer* lexer = new yyFlexLexer;

map<string, TYPE> declaredVariables;
unsigned long labelNumber = 0;

// Affiche une erreur syntaxique et quitte
void Error(string message){
	cerr << "Ligne " << lexer->lineno()
	     << " : lu [" << lexer->YYText() << "] mais "
	     << message << endl;
	exit(-1);
}

// Affiche une erreur de type et quitte
void TypeError(string message){
	cerr << "Erreur de type ligne " << lexer->lineno()
	     << " : " << message << endl;
	exit(-1);
}

// Avance au token suivant
void NextToken(void){
	current = (TOKEN) lexer->yylex();
}

bool IsDeclared(string id){
	return declaredVariables.find(id) != declaredVariables.end();
}

TYPE TypeOfIdentifier(string id){
	if(!IsDeclared(id))
		Error("variable non declaree : " + id);
	return declaredVariables[id];
}

// Retourne le nom lisible d'un type
string TypeName(TYPE t){
	if(t == UNSIGNED_INT) return "INTEGER";
	if(t == BOOLEAN)      return "BOOLEAN";
	if(t == DOUBLE_TYPE)  return "DOUBLE";
	return "CHAR";
}

// Programme := [VarDeclarationPart] StatementPart
// VarDeclarationPart := "VAR" VarDeclaration {";" VarDeclaration} "."
// VarDeclaration := Identificateur {"," Identificateur} ":" Type
// Type := "INTEGER" | "BOOLEAN" | "DOUBLE" | "CHAR"
// Statement := AssignementStatement | IfStatement | WhileStatement
//            | ForStatement | BlockStatement | DisplayStatement | CaseStatement
// Expression := SimpleExpression [OpRel SimpleExpression]
// SimpleExpression := Term {OpAdd Term}
// Term := Factor {OpMul Factor}
// Factor := Nombre | Identificateur | "(" Expression ")" | "!" Factor | CharConst | FloatNombre

TYPE Expression(void);
void Statement(void);

// Empile un flottant 64 bits en interpretant ses bits comme un entier
void PushDoubleFromBits(double value){
	unsigned long long bits;
	memcpy(&bits, &value, sizeof(double));
	cout << "\tmovabsq $" << bits << ", %rax\t# constante double " << value << endl;
	cout << "\tpushq %rax" << endl;
}

// Lit le type d'une variable dans la declaration
TYPE ReadType(void){
	if(current == INTEGER_T){ NextToken(); return UNSIGNED_INT; }
	if(current == BOOLEAN_T){ NextToken(); return BOOLEAN; }
	if(current == DOUBLE_T) { NextToken(); return DOUBLE_TYPE; }
	if(current == CHAR_T)   { NextToken(); return CHAR_TYPE; }
	Error("type INTEGER, BOOLEAN, DOUBLE ou CHAR attendu");
	return UNSIGNED_INT;
}

// Charge la valeur d'un identificateur sur la pile
TYPE Identifier(void){
	string name = lexer->YYText();
	TYPE t = TypeOfIdentifier(name);

	if(t == CHAR_TYPE){
		// Extension zero vers 64 bits pour les caracteres
		cout << "\tmovzbq " << name << "(%rip), %rax" << endl;
		cout << "\tpushq %rax" << endl;
	}
	else{
		cout << "\tpushq " << name << "(%rip)" << endl;
	}

	NextToken();
	return t;
}

// Empile une constante entiere
TYPE Number(void){
	cout << "\tpush $" << lexer->YYText() << endl;
	NextToken();
	return UNSIGNED_INT;
}

// Empile une constante flottante
TYPE FloatNumber(void){
	double value = atof(lexer->YYText());
	PushDoubleFromBits(value);
	NextToken();
	return DOUBLE_TYPE;
}

// Empile une constante caractere (ex: 'a')
TYPE CharConstant(void){
	string txt = lexer->YYText();
	unsigned int ascii = (unsigned char)txt[1];
	cout << "\tpush $" << ascii << "\t# caractere " << txt << endl;
	NextToken();
	return CHAR_TYPE;
}

// Genere le code d'une operation arithmetique sur les doubles (FPU x87)
void GenerateDoubleOperation(string op){
	// Les deux operandes sont sur la pile : [8(%rsp)] = gauche, [(%rsp)] = droite
	cout << "\tfldl 8(%rsp)" << endl;       // st(0) = operande gauche
	cout << "\tfldl (%rsp)" << endl;        // st(0) = droite, st(1) = gauche
	cout << "\taddq $16, %rsp" << endl;     // depiler les deux operandes

	if(op == "+")
		cout << "\tfaddp %st, %st(1)" << endl;
	else if(op == "-")
		cout << "\tfsubp %st, %st(1)" << endl;  // st(1) - st(0) = gauche - droite
	else if(op == "*")
		cout << "\tfmulp %st, %st(1)" << endl;
	else if(op == "/")
		cout << "\tfdivp %st, %st(1)" << endl;  // st(1) / st(0) = gauche / droite
	else
		Error("operation flottante inconnue : " + op);

	// Empiler le resultat sur la pile generale
	cout << "\tsubq $8, %rsp" << endl;
	cout << "\tfstpl (%rsp)" << endl;
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
	else if(current == FLOAT_NUMBER){
		return FloatNumber();
	}
	else if(current == CHARCONST){
		return CharConstant();
	}
	else if(current == ID){
		return Identifier();
	}
	else if(current == NOT){
		unsigned long n = ++labelNumber;
		NextToken();
		factorType = Factor();

		if(factorType != BOOLEAN)
			TypeError("l'operateur ! doit s'appliquer a un BOOLEAN");

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

// Term := Factor {OpMul Factor}
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
				TypeError("&& attend deux BOOLEAN");
			cout << "\tpop %rbx" << endl;
			cout << "\tpop %rax" << endl;
			cout << "\tandq %rbx, %rax" << endl;
			cout << "\tpush %rax" << endl;
			leftType = BOOLEAN;
		}
		else if(leftType == DOUBLE_TYPE || rightType == DOUBLE_TYPE){
			if(leftType != DOUBLE_TYPE || rightType != DOUBLE_TYPE)
				TypeError("operation entre " + TypeName(leftType) + " et " + TypeName(rightType));
			if(op != "*" && op != "/")
				TypeError("seuls * et / sont permis pour les DOUBLE");
			GenerateDoubleOperation(op);
			leftType = DOUBLE_TYPE;
		}
		else{
			if(leftType != UNSIGNED_INT || rightType != UNSIGNED_INT)
				TypeError("* / % attendent deux INTEGER");
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
			else{
				Error("operateur multiplicatif inconnu");
			}
			leftType = UNSIGNED_INT;
		}
	}

	return leftType;
}

// SimpleExpression := Term {OpAdd Term}
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
				TypeError("|| attend deux BOOLEAN");
			cout << "\tpop %rbx" << endl;
			cout << "\tpop %rax" << endl;
			cout << "\torq %rbx, %rax" << endl;
			cout << "\tpush %rax" << endl;
			leftType = BOOLEAN;
		}
		else if(leftType == DOUBLE_TYPE || rightType == DOUBLE_TYPE){
			if(leftType != DOUBLE_TYPE || rightType != DOUBLE_TYPE)
				TypeError("operation entre " + TypeName(leftType) + " et " + TypeName(rightType));
			GenerateDoubleOperation(op);
			leftType = DOUBLE_TYPE;
		}
		else{
			if(leftType != UNSIGNED_INT || rightType != UNSIGNED_INT)
				TypeError("+ et - attendent deux INTEGER");
			cout << "\tpop %rbx" << endl;
			cout << "\tpop %rax" << endl;
			if(op == "+")
				cout << "\taddq %rbx, %rax" << endl;
			else if(op == "-")
				cout << "\tsubq %rbx, %rax" << endl;
			else
				Error("operateur additif inconnu");
			cout << "\tpush %rax" << endl;
			leftType = UNSIGNED_INT;
		}
	}

	return leftType;
}

// Genere la comparaison de deux doubles (FPU), les drapeaux EFLAGS sont mis a jour
void GenerateDoubleComparison(string op, unsigned long n){
	// [8(%rsp)] = gauche, [(%rsp)] = droite (comme les entiers)
	cout << "\tfldl (%rsp)" << endl;         // st(0) = droite
	cout << "\tfldl 8(%rsp)" << endl;        // st(0) = gauche, st(1) = droite
	cout << "\taddq $16, %rsp" << endl;      // depiler les deux doubles
	cout << "\tfcomip %st(1), %st" << endl;  // compare gauche (st0) avec droite (st1), pop st0
	cout << "\tfstp %st(0)" << endl;         // pop st1 (ancien droite)

	if(op == "==")      cout << "\tje BoolTrue" << n << endl;
	else if(op == "!=") cout << "\tjne BoolTrue" << n << endl;
	else if(op == "<")  cout << "\tjb BoolTrue" << n << endl;
	else if(op == ">")  cout << "\tja BoolTrue" << n << endl;
	else if(op == "<=") cout << "\tjbe BoolTrue" << n << endl;
	else if(op == ">=") cout << "\tjae BoolTrue" << n << endl;
	else Error("operateur de comparaison inconnu");
}

// Genere la comparaison de deux entiers, les drapeaux EFLAGS sont mis a jour
void GenerateIntegerComparison(string op, unsigned long n){
	cout << "\tpop %rbx" << endl;   // droite
	cout << "\tpop %rax" << endl;   // gauche
	cout << "\tcmpq %rbx, %rax" << endl;

	if(op == "==")      cout << "\tje BoolTrue" << n << endl;
	else if(op == "!=") cout << "\tjne BoolTrue" << n << endl;
	else if(op == "<")  cout << "\tjb BoolTrue" << n << endl;
	else if(op == ">")  cout << "\tja BoolTrue" << n << endl;
	else if(op == "<=") cout << "\tjbe BoolTrue" << n << endl;
	else if(op == ">=") cout << "\tjae BoolTrue" << n << endl;
	else Error("operateur de comparaison inconnu");
}

// Expression := SimpleExpression [OpRel SimpleExpression]
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

		if(leftType == DOUBLE_TYPE)
			GenerateDoubleComparison(op, n);
		else
			GenerateIntegerComparison(op, n);

		cout << "\tpush $0" << endl;
		cout << "\tjmp BoolEnd" << n << endl;
		cout << "BoolTrue" << n << ":" << endl;
		cout << "\tpush $-1" << endl;
		cout << "BoolEnd" << n << ":" << endl;

		return BOOLEAN;
	}

	return leftType;
}

// Lit une ligne de declaration : id1, id2, ... : TYPE
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
		Error("':' attendu dans la declaration de variable");

	NextToken();
	declaredType = ReadType();

	for(unsigned int i = 0; i < names.size(); i++){
		if(IsDeclared(names[i]))
			Error("variable deja declaree : " + names[i]);

		declaredVariables[names[i]] = declaredType;

		if(declaredType == DOUBLE_TYPE)
			cout << names[i] << ":\t.double 0.0\t# " << TypeName(declaredType) << endl;
		else if(declaredType == CHAR_TYPE)
			cout << names[i] << ":\t.byte 0\t# " << TypeName(declaredType) << endl;
		else
			cout << names[i] << ":\t.quad 0\t# " << TypeName(declaredType) << endl;
	}
}

// VarDeclarationPart := "VAR" VarDeclaration {";" VarDeclaration} "."
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

// AssignementStatement := Identificateur ":=" Expression
string AssignementStatement(void){
	if(current != ID)
		Error("identificateur attendu a gauche de l'affectation");

	string variable = lexer->YYText();
	TYPE variableType = TypeOfIdentifier(variable);

	NextToken();

	if(current != ASSIGN)
		Error("':=' attendu apres l'identificateur");

	NextToken();

	TYPE expressionType = Expression();

	if(variableType != expressionType)
		TypeError("affectation impossible : " + variable + " est " + TypeName(variableType)
		          + " mais l'expression est " + TypeName(expressionType));

	if(variableType == CHAR_TYPE){
		cout << "\tpop %rax" << endl;
		cout << "\tmovb %al, " << variable << "(%rip)" << endl;
	}
	else{
		cout << "\tpopq " << variable << "(%rip)" << endl;
	}

	return variable;
}

// DisplayStatement := "DISPLAY" Expression
// Convention d'appel Windows x64 :
//   %rcx = chaine de format, %rdx = 2eme arg entier, %xmm1 = 2eme arg flottant
//   Shadow space obligatoire : 32 octets avant chaque call
void DisplayStatement(void){
	NextToken();  // consomme DISPLAY

	TYPE expressionType = Expression();

	if(expressionType == UNSIGNED_INT || expressionType == BOOLEAN){
		cout << "\tpop %rdx\t\t\t# valeur a afficher" << endl;
		cout << "\tleaq FormatInteger(%rip), %rcx" << endl;
		cout << "\tmovq %rsp, %r12\t\t\t# sauvegarder rsp" << endl;
		cout << "\tandq $-16, %rsp\t\t\t# aligner pile 16 octets" << endl;
		cout << "\tsubq $32, %rsp\t\t\t# shadow space Windows" << endl;
		cout << "\tcall printf" << endl;
		cout << "\tmovq %r12, %rsp\t\t\t# restaurer rsp" << endl;
	}
	else if(expressionType == CHAR_TYPE){
		cout << "\tpop %rdx\t\t\t# caractere a afficher" << endl;
		cout << "\tleaq FormatChar(%rip), %rcx" << endl;
		cout << "\tmovq %rsp, %r12\t\t\t# sauvegarder rsp" << endl;
		cout << "\tandq $-16, %rsp\t\t\t# aligner pile 16 octets" << endl;
		cout << "\tsubq $32, %rsp\t\t\t# shadow space Windows" << endl;
		cout << "\tcall printf" << endl;
		cout << "\tmovq %r12, %rsp\t\t\t# restaurer rsp" << endl;
	}
	else if(expressionType == DOUBLE_TYPE){
		cout << "\tmovq (%rsp), %rdx\t\t# lire bits du double" << endl;
		cout << "\tmovq %rdx, %xmm1\t\t# copier dans xmm1" << endl;
		cout << "\taddq $8, %rsp\t\t\t# depiler le double" << endl;
		cout << "\tleaq FormatDouble(%rip), %rcx" << endl;
		cout << "\tmovq %rsp, %r12\t\t\t# sauvegarder rsp" << endl;
		cout << "\tandq $-16, %rsp\t\t\t# aligner pile 16 octets" << endl;
		cout << "\tsubq $32, %rsp\t\t\t# shadow space Windows" << endl;
		cout << "\tcall printf" << endl;
		cout << "\tmovq %r12, %rsp\t\t\t# restaurer rsp" << endl;
	}
	else{
		TypeError("type non affichable par DISPLAY");
	}
}

// IfStatement := "IF" Expression "THEN" Statement ["ELSE" Statement]
void IfStatement(void){
	unsigned long n = ++labelNumber;

	NextToken();  // consomme IF

	TYPE conditionType = Expression();
	if(conditionType != BOOLEAN)
		TypeError("la condition du IF doit etre de type BOOLEAN");

	if(current != THEN_T)
		Error("THEN attendu apres la condition du IF");

	cout << "\tpop %rax" << endl;
	cout << "\tcmpq $0, %rax" << endl;
	cout << "\tje ElsePart" << n << endl;

	NextToken();  // consomme THEN
	Statement();

	if(current == ELSE_T){
		cout << "\tjmp EndIf" << n << endl;
		cout << "ElsePart" << n << ":" << endl;
		NextToken();  // consomme ELSE
		Statement();
		cout << "EndIf" << n << ":" << endl;
	}
	else{
		cout << "ElsePart" << n << ":" << endl;
	}
}

// WhileStatement := "WHILE" Expression "DO" Statement
void WhileStatement(void){
	unsigned long n = ++labelNumber;

	cout << "WhileBegin" << n << ":" << endl;

	NextToken();  // consomme WHILE

	TYPE conditionType = Expression();
	if(conditionType != BOOLEAN)
		TypeError("la condition du WHILE doit etre de type BOOLEAN");

	if(current != DO_T)
		Error("DO attendu apres la condition du WHILE");

	cout << "\tpop %rax" << endl;
	cout << "\tcmpq $0, %rax" << endl;
	cout << "\tje WhileEnd" << n << endl;

	NextToken();  // consomme DO
	Statement();

	cout << "\tjmp WhileBegin" << n << endl;
	cout << "WhileEnd" << n << ":" << endl;
}

// ForStatement := "FOR" AssignementStatement ("TO"|"DOWNTO") Expression "DO" Statement
void ForStatement(void){
	unsigned long n = ++labelNumber;

	NextToken();  // consomme FOR

	string variable = AssignementStatement();

	if(TypeOfIdentifier(variable) != UNSIGNED_INT)
		TypeError("la variable de boucle du FOR doit etre INTEGER");

	// Verifie si c'est TO ou DOWNTO
	bool isDownTo = false;
	if(current == TO_T){
		isDownTo = false;
	}
	else if(current == DOWNTO_T){
		isDownTo = true;
	}
	else{
		Error("TO ou DOWNTO attendu apres l'affectation du FOR");
	}

	// Label de debut de boucle (la borne limite est reevaluee a chaque iteration,
	// ce qui est correct pour les expressions simples)
	cout << "ForBegin" << n << ":" << endl;

	NextToken();  // consomme TO ou DOWNTO

	TYPE limitType = Expression();
	if(limitType != UNSIGNED_INT)
		TypeError("la borne du FOR doit etre de type INTEGER");

	if(current != DO_T)
		Error("DO attendu apres la borne du FOR");

	// Teste si on doit continuer la boucle
	cout << "\tpop %rbx\t\t\t# borne limite" << endl;
	cout << "\tmovq " << variable << "(%rip), %rax\t# valeur courante" << endl;
	cout << "\tcmpq %rbx, %rax" << endl;

	if(isDownTo)
		cout << "\tjb ForEnd" << n << "\t# DOWNTO : sortir si var < borne" << endl;
	else
		cout << "\tja ForEnd" << n << "\t# TO : sortir si var > borne" << endl;

	NextToken();  // consomme DO
	Statement();

	// Incremente ou decremente la variable de boucle
	if(isDownTo)
		cout << "\tsubq $1, " << variable << "(%rip)\t# DOWNTO : decrementation" << endl;
	else
		cout << "\taddq $1, " << variable << "(%rip)\t# TO : incrementation" << endl;

	cout << "\tjmp ForBegin" << n << endl;
	cout << "ForEnd" << n << ":" << endl;
}

// BlockStatement := "BEGIN" Statement {";" Statement} "END"
void BlockStatement(void){
	NextToken();  // consomme BEGIN

	Statement();

	while(current == SEMICOLON){
		NextToken();
		if(current == END_T)
			break;
		Statement();
	}

	if(current != END_T)
		Error("END attendu pour fermer le BEGIN");

	NextToken();  // consomme END
}

// CaseStatement := "CASE" Expression "OF" CaseElement {";" CaseElement} "END"
// CaseElement   := CaseLabelList ":" Statement | vide
// CaseLabelList := Constante {"," Constante}
//
// La valeur de l'expression reste sur la pile pendant tout le CASE.
// Chaque branche compare la valeur (sans la depiler) avec ses etiquettes.
// Apres execution d'une branche, on saute a la fin du CASE.
void CaseStatement(void){
	unsigned long n = ++labelNumber;

	NextToken();  // consomme CASE

	TYPE exprType = Expression();

	// CASE ne supporte que les types ordinaux
	if(exprType != UNSIGNED_INT && exprType != CHAR_TYPE && exprType != BOOLEAN)
		TypeError("l'expression du CASE doit etre INTEGER, CHAR ou BOOLEAN");

	if(current != OF_T)
		Error("OF attendu apres l'expression du CASE");

	NextToken();  // consomme OF

	// La valeur de l'expression est au sommet de la pile.
	// On la consulte sans la depiler dans chaque branche.

	while(current != END_T && current != FEOF){

		// Ignore les points-virgules entre les elements (elements vides)
		if(current == SEMICOLON){
			NextToken();
			continue;
		}

		if(current == END_T)
			break;

		// Genere deux etiquettes : une pour le corps, une pour passer au suivant
		unsigned long bodyTag = ++labelNumber;
		unsigned long nextTag = ++labelNumber;

		// Lit la liste d'etiquettes et genere les comparaisons
		while(true){
			long long constValue = 0;

			if(current == NUMBER){
				constValue = atoll(lexer->YYText());
				NextToken();
			}
			else if(current == CHARCONST){
				string s = lexer->YYText();
				constValue = (unsigned char)s[1];
				NextToken();
			}
			else{
				Error("constante entiere ou caractere attendue dans les etiquettes du CASE");
			}

			// Lit la valeur du CASE depuis le sommet de la pile (sans depiler)
			cout << "\tmovq (%rsp), %rax\t# lire valeur du CASE" << endl;
			cout << "\tcmpq $" << constValue << ", %rax" << endl;
			cout << "\tje CaseBody" << bodyTag << endl;

			if(current == COMMA)
				NextToken();  // plusieurs etiquettes pour le meme cas
			else
				break;
		}

		// Aucune etiquette ne correspond : passer a l'element suivant
		cout << "\tjmp CaseNext" << nextTag << endl;

		// Corps de cet element
		cout << "CaseBody" << bodyTag << ":" << endl;

		if(current != COLON)
			Error("':' attendu apres les etiquettes du CASE");

		NextToken();  // consomme ':'
		Statement();

		// Apres l'instruction, sauter a la fin du CASE
		cout << "\tjmp CaseEnd" << n << endl;

		cout << "CaseNext" << nextTag << ":" << endl;
	}

	// Fin du CASE : depiler la valeur de l'expression
	cout << "CaseEnd" << n << ":" << endl;
	cout << "\taddq $8, %rsp\t\t# depiler la valeur du CASE" << endl;

	if(current != END_T)
		Error("END attendu pour fermer le CASE");

	NextToken();  // consomme END
}

// Statement := AssignementStatement | IfStatement | WhileStatement
//            | ForStatement | BlockStatement | DisplayStatement | CaseStatement
void Statement(void){
	if(current == ID)
		AssignementStatement();
	else if(current == IF_T)
		IfStatement();
	else if(current == WHILE_T)
		WhileStatement();
	else if(current == FOR_T)
		ForStatement();
	else if(current == BEGIN_T)
		BlockStatement();
	else if(current == DISPLAY_T)
		DisplayStatement();
	else if(current == CASE_T)
		CaseStatement();
	else
		Error("instruction attendue (identificateur, IF, WHILE, FOR, BEGIN, DISPLAY ou CASE)");
}

// StatementPart := Statement {";" Statement} "."
void StatementPart(void){
	cout << "\t.text" << endl;
	cout << "\t.globl main" << endl;
	cout << "main:" << endl;
	cout << "\tmovq %rsp, %rbp\t\t# sauvegarde du sommet de pile" << endl;

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

// Program := [VarDeclarationPart] StatementPart
void Program(void){
	cout << "\t.data" << endl;
	cout << "\t.align 8" << endl;
	// Chaines de formatage pour printf
	cout << "FormatInteger:\t.string \"%llu\\n\"\t# entier non signe 64 bits" << endl;
	cout << "FormatDouble:\t.string \"%f\\n\"\t# flottant 64 bits" << endl;
	cout << "FormatChar:\t.string \"%c\\n\"\t# caractere" << endl;

	VarDeclarationPart();
	StatementPart();
}

int main(void){
	cout << "\t\t\t# Code produit par le compilateur CERI" << endl;

	NextToken();
	Program();

	cout << "\tmovq %rbp, %rsp\t\t# restauration du sommet de pile" << endl;
	cout << "\tret\t\t\t# retour de la fonction main" << endl;

	if(current != FEOF)
		Error("caracteres en trop a la fin du programme");

	return 0;
}
