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

        Type *types = (Type*)realloc(table->types, sizeof(Type) * table->count);
        if(types == NULL)
        {
            printf("[ERROR] memory allocation for types array failed");
            return;
        }
        table->types = types;
    }

    table->types[table->count - 1] = type;
}

void PushSymbol(SymbolTable *table, Symbol symbol) 
{
    if(table->count == 0) 
    {
        table->count++;
        table->symbols = (Symbol*)malloc(sizeof(Symbol));
    }
    else 
    {
        table->count++;
        table->symbols = (Symbol*)realloc(table->symbols, sizeof(Symbol) * table->count);
    }

    table->symbols[table->count - 1] = symbol;
}

void PushField(SymbolTable *table, StructField field) 
{
    PushSymbol(table, field);
}

void PushParam(SymbolTable *table, Parameter param) 
{
    PushSymbol(table, param);
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

int GetTypeTableIndexForFunctionId(TypeTable *typeTable, const char *id) 
{
    for(int n = 0; n < typeTable->count; n++) 
    {
        if(strcmp(typeTable->types[n].id, id) == 0 && typeTable->types[n].isFunction) 
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

bool BuildTypeTable(AST *ast, Index rootIndex, TypeTable *typeTable)
{
    if(ast == NULL) return false;
    if(typeTable == NULL) return false;
    if(rootIndex >= ast->nodeCount) return false;

    Node node = ast->nodeList[rootIndex];
    if(node.type == NODE_PROGRAM) 
    {
        for(int n = 0; n < node.program.defCount; n++) 
        {
            Index index = node.program.definitions[n];
            Node defNode = ast->nodeList[index];

            // insert structs
            if(defNode.type == NODE_STRUCT_DEFINITION) 
            {
                // check if type with same name already exist
                if(GetTypeTableIndexForId(typeTable, defNode.structDef.name) != -1)
                {
                    printf("[ERROR] redefinition of struct: '%s'\n", defNode.structDef.name);
                    return false;
                }

                Type type = {0};
                type.id = defNode.structDef.name;
                type.isStruct = true;
                type.astIndex = index;

                PushType(typeTable, type);
            }
        }

        // insert struct fields
        for(int n = 0; n < typeTable->count; n++)
        {
            Type *type = &typeTable->types[n];

            if(type->isStruct == false) continue;

            Index astIndex = type->astIndex;
            Node defNode = ast->nodeList[astIndex];

            int totalSize = 0;

            for(int i = 0; i < defNode.structDef.fieldCount; i++)
            {
                Node fieldNode = ast->nodeList[defNode.structDef.fields[i]];
                Node idNode = ast->nodeList[fieldNode.field.id];
                Node typeNode = ast->nodeList[fieldNode.field.type];

                const char* typeId = typeNode.typeAnnotation.id;

                int typeTableIndex = -1;

                // if field type name is same as the current struct name (recursive field)
                if(!strcmp(typeId, type->id)) 
                {
                    typeTableIndex = n;
                    totalSize += 1;
                } 
                else 
                {
                    typeTableIndex = GetTypeTableIndexForId(typeTable, typeId);

                    if(typeTableIndex == -1) 
                    {
                        printf("[ERROR] undefined type: '%s' for field: '%s' in struct: '%s'\n", typeId, idNode.identifier.value, type->id);
                        return false;
                    }

                    totalSize += typeTable->types[typeTableIndex].size;
                }

                StructField field = {0};
                field.name = idNode.identifier.value;
                field.typeTableIndex = typeTableIndex;

                if(typeNode.typeAnnotation.isArrayType) 
                {
                    field.isArray = true;
                    field.arraySize = typeNode.typeAnnotation.arrayDim;
                }

                PushField(&type->fieldList, field);
            }

            type->size = totalSize;
        }

        // insert function
        for(int n = 0; n < node.program.defCount; n++)
        {
            Index index = node.program.definitions[n];
            Node defNode = ast->nodeList[index];
            
            if(defNode.type == NODE_FUNCTION_DEFINITION)
            {
                const char* functionName = defNode.functionDef.name;

                // check if function with same name is already defined
                int typeIndex = GetTypeTableIndexForFunctionId(typeTable, functionName); 

                if(typeIndex != -1)
                {
                    printf("[ERROR] function with name '%s' already defined\n", functionName);
                    return false;
                }

                int returnTypeIndex = -1;

                if(defNode.functionDef.isReturnTypeDeclared) 
                {
                    Node typeNode = ast->nodeList[defNode.functionDef.returnType];
                    returnTypeIndex = GetTypeTableIndexForId(typeTable, typeNode.typeAnnotation.id);
                } 
                else 
                {
                    returnTypeIndex = GetTypeTableIndexForId(typeTable, "void");
                }

                if(returnTypeIndex == -1) 
                {
                    printf("[ERROR] undefined return type declared for function: '%s'\n", functionName);
                    return false;
                }

                Type type = {0};
                type.id = functionName;
                type.isFunction = true;
                type.returnTypeIndex = returnTypeIndex;
                type.size = 0;
                type.astIndex = index;

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
                            return false;
                        }
                    }

                    int typeIndex = GetTypeTableIndexForId(typeTable, typeNode.typeAnnotation.id); 
                    if(typeIndex == -1)
                    {
                        printf("[ERROR] undefined type '%s' for argument '%s' in function '%s'\n", typeNode.typeAnnotation.id, idNode.identifier.value, functionName);
                        return false;
                    }

                    Parameter param = {0};
                    param.name = idNode.identifier.value;
                    param.typeTableIndex = typeIndex;

                    if(typeNode.typeAnnotation.isArrayType) 
                    {
                        param.isArray = true;
                        param.arraySize = typeNode.typeAnnotation.arrayDim;
                    }

                    PushParam(&(type.paramList), param);
                }

                PushType(typeTable, type);
            }
        }
    }

    return true;
}

// TODO: improve the type error messages
int TypeCheckNode(AST *ast, Index nodeIndex, TypeTable *typeTable, int currentFunctionTypeIndex, int parentTypeIndex, SymbolTable *localSymbolTable)
{
    if(ast == 0) return -1;
    if(nodeIndex >= ast->nodeCount) return -1;
    if(typeTable == 0) return -1;

    Node node = ast->nodeList[nodeIndex];

    switch (node.type)
    {
    case NODE_STATEMENT_LIST: 
    {
        for(int n = 0; n < node.statementList.statementCount; n++) 
        {
            TypeCheckNode(ast, node.statementList.statements[n], typeTable, currentFunctionTypeIndex, parentTypeIndex, localSymbolTable);
        }
    }
    break;

    case NODE_ASSIGN_STATEMENT:
    {
        int leftTypeIndex = TypeCheckNode(ast, node.assignStmt.lValue, typeTable, currentFunctionTypeIndex, parentTypeIndex, localSymbolTable);
        int rightTypeIndex = TypeCheckNode(ast, node.assignStmt.expression, typeTable, currentFunctionTypeIndex, parentTypeIndex, localSymbolTable);

        if(leftTypeIndex == -1) 
        {
             // type inference for variable declaration without type annotation
            Node varDeclNode = ast->nodeList[node.assignStmt.lValue];
            if(varDeclNode.type == NODE_VARIABLE_DECLARATION && !varDeclNode.varDecl.isTypeAnnotated) 
            {
                leftTypeIndex = rightTypeIndex;
                Node idNode = ast->nodeList[varDeclNode.varDecl.id];
                int symbolIndex = GetSymbolTableIndexForId(localSymbolTable, idNode.identifier.value);
                Symbol *symbol = &localSymbolTable->symbols[symbolIndex];
                symbol->typeTableIndex = rightTypeIndex;

                // TODO: tranasfer the isArray value from right expression to left expression
            }
        }
        
        if(leftTypeIndex != rightTypeIndex) 
        {
            printf("[TYPE ERROR] assignment statment : left = '%d' and right = '%d'\n", leftTypeIndex, rightTypeIndex);
        }
    }
    break;

    case NODE_OPERATOR:
    {
        int leftTypeIndex = TypeCheckNode(ast, node.operator.left, typeTable, currentFunctionTypeIndex, parentTypeIndex, localSymbolTable);

        int rightTypeIndex = -1;
        if(node.operator.opType == BOOL_OP_NOT) 
        {
            rightTypeIndex = GetTypeTableIndexForId(typeTable, "bool");
        }
        else 
        {
            rightTypeIndex = TypeCheckNode(ast, node.operator.right, typeTable, currentFunctionTypeIndex, parentTypeIndex, localSymbolTable);
        }
        
        if(leftTypeIndex != rightTypeIndex) 
        {
            printf("[TYPE ERROR] mismatch in operator parameter types , %d != %d\n", leftTypeIndex, rightTypeIndex);
        }

        int opType = node.operator.opType;

        bool isComparatorOp = opType == COMPARE_OP_EQ_EQ || opType == COMPARE_OP_GT || 
                            opType == COMPARE_OP_GT_EQ || opType == COMPARE_OP_LT || 
                            opType == COMPARE_OP_LT_EQ || opType == COMPARE_OP_NOT_EQ;

        if (isComparatorOp) {
            if (leftTypeIndex != GetTypeTableIndexForId(typeTable, "int")) {
                printf("[TYPE ERROR] compare operator can only operate on int type\n");
            }
            return GetTypeTableIndexForId(typeTable, "bool");
        }

        bool isArithmeticOp = opType == ARITHMETIC_OP_ADD || opType == ARITHMETIC_OP_SUB || 
                                opType == ARITHMETIC_OP_MUL || opType == ARITHMETIC_OP_DIV || opType == ARITHMETIC_OP_MOD;

        if (isArithmeticOp) {
            if (leftTypeIndex != GetTypeTableIndexForId(typeTable, "int")) {
                printf("[TYPE ERROR] arithmetic operator can only operate on int type\n");
            }
        }

        bool isBooleanOp = opType == BOOL_OP_AND || opType == BOOL_OP_NOT || opType == BOOL_OP_OR;

        if (isBooleanOp) {
            if (leftTypeIndex != GetTypeTableIndexForId(typeTable, "bool")) {
                printf("[TYPE ERROR] boolean operator can only operate on boolean type\n");
            }
        }

        return leftTypeIndex;
    }
    break;

    case NODE_L_VALUE:
    {
        int prevTypeIndex = -1;
        for(int n = 0; n < node.lValue.simpleLValueCount; n++)
        {
            if(n == 0) {
                int typeIndex = TypeCheckNode(ast, node.lValue.simpleLValues[n], typeTable, currentFunctionTypeIndex, parentTypeIndex, localSymbolTable);
                prevTypeIndex = typeIndex;
            } else {
                int typeIndex = TypeCheckNode(ast, node.lValue.simpleLValues[n], typeTable, currentFunctionTypeIndex, prevTypeIndex, localSymbolTable);
                prevTypeIndex = typeIndex;
            }
        }

        return prevTypeIndex;
    }
    break;

    case NODE_FUNCTION_CALL:
    {
        // check for implicit function
        if(!strcmp("make", node.functionCall.id)) {
            if (node.functionCall.argumentCount > 0 && node.functionCall.argumentCount <= 2) {

                Node lValue = ast->nodeList[node.functionCall.arguments[0]];
                if (lValue.type != NODE_L_VALUE || lValue.lValue.simpleLValueCount > 1) {
                    printf("[FUNCTION CALL ERROR] expected type name as argument for make funciton\n");
                    return -1;
                }
                
                Node id = ast->nodeList[lValue.lValue.simpleLValues[0]];

                if(id.type != NODE_IDENTIFIER) {
                    printf("[FUNCTION CALL ERROR] expected type name as argument for make funciton\n");
                    return -1;
                }

                int typeIndex = GetTypeTableIndexForId(typeTable, id.identifier.value);
                if (typeIndex == -1) {
                    printf("[FUNCTION CALL ERROR] undefined type '%s' passed to make function\n", id.identifier.value);
                    return -1; 
                }

                if (node.functionCall.argumentCount == 2) {
                    int sizeSymbolTypeIndex = TypeCheckNode(ast, node.functionCall.arguments[1], typeTable, currentFunctionTypeIndex, parentTypeIndex, localSymbolTable);
                    if (sizeSymbolTypeIndex != GetTypeTableIndexForId(typeTable, "int")) {
                        printf("[FUNCTION CALL ERROR] make function expects size to be of type 'int' but found '%d'\n", sizeSymbolTypeIndex);
                        return -1;
                    }
                }

                return typeIndex;

            } else {
                printf("[FUNCTION CALL ERROR] Expected max 2 parameters for function '%s', but found '%d' parameter\n", node.functionCall.id, node.functionCall.argumentCount);
                return -1;
            }
        } else if (!strcmp("print", node.functionCall.id)) {
            return -1;
        }

        // check function name
        int functionTypeIndex = GetTypeTableIndexForFunctionId(typeTable, node.functionCall.id);
        if(functionTypeIndex == -1) 
        {
            printf("[FUNCTION CALL ERROR] reference to undefined function '%s'\n", node.functionCall.id);
            return -1;
        }

        Type functionType = typeTable->types[functionTypeIndex];
        
        // check argument count
        if(functionType.paramList.count != node.functionCall.argumentCount) 
        {
            printf("[FUNCTION CALL ERROR] Expected '%d' parameter for function '%s', but found '%d' parameter\n", functionType.paramList.count, node.functionCall.id, node.functionCall.argumentCount);
            return -1;
        }

        // check arguments types
        for(int n = 0; n < node.functionCall.argumentCount; n++) 
        {
            int passedArguementTypeIndex = TypeCheckNode(ast, node.functionCall.arguments[n], typeTable, currentFunctionTypeIndex, parentTypeIndex, localSymbolTable);
            
            if(passedArguementTypeIndex == -1) return -1;

            int requiredParamTypeIndex = functionType.paramList.symbols[n].typeTableIndex;

            if(passedArguementTypeIndex != requiredParamTypeIndex) {
                printf("[FUNCTION CALL ERROR] Type mismatch for parameter '%s' of function '%s'\n", functionType.paramList.symbols[n].name, functionType.id);
                return -1;
            }       
        }

        return functionType.returnTypeIndex;
    }
    break;

    case NODE_VARIABLE_DECLARATION:
    {
        Node idNode = ast->nodeList[node.varDecl.id];

        // check in local symbol table
        int localSymbolTableIndex = GetSymbolTableIndexForId(localSymbolTable, idNode.identifier.value);

        if(localSymbolTableIndex != -1)
        {
            printf("[SYMBOL ERROR] local variable with name '%s' already declared\n", idNode.identifier.value);
            return -1;
        }
        
        // check in function parameter list
        int paramSymbolIndex = GetSymbolTableIndexForId(&typeTable->types[currentFunctionTypeIndex].paramList, idNode.identifier.value);
    
        if(paramSymbolIndex != -1)
        {
            printf("[SYMBOL ERROR] parameter with name '%s' already declared for the function, cannot decalre a local variable with same name\n", idNode.identifier.value);
            return -1;
        }

        Symbol localSymbol = {0};
        localSymbol.name = idNode.identifier.value;
        localSymbol.typeTableIndex = -1;

        // check if type is declared
        if(node.varDecl.isTypeAnnotated)
        {
            Node typeNode = ast->nodeList[node.varDecl.type];

            // search in type table
            int typeIndex = GetTypeTableIndexForId(typeTable, typeNode.typeAnnotation.id);
            if(typeIndex == -1)
            {
                printf("[TYPE ERROR] undefined type '%s' for variable: %s\n", typeNode.typeAnnotation.id, idNode.identifier.value);
                return -1;
            }
            else 
            {
                if(typeTable->types[typeIndex].isFunction) 
                {
                    printf("[TYPE ERROR] cannot declare variable of type function, variable '%s' has type '%s' which is a function\n", idNode.identifier.value, typeTable->types[typeIndex].id);
                    return -1;
                }
            }

            localSymbol.typeTableIndex = typeIndex;
            localSymbol.isArray = typeNode.typeAnnotation.isArrayType;
        }

        PushSymbol(localSymbolTable, localSymbol);

        return localSymbol.typeTableIndex;
    }
    break;

    case NODE_IDENTIFIER:
    {
        Symbol *symbol = 0;

        // if it has parent type, check if the current identifier is it's field
        if(parentTypeIndex != -1)
        {
            int fieldIndex = GetSymbolTableIndexForId(&typeTable->types[parentTypeIndex].fieldList, node.identifier.value);

            if(fieldIndex == -1)
            {
                printf("[ERROR] undefined field '%s'\n", node.identifier.value);
                return -1;
            }

            symbol = &typeTable->types[parentTypeIndex].fieldList.symbols[fieldIndex];
        }

        // check in local symbol table
        if(symbol == NULL)
        {
            int localSymbolTableIndex = GetSymbolTableIndexForId(localSymbolTable, node.identifier.value);

            if(localSymbolTableIndex != -1)
            {
                symbol = &localSymbolTable->symbols[localSymbolTableIndex];
            }
        }
        
        // check in function parameter list
        if(symbol == NULL)
        {
            int paramSymbolIndex = GetSymbolTableIndexForId(&typeTable->types[currentFunctionTypeIndex].paramList, node.identifier.value);
        
            if(paramSymbolIndex != -1)
            {
                symbol = &typeTable->types[currentFunctionTypeIndex].paramList.symbols[paramSymbolIndex];
            }
        }
        
        if(symbol != NULL) 
        {
            return symbol->typeTableIndex;
        }

        printf("[ERROR] undefined variable '%s'\n", node.identifier.value);
        return -1;
    }
    break;

    case NODE_ARRAY_ACCESS:
    {
        Node idNode = ast->nodeList[node.arrayAccess.id];

        Symbol *symbol = NULL;

        // if has parent type, check if the current identifier is its field
        if(parentTypeIndex != -1)
        {
            int fieldIndex = GetSymbolTableIndexForId(&typeTable->types[parentTypeIndex].fieldList, idNode.identifier.value);
            if(fieldIndex == -1)
            {
                printf("[ERROR] undefined field '%s'\n", idNode.identifier.value);
                return -1;
            }

            symbol = &typeTable->types[parentTypeIndex].fieldList.symbols[fieldIndex];             
        }

        // check in local symbol table
        if(symbol == NULL)
        {
            int localSymbolTableIndex = GetSymbolTableIndexForId(localSymbolTable, idNode.identifier.value);

            if(localSymbolTableIndex != -1)
            {
                symbol = &localSymbolTable->symbols[localSymbolTableIndex]; 
            }
        }

        // check in function parameter list
        if(symbol == NULL)
        {
            int paramSymbolIndex = GetSymbolTableIndexForId(&typeTable->types[currentFunctionTypeIndex].paramList, idNode.identifier.value);
            
            if(paramSymbolIndex != -1)
            {            
                symbol = &typeTable->types[currentFunctionTypeIndex].paramList.symbols[paramSymbolIndex]; 
            }
        }

        if(symbol != NULL) 
        {
            if(!symbol->isArray)
            {
                printf("[ERROR] symbol '%s' is not of type array, cannot be indexed as an array\n", idNode.identifier.value);
                return -1;
            }

            // check array index expression type
            int arrayIndexExprType = TypeCheckNode(ast, node.arrayAccess.expr, typeTable, currentFunctionTypeIndex, parentTypeIndex, localSymbolTable);
            int integerType = GetTypeTableIndexForId(typeTable, "int");

            if(arrayIndexExprType != integerType) 
            {
                printf("[ERROR] symbol '%s': array indexing expression should be of type integer but is of type '%s'\n", idNode.identifier.value, typeTable->types[arrayIndexExprType].id);
                return -1;
            }

            return symbol->typeTableIndex;
        }

        printf("[ERROR] undefined variable '%s'\n", idNode.identifier.value);
        return -1;
    }
    break;

    case NODE_IF_STATEMENT:
    {
        int conditionExprType = TypeCheckNode(ast, node.ifStmt.conditionExpr, typeTable, currentFunctionTypeIndex, parentTypeIndex, localSymbolTable);
        int boolTypeIndex = GetTypeTableIndexForId(typeTable, "bool");

        if(conditionExprType != boolTypeIndex)
        {
            printf("[TYPE ERROR] if statement: type of conditional expression should evaluate to boolean\n");
        }

        TypeCheckNode(ast, node.ifStmt.trueBlock, typeTable, currentFunctionTypeIndex, parentTypeIndex, localSymbolTable);
        if(node.ifStmt.falseBlockExist)
        {
            TypeCheckNode(ast, node.ifStmt.falseBlock, typeTable, currentFunctionTypeIndex, parentTypeIndex, localSymbolTable);
        }
    }
    break;

    case NODE_WHILE_STATEMENT:
    {
        int conditionExprType = TypeCheckNode(ast, node.whileStmt.conditionExpr, typeTable, currentFunctionTypeIndex, parentTypeIndex, localSymbolTable);
        int boolTypeIndex = GetTypeTableIndexForId(typeTable, "bool");

        if(conditionExprType != boolTypeIndex)
        {
            printf("[TYPE ERROR] while statement: type of conditional expression should evaluate to boolean\n");
        }

        TypeCheckNode(ast, node.whileStmt.block, typeTable, currentFunctionTypeIndex, parentTypeIndex, localSymbolTable);
    }
    break;

    case NODE_RETURN_STATEMENT:
    {
        int retExprTypeIndex = -1; 
        if(node.returnStmt.exprExist)
        {
            retExprTypeIndex = TypeCheckNode(ast, node.returnStmt.expression, typeTable, currentFunctionTypeIndex, parentTypeIndex, localSymbolTable);
        }
        else 
        {
            retExprTypeIndex = GetTypeTableIndexForId(typeTable, "void");
        }

        int definedRetExprTypeIndex = typeTable->types[currentFunctionTypeIndex].returnTypeIndex;
        if(definedRetExprTypeIndex != retExprTypeIndex) 
        {
            printf("[TYPE ERROR] function defines return type as '%d', but returns expression of type '%d'\n", definedRetExprTypeIndex, retExprTypeIndex);
        }
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

    case NODE_CHARACTER_CONSTANT:
    {
        return GetTypeTableIndexForId(typeTable, "char");
    }
    break;

    case NODE_STRING_CONSTANT:
    {
        return GetTypeTableIndexForId(typeTable, "string");
    }
    break;

    default:
        printf("Unsupported node for type checking: %d\n", node.type);
        return -1;
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
                int typeIndex = GetTypeTableIndexForId(typeTable, defNode.functionDef.name);
                TypeCheckNode(ast, defNode.functionDef.body, typeTable, typeIndex, -1, &typeTable->types[typeIndex].localSymbolList);
            }
        }
    }
}

void PrintTypeInfo(Type type)
{
    printf("name: '%s', size: %u, is_struct: %d, is_function: %d, return type: %d\n", type.id, type.size, type.isStruct, type.isFunction, type.returnTypeIndex);
    if (type.isStruct)
    {
        for(int n = 0; n < type.fieldList.count; n++)
        {
            printf("    %d. field_name: '%s', type_index : %d, is_array: %d, array_size: %d\n", n + 1, 
                                type.fieldList.symbols[n].name, 
                                type.fieldList.symbols[n].typeTableIndex,
                                type.fieldList.symbols[n].isArray,
                                type.fieldList.symbols[n].arraySize);
        }
    } else if(type.isFunction) {
        for(int n = 0; n < type.paramList.count; n++)
        {
            printf("    - param_name: '%s', type_index : %d, is_array: %d, array_size: %d\n", 
                                type.paramList.symbols[n].name, 
                                type.paramList.symbols[n].typeTableIndex,
                                type.paramList.symbols[n].isArray,
                                type.paramList.symbols[n].arraySize);
        }

        for(int n = 0; n < type.localSymbolList.count; n++)
        {
            printf("    - local_symbol_name: '%s', type_index : %d, is_array: %d, array_size: %d\n", 
                                type.localSymbolList.symbols[n].name, 
                                type.localSymbolList.symbols[n].typeTableIndex,
                                type.localSymbolList.symbols[n].isArray,
                                type.localSymbolList.symbols[n].arraySize);
        }
    }
}

void PrintTypeTable(TypeTable typeTable)
{
    printf("\nGlobal Type Table\n");

    for(int n = 0; n < typeTable.count; n++) 
    {
        printf("[%d] ", n);
        PrintTypeInfo(typeTable.types[n]);
    }

    printf("\n");
}
