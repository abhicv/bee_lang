#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "lexer.h"
#include "ast.h"

typedef struct {
    const char *fileName;
    const char *source;   
    TokenList tokenList;
    unsigned int tokenIndex;
} Parser;


#endif //PARSER_H