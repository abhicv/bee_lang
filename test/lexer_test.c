#include "..\source\error.c"
#include "..\source\lexer.c"

#include <assert.h>

void Test_TokenizeIntegerConstant() 
{
    printf("\n======== %s =======\n", __func__);

    char *source = "123456789";
    Lexer lexer = {0};
    lexer.source = source;

    Token token = TokenizeIntegerConstant(&lexer);

    assert(token.type == TOKEN_INTEGER_CONSTANT);
    assert(token.integerValue == 123456789);
    assert(token.size == 9);

    printf("%s: OK\n", __func__);
}

void Test_TryTokenizeKeyword() 
{
    printf("\n======== %s =======\n", __func__);

    char *testkeywordList[] = {"fn", "struct", "if", "else", "while", "return", "let", "true", "false"};
    enum TokenType keywordTokenTypes[] = {TOKEN_KEYWORD_FN, TOKEN_KEYWORD_STRUCT, TOKEN_KEYWORD_IF, TOKEN_KEYWORD_ELSE, TOKEN_KEYWORD_WHILE, TOKEN_KEYWORD_RETURN, TOKEN_KEYWORD_LET, TOKEN_KEYWORD_TRUE, TOKEN_KEYWORD_FALSE};

    int size = sizeof(testkeywordList) / sizeof(testkeywordList[0]);

    Lexer lexer = {0};
    for(int n = 0; n < size; n++) 
    {
        lexer.source = testkeywordList[n];
        printf("keyword: '%s'\n", lexer.source);

        Token token = {0};
        bool isKeyword = TryTokenizeKeyword(&lexer, &token);
        assert(isKeyword == true);
        assert(token.type == keywordTokenTypes[n]);
        assert(token.size == strlen(lexer.source));
        
        lexer.pos = 0;
    }

    printf("%s: OK\n", __func__);
}

void Test_TokenizeIdentifier()
{
    printf("\n======== %s =======\n", __func__);

    char *source = "_count123__";
    Lexer lexer = {0};
    lexer.source = source;

    Token token = TokenizeIdentifier(&lexer);

    assert(token.type == TOKEN_IDENTIFIER);
    assert(strcmp(token.identifier, source) == 0);
    assert(token.size == strlen(source));
    
    printf("%s: OK\n", __func__);
}

void Test_TokenizeCharacterConstant() 
{
    printf("\n======== %s =======\n", __func__);

    char *source = "'a'";
    Lexer lexer = {0};
    lexer.source = source;
    // Mock LoadedFile for error reporting
    lexer.loadedFile.source.data = source;
    lexer.loadedFile.source.length = strlen(source);

    Token token = TokenizeCharacterConstant(&lexer);

    assert(token.type == TOKEN_CHAR_CONSTANT);
    assert(token.characterValue == 'a');
    assert(token.size == 3);

    printf("%s: OK\n", __func__);
}

void Test_TokenizeStringLiteral()
{
    printf("\n======== %s =======\n", __func__);

    char *source = "\"hello world\"";
    Lexer lexer = {0};
    lexer.source = source;
    // Mock LoadedFile for error reporting
    lexer.loadedFile.source.data = source;
    lexer.loadedFile.source.length = strlen(source);

    Token token = TokenizeStringLiteral(&lexer);

    assert(token.type == TOKEN_STRING_CONSTANT);
    assert(strcmp(token.stringValue, "hello world") == 0);
    assert(token.size == 13);
    free(token.stringValue);

    printf("%s: OK\n", __func__);
}

void Test_TokenizeOperators()
{
    printf("\n======== %s =======\n", __func__);

    char *source = "+ - * / % < > <= >= == != && || ! = ( ) { } [ ] : ; , .";
    LoadedFile file = {.source = {.data = source, .length = strlen(source)}, .isLoaded = true};

    TokenList tokenList = TokenizeSource(file);

    enum TokenType expectedTokens[] = {
        TOKEN_PLUS, TOKEN_MINUS, TOKEN_MULTIPLY, TOKEN_DIVIDE, TOKEN_MODULUS,
        TOKEN_LT, TOKEN_GT, TOKEN_LT_EQ, TOKEN_GT_EQ, TOKEN_EQ_EQ, TOKEN_NOT_EQ,
        TOKEN_AND, TOKEN_OR, TOKEN_NOT, TOKEN_EQUAL,
        TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
        TOKEN_LEFT_BRACKET, TOKEN_RIGHT_BRACKET,
        TOKEN_COLON, TOKEN_SEMICOLON, TOKEN_COMMA, TOKEN_DOT,
        TOKEN_PROGRAM_END
    };

    assert(tokenList.count == sizeof(expectedTokens) / sizeof(expectedTokens[0]));

    for(unsigned int i = 0; i < tokenList.count; i++) {
        assert(tokenList.tokens[i].type == expectedTokens[i]);
    }

    free(tokenList.tokens);
    printf("%s: OK\n", __func__);
}

void Test_TokenizeSource_WithComments()
{
    printf("\n======== %s =======\n", __func__);

    char *source = "// this is a single line comment\nlet x = 10; /# this is a multi-line comment #/ let y = 20;";
    LoadedFile file = {.source = {.data = source, .length = strlen(source)}, .isLoaded = true};

    TokenList tokenList = TokenizeSource(file);

    enum TokenType expectedTokens[] = {
        TOKEN_KEYWORD_LET, TOKEN_IDENTIFIER, TOKEN_EQUAL, TOKEN_INTEGER_CONSTANT, TOKEN_SEMICOLON,
        TOKEN_KEYWORD_LET, TOKEN_IDENTIFIER, TOKEN_EQUAL, TOKEN_INTEGER_CONSTANT, TOKEN_SEMICOLON,
        TOKEN_PROGRAM_END
    };

    assert(tokenList.count == sizeof(expectedTokens) / sizeof(expectedTokens[0]));

    for(unsigned int i = 0; i < tokenList.count; i++) {
        assert(tokenList.tokens[i].type == expectedTokens[i]);
    }

    // Free allocated memory for identifiers
    free(tokenList.tokens[1].identifier);
    free(tokenList.tokens[6].identifier);
    free(tokenList.tokens);
    printf("%s: OK\n", __func__);
}

int main(int argc, char **argv) 
{    
    Test_TokenizeIntegerConstant();
    Test_TryTokenizeKeyword();
    Test_TokenizeIdentifier();
    Test_TokenizeCharacterConstant();
    Test_TokenizeStringLiteral();
    Test_TokenizeOperators();
    Test_TokenizeSource_WithComments();
    return 0;
}
