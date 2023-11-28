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
    TOKEN_CHAR_CONSTANT,

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

    // boolean operators (&&, ||, !)
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
    TOKEN_KEYWORD_TRUE,
    TOKEN_KEYWORD_FALSE,

    TOKEN_LEFT_PAREN, // '('
    TOKEN_RIGHT_PAREN, // ')'
    TOKEN_LEFT_BRACE, // '{'
    TOKEN_RIGHT_BRACE, // '}'
    TOKEN_LEFT_BRACKET, // '['
    TOKEN_RIGHT_BRACKET, // ']'
    TOKEN_COLON, // ':'
    TOKEN_SEMICOLON, // ';' 
    TOKEN_COMMA, // ','
    TOKEN_DOT, // '.'

    TOKEN_PROGRAM_END
};

typedef struct String {
    char *data;
    unsigned int length;    
} String;

typedef struct LoadedFile {
    String source;
    String path;
    bool isLoaded;
} LoadedFile;

typedef struct {
    char *keywordString;
    unsigned int len;
    unsigned int tokenType;
} Keyword;

typedef struct {
    unsigned int type;

    union {
        int integerValue;
        char characterValue;
        char *identifier;
        char *stringValue;
    };

    unsigned int size;
    unsigned int pos;
    unsigned int column;
    unsigned int line;
} Token;

typedef struct {
    Token *tokens;
    unsigned int count;
} TokenList;

typedef struct {
    char *source;
    unsigned int pos;
    unsigned int line;
    unsigned int column;
    LoadedFile loadedFile;
} Lexer;

void PrintErrorLocationInSource(LoadedFile loadedFile, unsigned int location, unsigned int lineNumber, unsigned int column, char *errorMsg);

#endif