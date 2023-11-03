#include "..\source\lexer.c"

void Test_TokenizeIntegerConstant() 
{
    char *source = "10";
    Lexer lexer = {0};
    lexer.source = source;

    Token token = TokenizeIntegerConstant(&lexer);

    if(token.type == TOKEN_INTEGER_CONSTANT && token.integerValue == 10 && token.size == 2)
    {
        printf("Interger tokenize test: OK\n");
    } 
    else 
    {
        printf("Interger tokenize test: FAILED\n");
    }
}

int main(int argc, char **argv) 
{    
    Test_TokenizeIntegerConstant();

    return 0;
}
