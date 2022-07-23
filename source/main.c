#include "lexer.c"
#include "parser.c"
#include "ast.c"

int main(int argc, char *argv[argc])
{
    AST ast = {0};
    InitAST(&ast);

    printf("size of ast node: %ld bytes\n", sizeof(Node));

    if(argc > 1)
    {
        char *source = LoadFileNullTerminated(argv[1]);
        
        if(source)
        {
            Parser parser = {0};
            parser.fileName = argv[1];
            parser.source = source;            
            parser.tokenList = TokenizeSource(source);
            
            printf("token count: %u\n", parser.tokenList.count);

            // for(int n = 0; n < parser.tokenList.count; n++)
            // {
            //     PrintTokenInfo(parser.tokenList.tokens[n]);
            //     if(parser.tokenList.tokens[n].type == TOKEN_STRING_CONSTANT)
            //     {
            //         printf("string value: %s\n", parser.tokenList.tokens[n].stringValue);
            //     }
            // }
            
            Index index = ParseProgram(&ast, &parser);
            PrintNode(ast, index, 0);

            printf("ast list memory usage: %ld bytes\n", ast.nodeCount * sizeof(Node));
            
            free(source);
        }
    }
    else
    {        
        // char *source = "b * 10 > c + d *  e";
        char *source = "c < b != 10 * 100 + 10";
 
        // char *source = "if(a) { let a = 10;}";
        // char *source = "if(a) { let a = 10; } else { let b = 10; }";
        // char *source = "if(a) { let a = 10; } else if(b) { let b = 10; }";
        // char *source = "if(a) { let a = 10; } else if(b) { let b = 10; } else { let c = 10; }";
        // char *source = "if(a) { let a = 10; } else if(b) { let b = 10; } else if(c) { let c = 10; }";
        Parser parser = {0};

        parser.fileName = "source";
        parser.source = source;
        parser.tokenList = TokenizeSource(source);

        Index index = ParseExpression(&ast, &parser, 1);
        // Index index = ParseIfStatement(&ast, &parser);
    
        PrintNode(ast, index, 0);
    }
    
    return 0;
}