#include "lexer.h"
#include "ast.h"

Keyword keywordList[] = {
    {.keywordString = "fn", .len = 2, .tokenType = TOKEN_KEYWORD_FN},
    {.keywordString = "struct", .len = 6, .tokenType = TOKEN_KEYWORD_STRUCT},
    {.keywordString = "if", .len = 2, .tokenType = TOKEN_KEYWORD_IF},
    {.keywordString = "else", .len = 4, .tokenType = TOKEN_KEYWORD_ELSE},
    {.keywordString = "while", .len = 5, .tokenType = TOKEN_KEYWORD_WHILE},
    {.keywordString = "return", .len = 6, .tokenType = TOKEN_KEYWORD_RETURN},
    {.keywordString = "let", .len = 3, .tokenType = TOKEN_KEYWORD_LET},
    {.keywordString = "true", .len = 4, .tokenType = TOKEN_KEYWORD_TRUE},
    {.keywordString = "false", .len = 5, .tokenType = TOKEN_KEYWORD_FALSE},
};

char GetNextCharacter(Lexer *lexer)
{
    return lexer->source[lexer->pos++];
}

char PeekNextCharacter(Lexer *lexer)
{
    return lexer->source[lexer->pos];
}

LoadedFile LoadFileNullTerminated(const char *fileName)
{
    LoadedFile loadedFile = {0};
    loadedFile.isLoaded = false;

    FILE *input = fopen(fileName, "r");
    if(input)
    {
        fseek(input, 0, SEEK_END);
        unsigned int size = ftell(input);
        fseek(input, 0, SEEK_SET);
        loadedFile.data = (char*)malloc(size + 1);
        fread(loadedFile.data, sizeof(char), size, input);
        loadedFile.data[size] = 0;
        loadedFile.size = size;
        loadedFile.isLoaded = true;
        fclose(input);
    }
    else
    {
        printf("error: failed to open input file '%s'\n", fileName);
    }
    
    return loadedFile;
}

bool IsNumeralCharacter(char c)
{
    return (c >= '0' && c <= '9');
}

bool IsAlphabetCharacter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool IsIdentifierCharacter(char c)
{
    return IsAlphabetCharacter(c) || (c == '_');
}

bool IsBinaryOperatorCharacter(char c)
{
    return (c == '+') || (c == '-') || (c == '*') || (c == '/') || (c == '%');
}

bool IsVisibleCharacter(char c)
{
    return (c >= 33 && c <= 126);
}

bool IsWhiteSpaceCharacter(char c)
{
    return (c == '\n') || (c == '\r') || (c == '\t') || (c == ' ');
}

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void PrintErrorLocationInSource(char *source, unsigned int location, unsigned int lineNumber, unsigned int column, char *errorMsg)
{
    printf("%u:%u "  ANSI_COLOR_RED "error:" ANSI_COLOR_RESET "%s\n", lineNumber, column, errorMsg);

    int size = strlen(source);
    if (location >= size) return;

    int leftPos = location;
    int rightPos = location;

    while(source[leftPos] != '\n' && leftPos >= 0) leftPos--;
    while(source[rightPos] != '\n' && rightPos < size) rightPos++;

    leftPos++;
    rightPos--;

    // printf("location: %d, left: %d, right: %d\n", location, leftPos, rightPos);
    // printf("location: %d, left: %d, right: %d\n", source[location], source[leftPos], source[rightPos]);

    int lineSize = rightPos - leftPos + 1;
    char *line = (char*)malloc(lineSize);
    strncpy(line, source + leftPos, lineSize);

    printf("\t | %s\n", line);
    printf("\t | ");
    for(int n = 0; n < location - leftPos; n++) printf(" ");
    printf(ANSI_COLOR_RED);
    printf("^");
    for(int n = 0; n < rightPos - location; n++) printf("-");
    printf(ANSI_COLOR_RESET);
    printf("\n");
    free(line);
}

Token TokenizeIntegerConstant(Lexer *lexer)
{
    unsigned int start = lexer->pos;
    
    int value = 0;
    while(true)
    {
        char character = PeekNextCharacter(lexer);
        if(IsNumeralCharacter(character))
        {
            character = GetNextCharacter(lexer);
            value = (value * 10) + (character - '0');
        }
        else
        {
            break;
        }
    }
    
    char character = PeekNextCharacter(lexer);
    if(IsIdentifierCharacter(character))
    {
        PrintErrorLocationInSource(lexer->source, lexer->pos, lexer->line+1, lexer->column+1, "invalid suffix for integer constant");
        exit(1);
    }
    
    unsigned int end = lexer->pos;
    
    Token token = {0};
    token.type = TOKEN_INTEGER_CONSTANT;
    token.integerValue = value;
    token.size = end - start;
    token.column = lexer->column;
    token.line = lexer->line;
    
    return token;
}

Token TokenizeStringLiteral(Lexer *lexer)
{   
    GetNextCharacter(lexer);

    unsigned int len = 0;
    unsigned int start = lexer->pos;
    
    while(true)
    {
        char character = PeekNextCharacter(lexer);

        if(character == '\"')
        {
            break;
        }
        else if(IsVisibleCharacter(character) || IsWhiteSpaceCharacter(character))
        {
            GetNextCharacter(lexer);
            len++;
        }
        else if(character == 0)
        {
            PrintErrorLocationInSource(lexer->source, start, lexer->line+1, lexer->column+1, "string literal missing terminating character '\"'");
            exit(1);
        }
    }

    GetNextCharacter(lexer);

    if(len == 0)
    {
        PrintErrorLocationInSource(lexer->source, start, lexer->line+1, lexer->column+1, "string literal cannot be empty");
        exit(1);
    }

    Token token = {0};
    token.type = TOKEN_STRING_CONSTANT;
    token.size = len;
    token.column = lexer->column;
    token.line = lexer->line;

    token.stringValue = (char*)malloc(len + 1);
    strncpy(token.stringValue, &lexer->source[start], len);
    token.stringValue[len] = 0;

    return token;
}

Token TokenizeIdentifier(Lexer *lexer)
{
    unsigned int len = 0;
    unsigned int start = lexer->pos;
    
    while(true)
    {
        char character = PeekNextCharacter(lexer);
        if(IsIdentifierCharacter(character) || IsNumeralCharacter(character)) 
        {
            GetNextCharacter(lexer);
            len++;
        }
        else 
        {
            break;
        }
    }

    Token token = {0};
    token.type = TOKEN_IDENTIFIER;
    token.size = len;
    token.column = lexer->column;
    token.line = lexer->line;
    
    token.identifier = (char*)malloc(len + 1);
    strncpy(token.identifier, &lexer->source[start], len);
    token.identifier[len] = 0;
    
    return token;
}

bool TryTokenizeKeyword(Lexer *lexer, Token *token)
{
    unsigned int len = 0;
    
    unsigned int start = lexer->pos;
    
    while(true)
    {
        char character = GetNextCharacter(lexer);
        if(IsAlphabetCharacter(character)) len++;
        else break;
    }
    
    lexer->pos = start;
    
    unsigned int keywordListSize = sizeof(keywordList) / sizeof(keywordList[0]);
    
    bool keywordMatched = false;
    unsigned int index = 0;
    
    for(unsigned int n = 0; n < keywordListSize; n++)
    {
        if(len == keywordList[n].len)
        {
            if(!strncmp(&lexer->source[start], keywordList[n].keywordString, len))
            {
                keywordMatched = true;
                index = n;
                lexer->pos = start + len;
                break;
            }
        }
    }
    
    token->column = lexer->column;
    token->line = lexer->line;
    token->size = len;
    token->type = keywordList[index].tokenType;
    
    return keywordMatched;
}

void PushToken(TokenList *tokenList, Token token)
{
    if(tokenList->count == 0)
    {
        tokenList->count++;
        tokenList->tokens = (Token *)malloc(sizeof(Token));
    }
    else
    {
        tokenList->count++;
        tokenList->tokens = (Token *)realloc(tokenList->tokens, sizeof(Token) * tokenList->count);
    }
    
    tokenList->tokens[tokenList->count - 1] = token;
}

TokenList TokenizeSource(char *source)
{
    TokenList tokenList = {0};
    
    Lexer lexer = {0};
    lexer.source = source;
    
    while(true)
    {
        char character = PeekNextCharacter(&lexer);

        if(IsNumeralCharacter(character))
        {
            Token token = TokenizeIntegerConstant(&lexer);
            lexer.column += token.size;
            token.pos = lexer.pos;
            PushToken(&tokenList, token);
        }
        else if(IsIdentifierCharacter(character))
        {
            Token token = {0};
            bool isKeyword = TryTokenizeKeyword(&lexer, &token);
            if(!isKeyword)
            {
                token = TokenizeIdentifier(&lexer);
            }

            lexer.column += token.size;
            token.pos = lexer.pos;
            PushToken(&tokenList, token);
        }
        else if(character == '+')
        {
            GetNextCharacter(&lexer);

            Token token = {0};
            token.type = TOKEN_PLUS;
            token.opType = ARITHMETIC_OP_ADD;
            token.line = lexer.line;
            token.column = lexer.column;
            token.size = 1;
            
            lexer.column += token.size;
            token.pos = lexer.pos;
            
            PushToken(&tokenList, token);
        }
        else if(character == '-')
        {
            GetNextCharacter(&lexer);

            Token token = {0};
            token.type = TOKEN_MINUS;
            token.opType = ARITHMETIC_OP_SUB;
            token.line = lexer.line;
            token.column = lexer.column;
            token.size = 1;
            
            lexer.column += token.size;
            token.pos = lexer.pos;
            
            PushToken(&tokenList, token);
        }
        else if(character == '*')
        {
            GetNextCharacter(&lexer);

            Token token = {0};
            token.type = TOKEN_MULTIPLY;
            token.opType = ARITHMETIC_OP_MUL;
            token.line = lexer.line;
            token.column = lexer.column;
            token.size = 1;
            lexer.column += token.size;
            token.pos = lexer.pos;

            PushToken(&tokenList, token);
        }
        else if(character == '/')
        {
            GetNextCharacter(&lexer);
            char c = PeekNextCharacter(&lexer);

            // single line comment            
            if(c == '/') {
                while(true)
                {
                    c = PeekNextCharacter(&lexer);
                    if(c == '\n' || c == 0)
                    {
                        break;
                    }
                    GetNextCharacter(&lexer);
                }
            } else {
                Token token = {0};
                token.type = TOKEN_DIVIDE;
                token.opType = ARITHMETIC_OP_DIV;
                token.line = lexer.line;
                token.column = lexer.column;
                token.size = 1;
                lexer.column += token.size;    
                token.pos = lexer.pos;
                PushToken(&tokenList, token);
            }
        }
        else if(character == '%')
        {
            GetNextCharacter(&lexer);

            Token token = {0};
            token.type = TOKEN_MODULUS;
            token.opType = ARITHMETIC_OP_MOD;
            token.line = lexer.line;
            token.column = lexer.column;
            token.size = 1;

            lexer.column += token.size;
            token.pos = lexer.pos;
            
            PushToken(&tokenList, token);
        }
        else if(character == '\"') // string literal
        {
            Token token = TokenizeStringLiteral(&lexer);
            lexer.column += token.size + 2;
            token.pos = lexer.pos;
            PushToken(&tokenList, token);
        }
        else if(character == '=')
        {
            GetNextCharacter(&lexer);

            Token token = {0};
            
            char c = PeekNextCharacter(&lexer);
            if(c == '=')
            {
                GetNextCharacter(&lexer);
                token.type = TOKEN_EQ_EQ;
                token.opType = COMPARE_OP_EQ_EQ;
                token.line = lexer.line;
                token.column = lexer.column;
                token.size = 2;
            }
            else
            {
                token.type = TOKEN_EQUAL;
                token.line = lexer.line;
                token.column = lexer.column;
                token.size = 1;
            }

            lexer.column += token.size;
            token.pos = lexer.pos;
            PushToken(&tokenList, token);
        }
        else if(character == '<')
        {
            GetNextCharacter(&lexer);

            Token token = {0};
            
            char c = PeekNextCharacter(&lexer);
            if(c == '=')
            {
                GetNextCharacter(&lexer);
                token.type = TOKEN_LT_EQ;
                token.opType = COMPARE_OP_LT_EQ;
                token.line = lexer.line;
                token.column = lexer.column;
                token.size = 2;
            }
            else
            {
                token.type = TOKEN_LT;
                token.opType = COMPARE_OP_LT;
                token.line = lexer.line;
                token.column = lexer.column;
                token.size = 1;
            }

            lexer.column += token.size;
            token.pos = lexer.pos;

            PushToken(&tokenList, token);
        }
        else if(character == '>')
        {
            GetNextCharacter(&lexer);

            Token token = {0};
            
            char c = PeekNextCharacter(&lexer);
            if(c == '=')
            {
                GetNextCharacter(&lexer);
                token.type = TOKEN_GT_EQ;
                token.opType = COMPARE_OP_GT_EQ;
                token.line = lexer.line;
                token.column = lexer.column;
                token.size = 2;
            }
            else
            {
                token.type = TOKEN_GT;
                token.opType = COMPARE_OP_GT;
                token.line = lexer.line;
                token.column = lexer.column;
                token.size = 1;
            }

            lexer.column += token.size;
            token.pos = lexer.pos;
            
            PushToken(&tokenList, token);
        }
        else if(character == '!')
        {
            GetNextCharacter(&lexer);
            char c = PeekNextCharacter(&lexer);

            Token token = {0};
            
            if(c == '=')
            {
                GetNextCharacter(&lexer);
                token.type = TOKEN_GT_EQ;
                token.opType = COMPARE_OP_NOT_EQ;
                token.line = lexer.line;
                token.column = lexer.column;
                token.size = 2;
            }
            else 
            {
                token.type = TOKEN_NOT;
                token.opType = BOOL_OP_NOT;
                token.line = lexer.line;
                token.column = lexer.column;
                token.size = 1;
            }

            lexer.column += token.size;
            token.pos = lexer.pos;
            
            PushToken(&tokenList, token);
        }
        else if(character == '&')
        {
            GetNextCharacter(&lexer);
            char c = PeekNextCharacter(&lexer);
            
            if(c != '&')
            {
                PrintErrorLocationInSource(lexer.source, lexer.pos - 1, lexer.line + 1, lexer.column + 1, "found '&' expected '&&'");
                exit(1);
            }

            GetNextCharacter(&lexer);
            
            Token token = {0};
            token.type = TOKEN_AND;
            token.opType = BOOL_OP_AND;
            token.line = lexer.line;
            token.column = lexer.column;
            token.size = 2;
         
            lexer.column += token.size;
            token.pos = lexer.pos;
            
            PushToken(&tokenList, token);
        }
        else if(character == '|')
        {
            GetNextCharacter(&lexer);
            char c = PeekNextCharacter(&lexer);

            if(c != '|')
            {
                PrintErrorLocationInSource(lexer.source, lexer.pos - 1, lexer.line + 1, lexer.column + 1, "found '|' expected '||'");
                exit(1);
            }

            GetNextCharacter(&lexer);
            
            Token token = {0};
            token.type = TOKEN_OR;
            token.opType = BOOL_OP_OR;
            token.line = lexer.line;
            token.column = lexer.column;
            token.size = 2;
         
            lexer.column += token.size;
            token.pos = lexer.pos;
            
            PushToken(&tokenList, token);
        }
        else if(character == '(')
        {
            GetNextCharacter(&lexer);

            Token token = {0};
            token.type = TOKEN_LEFT_PAREN;
            token.line = lexer.line;
            token.column = lexer.column;
            token.size = 1;
            
            lexer.column += token.size;
            token.pos = lexer.pos;
            
            PushToken(&tokenList, token);
        }
        else if(character == ')')
        {
            GetNextCharacter(&lexer);

            Token token = {0};
            token.type = TOKEN_RIGHT_PAREN;
            token.line = lexer.line;
            token.column = lexer.column;
            token.size = 1;
            
            lexer.column += token.size;
            token.pos = lexer.pos;
            
            PushToken(&tokenList, token);
        }
        else if(character == '{')
        {
            GetNextCharacter(&lexer);

            Token token = {0};
            token.type = TOKEN_LEFT_BRACE;
            token.line = lexer.line;
            token.column = lexer.column;
            token.size = 1;
            
            lexer.column += token.size;
            token.pos = lexer.pos;
            
            PushToken(&tokenList, token);
        }
        else if(character == '}')
        {
            GetNextCharacter(&lexer);

            Token token = {0};
            token.type = TOKEN_RIGHT_BRACE;
            token.line = lexer.line;
            token.column = lexer.column;
            token.size = 1;
            
            lexer.column += token.size;
            token.pos = lexer.pos;
            
            PushToken(&tokenList, token);
        }
        else if(character == '[')
        {
            GetNextCharacter(&lexer);

            Token token = {0};
            token.type = TOKEN_LEFT_BRACKET;
            token.line = lexer.line;
            token.column = lexer.column;
            token.size = 1;
            
            lexer.column += token.size;
            token.pos = lexer.pos;
            
            PushToken(&tokenList, token);
        }
        else if(character == ']')
        {
            GetNextCharacter(&lexer);

            Token token = {0};
            token.type = TOKEN_RIGHT_BRACKET;
            token.line = lexer.line;
            token.column = lexer.column;
            token.size = 1;
            
            lexer.column += token.size;
            token.pos = lexer.pos;
            
            PushToken(&tokenList, token);
        }
        else if(character == ';')
        {
            GetNextCharacter(&lexer);

            Token token = {0};
            token.type = TOKEN_SEMICOLON;
            token.line = lexer.line;
            token.column = lexer.column;
            token.size = 1;
            
            lexer.column += token.size;
            token.pos = lexer.pos;
            
            PushToken(&tokenList, token);
        }
        else if(character == ',')
        {
            GetNextCharacter(&lexer);

            Token token = {0};
            token.type = TOKEN_COMMA;
            token.line = lexer.line;
            token.column = lexer.column;
            token.size = 1;
            
            lexer.column += token.size;
            token.pos = lexer.pos;
            
            PushToken(&tokenList, token);
        }
        else if(character == ':')
        {            
            GetNextCharacter(&lexer);

            Token token = {0};
            token.type = TOKEN_COLON;
            token.line = lexer.line;
            token.column = lexer.column;
            token.size = 1;
            
            lexer.column += token.size;
            token.pos = lexer.pos;
            PushToken(&tokenList, token);
        }
        else if(character == '.')
        {            
            GetNextCharacter(&lexer);

            Token token = {0};
            token.type = TOKEN_DOT;
            token.line = lexer.line;
            token.column = lexer.column;
            token.size = 1;
            
            lexer.column += token.size;
            token.pos = lexer.pos;
            PushToken(&tokenList, token);
        }
        else if(character == 0)
        {
            Token token = {0};
            token.type = TOKEN_PROGRAM_END;
            token.column = lexer.column;
            token.line = lexer.line;
            token.pos = lexer.pos;
            PushToken(&tokenList, token);
            break;
        }
        else if(character == '\n')
        {
            lexer.line++;
            lexer.column = 0;
            GetNextCharacter(&lexer);
        }
        else if(character == ' ')
        {
            lexer.column++;
            GetNextCharacter(&lexer);
        }
        else
        {
            printf("%u:%u: error: unsupported character '%c'\n",  lexer.line + 1, lexer.column + 1, character);
            exit(1);
        }
    }
    
    return tokenList;
}

char *TokenTypeToString(unsigned int type)
{
    switch(type)
    {
        case TOKEN_INTEGER_CONSTANT:        return "token_integer_constant"; break;
        case TOKEN_STRING_CONSTANT:         return "token_string_constant"; break;
        case TOKEN_IDENTIFIER:              return "token_identifier"; break;
        case TOKEN_PLUS:                    return "token_plus"; break;
        case TOKEN_MINUS:                   return "token_minus"; break;
        case TOKEN_MULTIPLY:                return "token_multiply"; break;
        case TOKEN_DIVIDE:                  return "token_divide"; break;
        case TOKEN_MODULUS:                 return "token_modulus"; break;
        case TOKEN_EQUAL:                   return "token_equal"; break;
        case TOKEN_LT:                      return "token_less_than"; break;
        case TOKEN_GT:                      return "token_greater_than"; break;
        case TOKEN_LT_EQ:                   return "token_less_than_or_equal"; break;
        case TOKEN_GT_EQ:                   return "token_greater_than_or_equal"; break;
        case TOKEN_EQ_EQ:                   return "token_equal_equal"; break;
        case TOKEN_NOT_EQ:                  return "token_not_equal"; break;
        case TOKEN_AND:                     return "token_and"; break;
        case TOKEN_OR:                      return "token_or"; break;
        case TOKEN_NOT:                     return "token_not"; break;
        case TOKEN_KEYWORD_FN:              return "token_keyword_fn"; break;
        case TOKEN_KEYWORD_STRUCT:          return "token_keyword_struct"; break;
        case TOKEN_KEYWORD_IF:              return "token_keyword_if"; break;
        case TOKEN_KEYWORD_ELSE:            return "token_keyword_else"; break;
        case TOKEN_KEYWORD_WHILE:           return "token_keyword_while"; break;
        case TOKEN_KEYWORD_RETURN:          return "token_keyword_return"; break;
        case TOKEN_KEYWORD_LET:             return "token_keyword_let"; break;
        case TOKEN_KEYWORD_TRUE:            return "token_keyword_true"; break;
        case TOKEN_KEYWORD_FALSE:           return "token_keyword_false"; break;
        case TOKEN_LEFT_PAREN:              return "token_left_paren"; break;
        case TOKEN_RIGHT_PAREN:             return "token_right_paren"; break;
        case TOKEN_LEFT_BRACE:              return "token_left_brace"; break;
        case TOKEN_RIGHT_BRACE:             return "token_right_brace"; break;
        case TOKEN_LEFT_BRACKET:            return "token_left_bracket"; break;
        case TOKEN_RIGHT_BRACKET:           return "token_right_bracket"; break;
        case TOKEN_COLON:                   return "token_colon"; break;
        case TOKEN_SEMICOLON:               return "token_semicolon"; break;
        case TOKEN_COMMA:                   return "token_comma"; break;
        case TOKEN_DOT:                     return "token_dot"; break;
        case TOKEN_PROGRAM_END:             return "token_program_end"; break;
        case TOKEN_TYPE_COUNT:              return "token_count"; break;
        default:                            return "unknown_token_type";
    }
}

void PrintTokenInfo(Token token)
{
    printf("%u:%u:%u type: '%s', size: %u\n", token.line+1, token.column+1, token.pos, TokenTypeToString(token.type), token.size);
}
