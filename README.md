# MonCompilateur

Compilateur Pascal simplifié vers assembleur 80x86 64 bits (AT&T).
Réalisé dans le cadre du cours Assembleur et Compilation - S4, Université d'Avignon.

**Auteure : Kamelia Bouzourene**
Note : développé sous Windows avec MSYS2 UCRT64.
Le DISPLAY utilise la convention d'appel Windows x64.


**Pour compiler et tester :**

> make test

**Pour voir le code assembleur généré :**

> cat test.s

**Pour déboguer :**

> ddd ./test

**Pour envoyer vers GitHub :**

> git add .
> git commit -m "..."
> git push origin main



**Modifications et ajouts par rapport à la version de base :**

    Ajout des structures de contrôle : IF/THEN/ELSE, WHILE/DO, FOR/TO/DOWNTO/DO, BEGIN/END
    Ajout de la gestion des types : INTEGER, BOOLEAN, DOUBLE, CHAR
    Ajout des déclarations de variables typées avec VAR
    Ajout de l'instruction DISPLAY pour afficher les résultats
    Ajout de l'instruction CASE avec plusieurs étiquettes par branche
    Support du DOWNTO dans la boucle FOR

**Nouvelles méthodes ajoutées :**

    void IfStatement()
    void WhileStatement()
    void ForStatement()
    void BlockStatement()
    void DisplayStatement()
    void CaseStatement()
    void VarDeclarationPart()
    void OneVarDeclaration()

**Méthodes modifiées :**

    TYPE Expression()
    TYPE Term()
    TYPE SimpleExpression()
    TYPE Factor()
    void Statement()
    void Program()



**Grammaire supportée :**

Program          := [VarDeclarationPart] StatementPart
VarDeclarationPart := "VAR" VarDeclaration {";" VarDeclaration} "."
VarDeclaration   := Ident {"," Ident} ":" Type
Type             := "INTEGER" | "BOOLEAN" | "DOUBLE" | "CHAR"
StatementPart    := Statement {";" Statement} "."
Statement        := AssignementStatement | IfStatement | WhileStatement
                  | ForStatement | BlockStatement | DisplayStatement | CaseStatement
AssignementStatement := Ident ":=" Expression
DisplayStatement := "DISPLAY" Expression
IfStatement      := "IF" Expression "THEN" Statement ["ELSE" Statement]
WhileStatement   := "WHILE" Expression "DO" Statement
ForStatement     := "FOR" Ident ":=" Expression ("TO"|"DOWNTO") Expression "DO" Statement
BlockStatement   := "BEGIN" Statement {";" Statement} "END"
CaseStatement    := "CASE" Expression "OF" CaseElement {";" CaseElement} "END"
CaseElement      := CaseLabelList ":" Statement
CaseLabelList    := Constante {"," Constante}
Expression       := SimpleExpression [OpRel SimpleExpression]
SimpleExpression := Term {OpAdd Term}
Term             := Factor {OpMul Factor}
Factor           := Nombre | Ident | "(" Expression ")" | "!" Factor
AdditiveOperator := "+" | "-" | "||"
MultiplicativeOperator := "*" | "/" | "%" | "&&"
RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="



**Exemples de programmes Pascal compilables :**

**Exemple 1 : Boucle FOR avec TO et DOWNTO**

VAR i : INTEGER.
FOR i := 1 TO 5 DO
  DISPLAY i.

**Exemple 2 : Instruction CASE**

VAR note : INTEGER.
note := 3;
CASE note OF
  1, 2 : DISPLAY note;
  3, 4 : DISPLAY note;
  5    : DISPLAY note
END.

**Exemple 3 : Types DOUBLE et CHAR**

VAR x : DOUBLE;
    c : CHAR.
x := 1.5 + 2.5;
c := 'A';
DISPLAY x;
DISPLAY c.