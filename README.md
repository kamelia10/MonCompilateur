# MonCompilateur

compilateur pascal simplifié -> assembleur x86 64 bits
cours assembleur S4 - université d'Avignon
auteure : Kamelia Bouzourene

Note : développé sous Windows avec MSYS2 UCRT64. 
Le DISPLAY utilise la convention d'appel Windows x64.

---

**compiler et tester :**

> make test

**voir le code asm généré :**

> cat test.s

**débugger :**

> ddd ./test

---

**ce que j'ai ajouté par rapport à la version de base :**

par rapport au code de départ donné par le prof, j'ai ajouté et modifié les choses suivantes :

nouvelles méthodes que j'ai écrites moi-même :

- IfStatement() : gère le IF/THEN et IF/THEN/ELSE
- WhileStatement() : boucle while avec vérification que la condition est bien un booléen
- ForStatement() : boucle for avec TO et DOWNTO (incrémentation et décrémentation)
- BlockStatement() : le BEGIN/END pour grouper plusieurs instructions
- DisplayStatement() : affiche les valeurs, fonctionne pour INTEGER BOOLEAN DOUBLE et CHAR
- CaseStatement() : instruction CASE, on peut mettre plusieurs étiquettes séparées par des virgules genre "1, 2, 3 : instruction"
- VarDeclarationPart() et OneVarDeclaration() : pour les déclarations VAR avec les types

méthodes que j'ai modifiées :

- Expression(), Term(), SimpleExpression(), Factor() : j'ai ajouté le retour de type et la vérification de compatibilité des types, et aussi le support pour DOUBLE (calculs avec le FPU x87) et CHAR
- Statement() : j'ai ajouté tous les nouveaux cas IF WHILE FOR BEGIN DISPLAY CASE
- Program() : utilise maintenant VarDeclarationPart

autres modifications :

- dans tokeniser.h j'ai ajouté CASE_T et OF_T
- dans tokeniser.l j'ai ajouté les règles pour reconnaître CASE et OF
- j'ai changé le set<string> en map<string, TYPE> pour pouvoir stocker le type de chaque variable

---

**grammaire :**

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

---

**exemples qui compilent correctement :**

exemple 1 - boucle for avec to :

    VAR i : INTEGER.
    FOR i := 1 TO 5 DO
      DISPLAY i.

exemple 2 - boucle for avec downto :

    VAR i : INTEGER.
    FOR i := 5 DOWNTO 1 DO
      DISPLAY i.

exemple 3 - case avec plusieurs etiquettes :

    VAR a : INTEGER.
    a := 2;
    CASE a OF
      1, 2 : DISPLAY a;
      3    : DISPLAY a
    END.

exemple 4 - double et char :

    VAR x : DOUBLE;
        c : CHAR.
    x := 1.5 + 2.5;
    c := 'A';
    DISPLAY x;
    DISPLAY c.

exemple 5 - if et while :

    VAR a, b : INTEGER;
        ok : BOOLEAN.
    a := 10;
    b := 3;
    ok := a > b;
    IF ok THEN
      DISPLAY a
    ELSE
      DISPLAY b;
    WHILE b < 5 DO
    BEGIN
      b := b + 1;
      DISPLAY b
    END.
