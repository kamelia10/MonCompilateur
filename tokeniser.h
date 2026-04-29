#ifndef TOKENISER_H
#define TOKENISER_H

enum TOKEN {
    FEOF,
    UNKNOWN,

    NUMBER,
    ID,
    STRINGCONST,

    VAR_T,
    INTEGER_T,
    BOOLEAN_T,

    IF_T,
    THEN_T,
    ELSE_T,

    WHILE_T,
    DO_T,

    FOR_T,
    TO_T,
    DOWNTO_T,

    BEGIN_T,
    END_T,

    DISPLAY_T,

    RBRACKET,
    LBRACKET,
    RPARENT,
    LPARENT,

    COMMA,
    SEMICOLON,
    DOT,
    COLON,

    ADDOP,
    MULOP,
    RELOP,
    NOT,
    ASSIGN
};

#endif