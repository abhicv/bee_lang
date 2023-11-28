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

    String path = {0};
    path.length = strlen(fileName);
    path.data = (char*)malloc(path.length + 1);
    strncpy(path.data, fileName, path.length);
    path.data[path.length] = 0;

    loadedFile.path = path;

    FILE *file = fopen(fileName, "r");
    if(file)
    {
        fseek(file, 0, SEEK_END);
        unsigned int size = ftell(file);
        fseek(file, 0, SEEK_SET);

        String source = {0};
        source.data = (char*)malloc(size + 1);
        fread(source.data, sizeof(char), size, file);
        source.data[size] = 0;
        source.length = size;
        
        fclose(file);

        loadedFile.source = source;
        loadedFile.isLoaded = true;
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

void PrintErrorLocationInSource(LoadedFile loadedFile, unsigned int location, unsigned int lineNumber, unsigned int column, char *errorMsg)
{
    printf("%s:%u:%u: error: %s\n", loadedFile.path.data, lineNumber, column, errorMsg);

    if (location >= loadedFile.source.length) return;

    char *source = loadedFile.source.data;

    int leftPos = location;
    int rightPos = location;

    while(source[leftPos] != '\n' && leftPos >= 0) leftPos--;
    while(source[rightPos] != '\n' && rightPos < loadedFile.source.length) rightPos++;

    leftPos++;
    rightPos--;

    // printf("location: %d, left: %d, right: %d\n", location, leftPos, rightPos);
    // printf("location: '%c'(%d), left: '%c'(%d), right: '%c'(%d)\n", source[location], source[location], source[leftPos], source[leftPos], source[rightPos], source[rightPos]);
    // printf("line size: %u\n", rightPos - leftPos + 1);

    int lineSize = rightPos - leftPos + 1;
    char *line = (char*)malloc(lineSize + 1);
    strncpy(line, source + leftPos, lineSize);
    line[lineSize] = 0;

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
        PrintErrorLocationInSource(lexer->loadedFile, lexer->pos, lexer->line+1, lexer->column+1, "invalid suffix for integer constant");
        exit(1);
    }
    
    unsigned int end = lexer->pos;
    
    Token token = {0};
    token.type = TOKEN_INTEGER_CONSTANT;
    token.integerValue = value;
    token.size = end - start;
    token.column = lexer->column;
    token.line = lexer->line;
    token.pos = start;    

    return token;
}

Token TokenizeCharacterConstant(Lexer *lexer)
{
    GetNextCharacter(lexer);

    unsigned int len = 0;
    unsigned int start = lexer->pos;

    while(true)
    {
        char character = PeekNextCharacter(lexer);

        if(character == '\'')
        {
            break;
        }
        else if(IsVisibleCharacter(character))
        {
            GetNextCharacter(lexer);
            len++;
        }
        else if(character == 0 || IsWhiteSpaceCharacter(character))
        {
            PrintErrorLocationInSource(lexer->loadedFile, start - 1, lexer->line + 1, lexer->column + 1, "string literal missing terminating character '\''");
            exit(1);
        }

        if(len > 1) 
        {
            PrintErrorLocationInSource(lexer->loadedFile, start - 1, lexer->line + 1, lexer->column + 1, "multi-character character constant found");
            exit(1);
        }
    }

    GetNextCharacter(lexer);

    Token token = {0};
    token.type = TOKEN_CHAR_CONSTANT;
    token.characterValue = lexer->source[start];
    token.line = lexer->line;
    token.column = lexer->column;
    token.pos = start - 1;
    token.size = len + 2;
    return token;
}

Token TokenizeStringLiteral(Lexer *lexer)
{   
    GetNextCharacter(lexer);

    unsigned int len = 0;
    unsigned int start = lexer->pos;

    // TODO: parse new_line character inside the string literal -> "\n" 
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
            PrintErrorLocationInSource(lexer->loadedFile, start - 1, lexer->line + 1, lexer->column + 1, "string literal missing terminating character '\"'");
            exit(1);
        }
    }

    GetNextCharacter(lexer);

    if(len == 0)
    {
        PrintErrorLocationInSource(lexer->loadedFile, start, lexer->line+1, lexer->column+1, "string literal cannot be empty");
        exit(1);
    }

    Token token = {0};
    token.type = TOKEN_STRING_CONSTANT;
    token.size = len + 2;
    token.column = lexer->column;
    token.line = lexer->line;
    token.pos = start - 1;
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
    token.pos = start;
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
    token->pos = start;

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

Token MakeSingleCharacterToken(Lexer *lexer, unsigned int tokenType)
{
    Token token = {0};
    token.type = tokenType;
    token.size = 1;
    token.line = lexer->line;
    token.column = lexer->column;
    token.pos = lexer->pos;
    return token;
}

TokenList TokenizeSource(LoadedFile loadedFile)
{
    TokenList tokenList = {0};
    
    Lexer lexer = {0};
    lexer.source = loadedFile.source.data;
    lexer.loadedFile = loadedFile;

    while(true)
    {
        char character = PeekNextCharacter(&lexer);

        if(IsNumeralCharacter(character)) // integer contants
        {
            Token token = TokenizeIntegerConstant(&lexer);
            lexer.column += token.size;
            PushToken(&tokenList, token);
        }
        else if(IsIdentifierCharacter(character)) // identifiers and keywords
        {
            Token token = {0};
            bool isKeyword = TryTokenizeKeyword(&lexer, &token);
            if(!isKeyword)
            {
                token = TokenizeIdentifier(&lexer);
            }
            lexer.column += token.size;
            PushToken(&tokenList, token);
        }
        else if(character == '\"') // string literal
        {
            Token token = TokenizeStringLiteral(&lexer);
            lexer.column += token.size + 2;
            PushToken(&tokenList, token);
        }
        else if(character == '\'') // character constant
        {
            Token token = TokenizeCharacterConstant(&lexer);
            lexer.column += token.size;
            PushToken(&tokenList, token);
        }
        else if(character == '+')
        {
            Token token = MakeSingleCharacterToken(&lexer, TOKEN_PLUS);
            lexer.column += 1;
            PushToken(&tokenList, token);
            GetNextCharacter(&lexer);
       }
        else if(character == '-')
        {
            Token token = MakeSingleCharacterToken(&lexer, TOKEN_MINUS);
            lexer.column += 1;
            PushToken(&tokenList, token);
            GetNextCharacter(&lexer);
        }
        else if(character == '*')
        {
            Token token = MakeSingleCharacterToken(&lexer, TOKEN_MULTIPLY);
            lexer.column += 1;
            PushToken(&tokenList, token);
            GetNextCharacter(&lexer);
        }
        else if(character == '%')
        {
            Token token = MakeSingleCharacterToken(&lexer, TOKEN_MODULUS);
            lexer.column += 1;
            PushToken(&tokenList, token);
            GetNextCharacter(&lexer);
        }
        else if(character == '/')
        {
            GetNextCharacter(&lexer);

            char next = PeekNextCharacter(&lexer);
            
            if(next == '/') // single line comment
            {
                GetNextCharacter(&lexer);

                while(true)
                {
                    next = PeekNextCharacter(&lexer);
                    if(next == '\n' || next == 0) break;
                    GetNextCharacter(&lexer);
                }
            } 
            else if(next == '#')  // multiline comment
            {
                GetNextCharacter(&lexer);
                lexer.column++;

                while(true)
                {
                    next = PeekNextCharacter(&lexer);

                    if(next == 0) break;
                    
                    if(next == '\n') 
                    {
                        lexer.line++;
                        lexer.column = 0;
                    }
                    else if(next == '#') 
                    {
                        GetNextCharacter(&lexer);
                        lexer.column++;
                        next = PeekNextCharacter(&lexer);
                        if(next == '/') 
                        {
                            GetNextCharacter(&lexer);
                            lexer.column++;
                            goto exit;
                        }
                        GetNextCharacter(&lexer);
                        lexer.column++;
                        continue;
                    }
                    
                    GetNextCharacter(&lexer);
                    lexer.column++;
                }
                exit: ;
            }
            else 
            {
                Token token = MakeSingleCharacterToken(&lexer, TOKEN_DIVIDE);
                lexer.column += 1;
                PushToken(&tokenList, token);
            }
        }
        else if(character == '=')
        {
            unsigned int startPos = lexer.pos;
            GetNextCharacter(&lexer);
            
            char next = PeekNextCharacter(&lexer);
            if(next == '=')
            {
                GetNextCharacter(&lexer);
                Token token = {0};
                token.type = TOKEN_EQ_EQ;
                token.size = 2;
                token.line = lexer.line;
                token.column = lexer.column;
                token.pos = startPos;                
                lexer.column += token.size;
                PushToken(&tokenList, token);
            }
            else
            {
                Token token = MakeSingleCharacterToken(&lexer, TOKEN_EQUAL);
                token.pos = startPos;
                lexer.column += 1;
                PushToken(&tokenList, token);            
            }
        }
        else if(character == '<')
        {
            unsigned int startPos = lexer.pos;

            GetNextCharacter(&lexer);

            char next = PeekNextCharacter(&lexer);
            if(next == '=')
            {
                GetNextCharacter(&lexer);
                Token token = {0};
                token.type = TOKEN_LT_EQ;
                token.line = lexer.line;
                token.column = lexer.column;
                token.size = 2;
                token.pos = lexer.pos;
                lexer.column += token.size;
                PushToken(&tokenList, token);
            }
            else
            {
                Token token = MakeSingleCharacterToken(&lexer, TOKEN_LT);
                token.pos = startPos;
                lexer.column += 1;
                PushToken(&tokenList, token);
            }
        }
        else if(character == '>')
        {
            unsigned int startPos = lexer.pos;

            GetNextCharacter(&lexer);
            
            char next = PeekNextCharacter(&lexer);
            if(next == '=')
            {
                GetNextCharacter(&lexer);
                Token token = {0};
                token.type = TOKEN_GT_EQ;
                token.line = lexer.line;
                token.column = lexer.column;
                token.size = 2;
                token.pos = startPos;        
                lexer.column += token.size;
                PushToken(&tokenList, token);
            }
            else
            {
                Token token = MakeSingleCharacterToken(&lexer, TOKEN_GT);
                token.pos = startPos;
                lexer.column += 1;
                PushToken(&tokenList, token);
            }
        }
        else if(character == '!')
        {
            unsigned int startPos = lexer.pos;

            GetNextCharacter(&lexer);
            
            char next = PeekNextCharacter(&lexer);

            if(next == '=')
            {
                GetNextCharacter(&lexer);
                Token token = {0};
                token.type = TOKEN_GT_EQ;
                token.line = lexer.line;
                token.column = lexer.column;
                token.size = 2;                    
                token.pos = startPos;            
                lexer.column += token.size;
                PushToken(&tokenList, token);
            }
            else 
            {
                Token token = MakeSingleCharacterToken(&lexer, TOKEN_NOT);
                token.pos = startPos;
                lexer.column += 1;
                PushToken(&tokenList, token);
            }
        }
        else if(character == '&')
        {
            unsigned int startPos = lexer.pos;

            GetNextCharacter(&lexer);
            char next = PeekNextCharacter(&lexer);
            
            if(next != '&')
            {
                PrintErrorLocationInSource(lexer.loadedFile, lexer.pos - 1, lexer.line + 1, lexer.column + 1, "found '&' expected '&&'");
                exit(1);
            }

            GetNextCharacter(&lexer);
            
            Token token = {0};
            token.type = TOKEN_AND;
            token.line = lexer.line;
            token.column = lexer.column;
            token.pos = startPos;
            token.size = 2;

            lexer.column += token.size;
            
            PushToken(&tokenList, token);
        }
        else if(character == '|')
        {
            unsigned int startPos = lexer.pos;

            GetNextCharacter(&lexer);
            char next = PeekNextCharacter(&lexer);

            if(next != '|')
            {
                PrintErrorLocationInSource(lexer.loadedFile, lexer.pos - 1, lexer.line + 1, lexer.column + 1, "found '|' expected '||'");
                exit(1);
            }

            GetNextCharacter(&lexer);
            
            Token token = {0};
            token.type = TOKEN_OR;
            token.line = lexer.line;
            token.column = lexer.column;
            token.size = 2;
            token.pos = startPos;

            lexer.column += token.size;
            
            PushToken(&tokenList, token);
        }
        else if(character == '(')
        {
            Token token = MakeSingleCharacterToken(&lexer, TOKEN_LEFT_PAREN);
            lexer.column += 1;
            PushToken(&tokenList, token);
            GetNextCharacter(&lexer);
        }
        else if(character == ')')
        {
            Token token = MakeSingleCharacterToken(&lexer, TOKEN_RIGHT_PAREN);
            lexer.column += 1;
            PushToken(&tokenList, token);
            GetNextCharacter(&lexer);
        }
        else if(character == '{')
        {
            Token token = MakeSingleCharacterToken(&lexer, TOKEN_LEFT_BRACE);
            lexer.column += 1;
            PushToken(&tokenList, token);
            GetNextCharacter(&lexer);
        }
        else if(character == '}')
        {
            Token token = MakeSingleCharacterToken(&lexer, TOKEN_RIGHT_BRACE);
            lexer.column += 1;
            PushToken(&tokenList, token);
            GetNextCharacter(&lexer);
        }
        else if(character == '[')
        {
            Token token = MakeSingleCharacterToken(&lexer, TOKEN_LEFT_BRACKET);
            lexer.column += 1;
            PushToken(&tokenList, token);
            GetNextCharacter(&lexer);
        }
        else if(character == ']')
        {
            Token token = MakeSingleCharacterToken(&lexer, TOKEN_RIGHT_BRACKET);
            lexer.column += 1;
            PushToken(&tokenList, token);
            GetNextCharacter(&lexer);
        }
        else if(character == ';')
        {
            Token token = MakeSingleCharacterToken(&lexer, TOKEN_SEMICOLON);
            lexer.column += 1;
            PushToken(&tokenList, token);
            GetNextCharacter(&lexer);
        }
        else if(character == ',')
        {
            Token token = MakeSingleCharacterToken(&lexer, TOKEN_COMMA);
            lexer.column += 1;
            PushToken(&tokenList, token);
            GetNextCharacter(&lexer);
        }
        else if(character == ':')
        {            
            Token token = MakeSingleCharacterToken(&lexer, TOKEN_COLON);
            lexer.column += 1;
            PushToken(&tokenList, token);
            GetNextCharacter(&lexer);
        }
        else if(character == '.')
        {            
            Token token = MakeSingleCharacterToken(&lexer, TOKEN_DOT);
            lexer.column += 1;
            PushToken(&tokenList, token);
            GetNextCharacter(&lexer);
        }
        else if(character == 0)
        {
            Token token = MakeSingleCharacterToken(&lexer, TOKEN_PROGRAM_END);
            lexer.column += 1;
            PushToken(&tokenList, token);
            GetNextCharacter(&lexer);
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
            char errorMsg[500] = {0};
            sprintf(errorMsg, "unsupported character ascii code '%d'", character);
            PrintErrorLocationInSource(lexer.loadedFile, lexer.pos, lexer.line + 1, lexer.column + 1, errorMsg);
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
        case TOKEN_CHAR_CONSTANT:           return "token_character_constant"; break;
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
        default:                            return "unknown_token_type";
    }
}

void PrintTokenInfo(Token token, char *source)
{
    printf("line %u, column: %u, position: %u, type: '%s', size: %u, lexeme: '", token.line+1, token.column+1, token.pos, TokenTypeToString(token.type), token.size);
    for(int n = token.pos; n < token.pos + token.size; n++) 
    {
        printf("%c", source[n]);
    }
    printf("'\n"); 
}
