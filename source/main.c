#include "lexer.c"
#include "parser.c"
#include "ast.c"
#include "symbol.c"

int main(int argc, char *argv[])
{
    TypeTable globalTypeTable = {0};
    SymbolTable globalSymbolTable = {0};

    Type integerType = {.id = "int", .size = 1};
    Type voidType = {.id = "void", .size = 1};
    Type boolType = {.id = "bool", .size = 1};

    // push primitve type to global type table
    PushType(&globalTypeTable, integerType);
    PushType(&globalTypeTable, voidType);
    PushType(&globalTypeTable, boolType);

    AST ast = {0};
    InitAST(&ast);

    // printf("size of ast node: %zd bytes\n", sizeof(Node));
    // printf("size of a token: %zd bytes\n", sizeof(Token));

    if(argc > 1)
    {
        LoadedFile loadedFile = LoadFileNullTerminated(argv[1]);
        
        if(loadedFile.isLoaded)
        {
            Parser parser = {0};
            parser.loadedFile = loadedFile;            
            parser.tokenList = TokenizeSource(loadedFile);
        
            // for(int n = 0; n < parser.tokenList.count; n++)
            // {
            //     PrintTokenInfo(parser.tokenList.tokens[n], loadedFile.source.data);
            // }
            
            Index rootIndex = ParseProgram(&ast, &parser);
            PrintNode(ast, rootIndex, 0);

            // BuildTypeTable(&ast, rootIndex, &globalTypeTable);
            // PrintTypeTable(globalTypeTable);

            // TypeCheckAST(&ast, rootIndex, &globalTypeTable);

            free(loadedFile.source.data);
            free(loadedFile.path.data);
        }
    }
    else
    {        
        printf("no input source file provided\n");
    }
    
    return 0;
}