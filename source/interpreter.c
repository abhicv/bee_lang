#include "ast.h"
#include "symbol.h"
#include "assert.h"

enum ResultType {
    INT = 0,
    STRUCT,
    ARRAY,
    POINTER,
};

typedef struct {
    enum ResultType type;
    union {
        int value;
        struct {
            int pointer;
            int size;
        };
    };
} Result;

// NOTE: no memory management as of now, allocate all struct and array in heap memory
// allow prebuild functions such as:
/*
make -> allocate struct and array in heap

let s : Point2d = make(Point2d)
let s : [] Point2d = make(Point2d, 10)

len() -> give back size of array

*/

Result EvaluateNode(AST ast, Index index, Result *stack, int *stackIndex, TypeTable typeTable, int currentFunctionTypeIndex) 
{
    assert(index > -1);

    Node node = ast.nodeList[index];

    switch (node.type) {
        case NODE_STATEMENT_LIST:
        {
            for(int n = 0; n < node.statementList.statementCount; n++) {
                EvaluateNode(ast, node.statementList.statements[n], stack, stackIndex, typeTable, currentFunctionTypeIndex);
            }
        }
        break;

        case NODE_VARIABLE_DECLARATION:
        {
            Node idNode = ast.nodeList[node.varDecl.id];

            Type functionType = typeTable.types[currentFunctionTypeIndex]; 
            int paramCount = functionType.paramList.count;
    
            LocalSymbolList localSymbolList = functionType.localSymbolList;

            int localSymbolPosition = -1;
            for(int n = 0; n < localSymbolList.count; n++) {
                Symbol symbol = localSymbolList.symbols[n];
                if(!strcmp(idNode.identifier.value, symbol.name)) {
                    localSymbolPosition = n;
                    break;
                }
            }                        

            int basePointer = (*stackIndex) - (paramCount + localSymbolList.count);

            int stackPosition = (basePointer + localSymbolPosition);
            
            Result result = {0};
            result.type = POINTER;
            result.pointer = stackPosition;

            return result;
        }
        break;

        case NODE_ASSIGN_STATEMENT:
        {
            Result left = EvaluateNode(ast, node.assignStmt.lValue, stack, stackIndex, typeTable, currentFunctionTypeIndex);
            
            assert(left.type == POINTER);

            Result right = EvaluateNode(ast, node.assignStmt.expression, stack, stackIndex, typeTable, currentFunctionTypeIndex);
            stack[left.pointer] = right;
        }
        break;

        case NODE_FUNCTION_CALL:
        {
            if(!strcmp("print", node.functionCall.id)) {
                for(int n = 0; n < node.functionCall.argumentCount; n++) {
                    Index argNodeIndex = node.functionCall.arguments[n];
                    Result result = EvaluateNode(ast, argNodeIndex, stack, stackIndex, typeTable, currentFunctionTypeIndex);
                    printf("%d, ", result.value);
                }
                printf("\n");
            }
        }
        break;

        case NODE_L_VALUE: 
        {
            return EvaluateNode(ast, node.lValue.simpleLValues[0], stack, stackIndex, typeTable, currentFunctionTypeIndex);
        }
        break;

        case NODE_IDENTIFIER: 
        {
            Type functionType = typeTable.types[currentFunctionTypeIndex]; 

            int symbolTableIndex = GetSymbolTableIndexForId(&functionType.paramList, node.identifier.value);

            if (symbolTableIndex != -1) {
                int basePointer = (*stackIndex) - (functionType.localSymbolList.count + functionType.paramList.count);
                int stackPosition = basePointer + symbolTableIndex;
                printf("sp: %d\n", stackPosition);
                return stack[stackPosition];
            }   

            symbolTableIndex = GetSymbolTableIndexForId(&functionType.localSymbolList, node.identifier.value);
            
            if(symbolTableIndex != -1) {
                int basePointer = (*stackIndex) - (functionType.localSymbolList.count);
                int stackPosition = basePointer + symbolTableIndex;
                return stack[stackPosition];
            }
        }
        break;

        case NODE_INTEGER_CONSTANT:
        {
            Result result = {0};
            result.type = INT;
            result.value = node.integer.value;
            return result;
        }   
        break;

        case NODE_OPERATOR:
        {
            Result left = EvaluateNode(ast, node.operator.left, stack, stackIndex, typeTable, currentFunctionTypeIndex);
            Result right = EvaluateNode(ast, node.operator.right, stack, stackIndex, typeTable, currentFunctionTypeIndex);
            
            Result result = {0};
            result.type = INT;

            switch(node.operator.opType) {
                case ARITHMETIC_OP_ADD:
                {
                    result.value = left.value + right.value;
                }
                break;

                case ARITHMETIC_OP_SUB:
                {
                    result.value = left.value - right.value;
                }
                break;

                case ARITHMETIC_OP_MUL:
                {
                    result.value = left.value * right.value;
                }
                break;

                case ARITHMETIC_OP_DIV:
                {
                    assert(right.value != 0);
                    result.value = left.value / right.value;
                }
                break;

                case ARITHMETIC_OP_MOD:
                {
                    assert(right.value != 0);
                    result.value = left.value % right.value;
                }
                break;

            }
            return result;
        }
        break;

        default: {
            break;
        }
    }

    return (Result){.type = -1};
}

// NOTE: pushing params and then the local symbols of the function
int CalculateLocalSymbolsAndParamsStackSize(TypeTable typeTable, Type functionType) {

    int totalSize = 0;

    for(int n = 0; n < functionType.paramList.count; n++) {
        Parameter param =  functionType.paramList.symbols[n];
        Type type = typeTable.types[param.typeTableIndex];
        totalSize += type.size;
    }

    for(int n = 0; n < functionType.localSymbolList.count; n++) {
        Symbol symbol = functionType.localSymbolList.symbols[n];
        Type type = typeTable.types[symbol.typeTableIndex];
        totalSize += type.size;
    }

    return totalSize;
}

void InterpretAST(AST ast, TypeTable typeTable) {

    // find node index of main function definition
    Index mainFunctionDefIndex = -1;
    for(int n = 0; n < ast.nodeCount; n++) {
        Node node = ast.nodeList[n];
        if(node.type == NODE_FUNCTION_DEFINITION && !strcmp("main", node.functionDef.name)) {
            mainFunctionDefIndex = n;
            break;
        }
    }

    if (mainFunctionDefIndex == -1) {
        printf("[ERRRO] Cannot find main method\n");
    }

    // NOTE: guarnteed to get an index 
    int functionTypeTableIndex = GetTypeTableIndexForFunctionId(&typeTable, "main");

    // printf("main: %d\n", mainFunctionDefIndex);

    // TODO: push main function arguments on to the stack

    // NOTE: a stack to store a computed values while evaluating the AST
    Result stack[1024] = {0};
    int stackIndex = 0;    

    // printf("before: %d\n", stackIndex);
    stackIndex += CalculateLocalSymbolsAndParamsStackSize(typeTable, typeTable.types[functionTypeTableIndex]);
    // printf("after: %d\n", stackIndex);

    EvaluateNode(ast, ast.nodeList[mainFunctionDefIndex].functionDef.body, stack, &stackIndex, typeTable, functionTypeTableIndex);
}
