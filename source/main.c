#include "file.c"
#include "error.c"
#include "lexer.c"
#include "ast.c"
#include "parser.c"
#include "symbol.c"
#include "interpreter.c"
#include "code_gen.c"

int main(int argc, char *argv[])
{
    TypeTable globalTypeTable = {0};

    // push primitve type to global type table
    PushType(&globalTypeTable, (Type){.id = "void", .size = 1});
    PushType(&globalTypeTable, (Type){.id = "char", .size = 1});
    PushType(&globalTypeTable, (Type){.id = "bool", .size = 1});
    PushType(&globalTypeTable, (Type){.id = "int", .size = 1});
    PushType(&globalTypeTable, (Type){.id = "string", .size = 1});
    
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
            ast.parser = parser;

            // PrintNode(ast, rootIndex, 0);
            WriteAsDotFile(ast, rootIndex, "ast.dot");

            bool isSuccess = BuildTypeTable(&ast, rootIndex, &globalTypeTable);

            if(isSuccess) {
                TypeCheckAST(&ast, rootIndex, &globalTypeTable);
                // PrintTypeTable(globalTypeTable);
            }

            // InterpretAST(ast, globalTypeTable);

            GenerateCode(ast, rootIndex, globalTypeTable, -1, false);
            PrintInstruction(stdout, instructions, instrCount, true);

            // StackVM vm = InitVM();

            free(loadedFile.source.data);
            free(loadedFile.path.data);
        }
    }
    else
    {        
        printf("error: no input source file provided\n");
    }
    
    return 0;
}