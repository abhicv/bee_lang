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

int main(int argc, char **argv) 
{    
    Test_TokenizeIntegerConstant();
    Test_TryTokenizeKeyword();
    Test_TokenizeIdentifier();
    return 0;
}
