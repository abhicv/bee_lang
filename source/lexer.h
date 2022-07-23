#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

enum TokenType
{
    // constants
    TOKEN_INTEGER_CONSTANT,
    TOKEN_STRING_CONSTANT,

    // name of variable, user defined type, function
    TOKEN_IDENTIFIER,

    // arithmetic operators (+, -, *, /, %)
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_MODULUS,

    // relational operators (<, >, ==, !=, <=, >=)
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_EQ_EQ,
    TOKEN_NOT_EQ,
    TOKEN_LT_EQ,
    TOKEN_GT_EQ,

    // boolean operators (and, or, not)
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,

    // assignment operator (=)
    TOKEN_EQUAL,

    // keywords
    TOKEN_KEYWORD_FN,
    TOKEN_KEYWORD_STRUCT,
    TOKEN_KEYWORD_IF,
    TOKEN_KEYWORD_ELSE,
    TOKEN_KEYWORD_WHILE,
    TOKEN_KEYWORD_LET,
    TOKEN_KEYWORD_RETURN,

    TOKEN_OPEN_BRACKET,
    TOKEN_CLOSE_BRACKET,
    TOKEN_OPEN_CURLY_BRACKET,
    TOKEN_CLOSE_CURLY_BRACKET,
    TOKEN_COLON,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,

    TOKEN_PROGRAM_END,

    TOKEN_COUNT,
};

typedef struct Keyword Keyword;
struct Keyword
{
    char *keywordString;
    unsigned int len;
    unsigned int tokenType;
};

typedef struct Token Token;
struct Token
{
    unsigned int type;
    
    // token data
    int integerValue;
    char *identifier;
    char *stringValue;
    unsigned int opType;

    // pos data
    unsigned int size;
    unsigned int column;
    unsigned int line;
};

typedef struct Lexer Lexer;
struct Lexer
{
    const char *source;
    unsigned int pos;
    unsigned int line;
    unsigned int column;
};

typedef struct TokenList TokenList;
struct TokenList
{
    Token *tokens;
    unsigned int count;
};

#endif