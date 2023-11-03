#include "lexer.c"
#include "parser.c"
#include "ast.c"
#include "symbol.c"

int main(int argc, char *argv[])
{
    TypeTable globalTypeTable = {0};
    SymbolTable globalSymbolTable = {0};

    // push primitve type to global type table
    Type integerType = {.id = "int", .size = 1};
    Type voidType = {.id = "void", .size = 1};
    Type boolType = {.id = "bool", .size = 1};

    PushType(&globalTypeTable, integerType);
    PushType(&globalTypeTable, voidType);
    PushType(&globalTypeTable, boolType);

    AST ast = {0};
    InitAST(&ast);

    // printf("size of ast node: %zd bytes\n", sizeof(Node));

    if(argc > 1)
    {
        LoadedFile loadedFile = LoadFileNullTerminated(argv[1]);
        
        if(loadedFile.isLoaded)
        {
            Parser parser = {0};
            parser.fileName = argv[1];
            parser.loadedFile = loadedFile;            
            parser.tokenList = TokenizeSource(loadedFile.data);
            
            printf("token count: %u\n", parser.tokenList.count);

            // for(int n = 0; n < parser.tokenList.count; n++)
            // {
                // PrintTokenInfo(parser.tokenList.tokens[n]);
                // if(parser.tokenList.tokens[n].type == TOKEN_STRING_CONSTANT)
                // {
                //     printf("string value: %s\n", parser.tokenList.tokens[n].stringValue);
                // }
            // }
            
            Index rootIndex = ParseProgram(&ast, &parser);
            // printf("parsing completed, AST build complete\n");
            // printf("AST memory usage: %zd bytes\n", ast.nodeCount * sizeof(Node));

            // PrintNode(ast, rootIndex, 0);

            // printf("int type index: %d\n", GetTypeTableIndexForId(globalTypeTable, "int"));
            // printf("str type index: %d\n", GetTypeTableIndexForId(globalTypeTable, "str"));
            // printf("void type index: %d\n", GetTypeTableIndexForId(globalTypeTable, "void"));

            // BuildTypeTable(&ast, rootIndex, &globalTypeTable);
            // PrintTypeTable(globalTypeTable);

            // TypeCheckAST(&ast, rootIndex, &globalTypeTable);

            free(loadedFile.data);
        }
    }
    else
    {        
        char *source = "b * 10 > c + d *  e";
        // char *source = "if(a) { let a = 10; }";
        // char *source = "if(a) { let a = 10; } else { let b = 10; }";
        // char *source = "if(a) { let a = 10; } else if(b) { let b = 10; }";
        // char *source = "if(a) { let a = 10; } else if(b) { let b = 10; } else { let c = 10; }";
        // char *source = "if(a) { let a = 10; } else if(b) { let b = 10; } else if(c) { let c = 10; }";
        
        LoadedFile loadedFile = {0};
        loadedFile.data = "c < b != 10 * 100 + 10";
        loadedFile.size = strlen(loadedFile.data);
        loadedFile.isLoaded = true;
        
        Parser parser = {0};        
        parser.fileName = "source";
        parser.loadedFile = loadedFile;
        parser.tokenList = TokenizeSource(source);

        Index index = ParseExpression(&ast, &parser, 1);
        // Index index = ParseIfStatement(&ast, &parser);
    
        PrintNode(ast, index, 0);
    }
    
    return 0;
}