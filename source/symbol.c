#include "symbol.h"
#include "stdlib.h"
#include "ast.h"

void PushType(TypeTable *table, Type type) 
{
    if(table->count == 0) 
    {
        table->count++;
        table->types = (Type*)malloc(sizeof(Type));
    }
    else 
    {
        table->count++;
        table->types = (Type*)realloc(table->types, sizeof(Type) * table->count);
    }

    table->types[table->count - 1] = type;
}

void PushSymbol(SymbolTable *table, Symbol symbol) 
{
    if(table->count == 0) 
    {
        table->count++;
        table->symbols = (Symbol*)malloc(sizeof(Type));
    }
    else 
    {
        table->count++;
        table->symbols = (Symbol*)realloc(table->symbols, sizeof(Type) * table->count);
    }

    table->symbols[table->count - 1] = symbol;
}

int GetTypeTableIndexForId(TypeTable *typeTable, const char *id) 
{
    for(int n = 0; n < typeTable->count; n++) 
    {
        if(strcmp(typeTable->types[n].id, id) == 0) 
        {
            return n;
        }        
    }

    return -1;
}

int GetSymbolTableIndexForId(SymbolTable *symbolTable, const char *symbolName) 
{
    for(int n = 0; n < symbolTable->count; n++)
    {
        Symbol symbol = symbolTable->symbols[n]; 
        if(strcmp(symbol.name, symbolName) == 0) 
        {
            return n;
        }      
    }

    return -1;
}

void BuildTypeTable(AST *ast, Index rootIndex, TypeTable *globalTypeTable)
{
    if(ast == NULL) return;
    if(globalTypeTable == NULL) return;
    if(rootIndex >= ast->nodeCount) return;

    Node node = ast->nodeList[rootIndex];

    if(node.type == NODE_PROGRAM) 
    {
        // insert structs
        for(int n = 0; n < node.program.defCount; n++) 
        {
            Index index = node.program.definitions[n];
            Node defNode = ast->nodeList[index];

            if(defNode.type == NODE_STRUCT_DEFINITION) 
            {
                // check if type with same name already exist
                if(GetTypeTableIndexForId(globalTypeTable, defNode.structDef.name) != -1)
                {
                    printf("[ERROR] redefinition of struct: '%s'\n", defNode.structDef.name);
                    return;
                }

                Type type = {0};
                type.id = defNode.structDef.name;
                type.isStruct = true;

                int totalSize = 0;

                for(int i = 0; i < defNode.structDef.fieldCount; i++)
                {
                    Node fieldNode = ast->nodeList[defNode.structDef.fields[i]];
                    Node idNode = ast->nodeList[fieldNode.field.id];
                    Node typeNode = ast->nodeList[fieldNode.field.type];

                    const char* typeId = typeNode.typeAnnotation.id;

                    int typeTableIndex = -1;

                    // if field type name is same as the current struct name(recursive field)
                    if(!strcmp(typeId, type.id)) {
                        typeTableIndex = globalTypeTable->count;
                        totalSize += 1;
                    } else {
                        typeTableIndex = GetTypeTableIndexForId(globalTypeTable, typeId);

                        if(typeTableIndex == -1) 
                        {
                            printf("[ERROR] undefined type: '%s' for field: '%s' in struct: '%s'\n", typeId, idNode.identifier.value, type.id);
                            return;
                        }

                        totalSize += globalTypeTable->types[typeTableIndex].size;
                    }

                    StructField field = {0};
                    field.name = idNode.identifier.value;
                    field.typeTableIndex = typeTableIndex;

                    if(typeNode.typeAnnotation.isArrayType) {
                        field.isArray = true;
                        field.arraySize = typeNode.typeAnnotation.arrayDim;
                    }

                    PushSymbol(&(type.fieldList), field);
                }

                type.size = totalSize;
                PushType(globalTypeTable, type);
            }
        }

        // insert function into type table
        for(int n = 0; n < node.program.defCount; n++) 
        {
            Index index = node.program.definitions[n];
            Node defNode = ast->nodeList[index];
            
            if(defNode.type == NODE_FUNCTION_DEFINITION)
            {
                const char* functionName = defNode.functionDef.name;

                // check if this function name is already used or not
                if(GetTypeTableIndexForId(globalTypeTable, functionName) != -1) 
                {
                    printf("[ERROR] type or function with name '%s' already defined\n", functionName);
                    return;
                }

                int returnTypeIndex = -1;

                if(defNode.functionDef.isReturnTypeDeclared) 
                {
                    Node typeNode = ast->nodeList[defNode.functionDef.returnType];
                    returnTypeIndex = GetTypeTableIndexForId(globalTypeTable, typeNode.typeAnnotation.id);
                } 
                else 
                {
                    returnTypeIndex = GetTypeTableIndexForId(globalTypeTable, "void");
                }

                if(returnTypeIndex == -1) 
                {
                    printf("[ERROR] undefined return type declared for function: '%s'\n", functionName);
                    return;
                }

                Type type = {0};
                type.id = functionName;
                type.isFunction = true;
                type.returnTypeIndex = returnTypeIndex;
                type.size = 0;

                for(int i = 0; i < defNode.functionDef.parameterCount; i++)
                {
                    Node paramNode = ast->nodeList[defNode.functionDef.parameters[i]];
                    Node idNode = ast->nodeList[paramNode.param.id];
                    Node typeNode = ast->nodeList[paramNode.param.type];

                    // check uniqueness of parameter name
                    for(int k = 0; k < type.paramList.count; k++) 
                    {
                        if(!strcmp(type.paramList.symbols[k].name, idNode.identifier.value)) 
                        {
                            printf("[ERROR] repeated declaration of argument '%s' in function '%s'\n", idNode.identifier.value, functionName);
                            return;
                        }
                    }

                    int typeIndex = GetTypeTableIndexForId(globalTypeTable, typeNode.typeAnnotation.id); 
                    if(typeIndex == -1) 
                    {
                        printf("[ERROR] undefined type '%s' for argument '%s' in function '%s'\n", typeNode.typeAnnotation.id, idNode.identifier.value, functionName);
                        return;
                    }

                    Parameter param = {0};
                    param.name = idNode.identifier.value;
                    param.typeTableIndex = typeIndex;

                    if(typeNode.typeAnnotation.isArrayType) 
                    {
                        param.isArray = true;
                        param.arraySize = typeNode.typeAnnotation.arrayDim;
                    }

                    PushSymbol(&(type.paramList), param);
                }

                PushType(globalTypeTable, type);
            }
        }
    }
}

int TypeCheckNode(AST *ast, Index nodeIndex, TypeTable *typeTable, int currentTypeIndex)
{
    if(ast == NULL) return -1;
    if(nodeIndex >= ast->nodeCount) return -1;
    if(typeTable == NULL) return -1;

    Node node = ast->nodeList[nodeIndex];

    switch (node.type)
    {
    case NODE_STATEMENT_LIST: 
    {
        for(int n = 0; n < node.statementList.statementCount; n++) 
        {
            TypeCheckNode(ast, node.statementList.statements[n], typeTable, currentTypeIndex);
        }
    }
    break;

    case NODE_ASSIGN_STATEMENT:
    {
        int leftTypeIndex = TypeCheckNode(ast, node.assignStmt.lValue, typeTable, currentTypeIndex);
        int rightTypeIndex = TypeCheckNode(ast, node.assignStmt.expression, typeTable, currentTypeIndex);
        
        if(leftTypeIndex != rightTypeIndex) 
        {
            printf("[TYPE ERROR]\n");
        }
    }
    break;

    case NODE_L_VALUE:
    {
        int prevTypeIndex = -1;
        for(int n = 0; n < node.lValue.simpleLValueCount; n++) 
        {
            if(n == 0) {
                int typeIndex = TypeCheckNode(ast, node.lValue.simpleLValues[n], typeTable, currentTypeIndex);
                prevTypeIndex = typeIndex;
            } else {
                int typeIndex = TypeCheckNode(ast, node.lValue.simpleLValues[n], typeTable, prevTypeIndex);
                prevTypeIndex = typeIndex;
            }
        }

        return prevTypeIndex;
    }
    break;

    case NODE_IDENTIFIER:
    {
        printf("type checking identifier: %s\n", node.identifier.value);
        int symbolTableIndex = GetSymbolTableIndexForId(&typeTable->types[currentTypeIndex].paramList, node.identifier.value);

        if(symbolTableIndex == -1)
        {
            printf("[ERROR] undefined variable '%s'\n", node.identifier.value);
            return -1;
        }

        Symbol symbol = typeTable->types[currentTypeIndex].paramList.symbols[symbolTableIndex];

        if(symbol.isArray) 
        {
            printf("[ERROR] variable '%s' is of array type need to be indexed\n", node.identifier.value);
            return -1;
        }

        return symbol.typeTableIndex;
    }
    break;

    case NODE_ARRAY_ACCESS:
    {
        Node idNode = ast->nodeList[node.arrayAccess.id];

        int symbolTableIndex = GetSymbolTableIndexForId(&typeTable->types[currentTypeIndex].paramList, idNode.identifier.value);

        if(symbolTableIndex == -1)
        {
            printf("[ERROR] undefined variable '%s'", node.identifier.value);
            return -1;
        }

        Symbol symbol = typeTable->types[currentTypeIndex].paramList.symbols[symbolTableIndex];

        if(!symbol.isArray)
        {
            printf("[ERROR] variable '%s' cannot be indexed as an array\n", idNode.identifier.value);
            return -1;
        }

        int arrayIndexExprType = TypeCheckNode(ast, node.arrayAccess.expr, typeTable, currentTypeIndex);

        int intergerTypeIndex = GetTypeTableIndexForId(typeTable, "int");

        if(arrayIndexExprType != intergerTypeIndex)
        {
            printf("[ERROR] array index should be of type 'int'!\n", idNode.identifier.value);
            return -1;
        }

        return symbol.typeTableIndex;
    }
    break;

    case NODE_INTEGER_CONSTANT:
    {
        return GetTypeTableIndexForId(typeTable, "int");
    }
    break;

    case NODE_BOOLEAN_CONSTANT:
    {
        return GetTypeTableIndexForId(typeTable, "bool");
    }
    break;

    default:
        break;
    }

    return -1;
}

void TypeCheckAST(AST *ast, Index rootIndex, TypeTable *typeTable)
{
    if(ast == NULL) return;
    if(rootIndex >= ast->nodeCount) return;

    Node rootNode = ast->nodeList[rootIndex];

    if(rootNode.type == NODE_PROGRAM) 
    {
        for(int n = 0; n < rootNode.program.defCount; n++)
        {
            Node defNode = ast->nodeList[rootNode.program.definitions[n]];

            if(defNode.type == NODE_FUNCTION_DEFINITION)
            {
                printf("type checking function: %s\n", defNode.functionDef.name);
                int typeIndex = GetTypeTableIndexForId(typeTable, defNode.functionDef.name);
                TypeCheckNode(ast, defNode.functionDef.body, typeTable, typeIndex);
            }
        }
    }
}

void PrintType(Type type)
{
    printf("name: '%s', size: %u, is_struct: %d, is_function: %d", type.id, type.size, type.isStruct, type.isFunction);
    printf("\n");
}

void PrintTypeTable(TypeTable typeTable)
{
    printf("\nGlobal Type Table\n");

    for(int n = 0; n < typeTable.count; n++) 
    {
        printf("[%d] ", n);
        PrintType(typeTable.types[n]);
    }

    printf("\n");
}
