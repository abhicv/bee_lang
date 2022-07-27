#include "ast.h"

#define MAX_NODE_COUNT 1000

void InitAST(AST *ast)
{
    ast->nodeList = (Node*)malloc(sizeof(Node) * MAX_NODE_COUNT);
    ast->nodeCount = 0;
}

Index PushNode(AST *ast, Node node)
{
    if(ast->nodeCount < MAX_NODE_COUNT)
    {
        Index index = ast->nodeCount;
        ast->nodeList[index] = node;
        ast->nodeCount++;
        return index;
    }
    else
    {
        printf("ast error: AST node list full! (capacity: %d nodes)\n", MAX_NODE_COUNT);
        exit(1);
    }
}

void PushIndex(Index **indexList, unsigned int *indexCount, Index index)
{
    if((*indexCount) == 0)
    {
        (*indexCount)++;
        (*indexList) = (Index*)malloc(sizeof(Index));
    }
    else
    {
        (*indexCount)++;
        (*indexList) = (Index*)realloc((*indexList), sizeof(Index) * (*indexCount));
    }

    (*indexList)[(*indexCount) - 1] = index;
}

void PrintNode(AST ast, Index index, int indent)
{
    for(int n = 0; n < indent; n++) printf("   ");
    printf("+- ");
    indent++;
    
    Node node = ast.nodeList[index];
    
    switch(node.type)
    {
        case NODE_PROGRAM:
        {
            printf("program:\n");
            
            for(int n = 0; n < node.indexCount; n++)
            {
                PrintNode(ast, node.indexList[n], indent);
            }
        };
        break;
        
        case NODE_STRUCT_DEF:
        {
            printf("struct def: '%s'\n", node.structName);
            
            for(int n = 0; n < node.fieldIndexCount; n++)
            {
                PrintNode(ast, node.fieldIndexList[n], indent);
            }
        };
        break;
        
        case NODE_PROC_DEF:
        {
            printf("function def: '%s'\n", node.procName);
            
            for(int n = 0; n < node.parameterIndexCount; n++)
            {
                PrintNode(ast, node.parameterIndexList[n], indent);
            }
            
            for(int n = 0; n < node.returnIndexCount; n++)
            {
                PrintNode(ast, node.returnIndexList[n], indent);
            }

            PrintNode(ast, node.procBody, indent);
        }
        break;

        case NODE_VAR_DECL:
        {
            printf("var decl : id: '%s', type: '%s'\n", node.identifier, node.isTypeDeclared ? node.typeId : "(infer type)");
        }
        break; 

        case NODE_FIELD:
        {
            printf("field: id: '%s', type: '%s'\n", ast.nodeList[node.left].identifier, ast.nodeList[node.right].identifier);
        }
        break;
        
        case NODE_PROC_CALL:
        {
            printf("function call: '%s()'\n", node.procId);

            for(int n = 0; n < node.argumentsListCount; n++)
            {
                PrintNode(ast, node.argumentsIndexList[n], indent);                
            }
        }
        break;

        case NODE_STATEMENT_LIST:
        {
            printf("statement block: '%s'\n", node.statementIndexCount == 0 ? "{empty}" : "{}");

            for(int n = 0; n < node.statementIndexCount; n++)
            {
                PrintNode(ast, node.statementIndexList[n], indent);
            }
        }
        break;

        case NODE_ASSIGN_STATEMENT:
        {
            printf("assign statement: '='\n");
            PrintNode(ast, node.left, indent);
            PrintNode(ast, node.right, indent);
        }
        break;

        case NODE_IF_STATEMENT:
        {
            printf("if statement:\n");
            PrintNode(ast, node.conditionExpr, indent);
            PrintNode(ast, node.trueBlock, indent);

            if(node.falseBlockExist)
            {
                PrintNode(ast, node.falseBlock, indent);
            }
        }
        break;

        case NODE_WHILE_STATEMENT:
        {
            printf("while statement:\n");
            PrintNode(ast, node.conditionExpr, indent);
            PrintNode(ast, node.trueBlock, indent);
        }
        break;

        case NODE_RETURN_STATEMENT:
        {
            printf("return statement:\n");
            if(node.returnExprExist)
                PrintNode(ast, node.returnExpr, indent);
        }
        break;
        
        case NODE_OPERATOR:
        {
            if(node.opType == ARITHMETIC_OP_ADD)
            {
                printf("math_op: '+'\n");
            }
            else if(node.opType == ARITHMETIC_OP_SUB)
            {
                printf("math_op: '-'\n");
            }
            else if(node.opType == ARITHMETIC_OP_MUL)
            {
                printf("math_op: '*'\n");
            }
            else if(node.opType == ARITHMETIC_OP_DIV)
            {
                printf("math_op: '/'\n");
            }
            else if(node.opType == ARITHMETIC_OP_MOD)
            {
                printf("math_op: '%%'\n");
            }
            else if(node.opType == COMPARE_OP_LT)
            {
                printf("compare_op: '<'\n");
            }
            else if(node.opType == COMPARE_OP_GT)
            {
                printf("compare_op: '>'\n");
            }
            else if(node.opType == COMPARE_OP_LT_EQ)
            {
                printf("compare_op: '<='\n");
            }
            else if(node.opType == COMPARE_OP_GT_EQ)
            {
                printf("compare_op: '>='\n");
            }
            else if(node.opType == COMPARE_OP_EQ_EQ)
            {
                printf("compare_op: '=='\n");
            }
            else if(node.opType == COMPARE_OP_NOT_EQ)
            {
                printf("compare_op: '!='\n");
            }
            else if(node.opType == BOOL_OP_AND)
            {
                printf("boolean_op: '&&'\n");
            }
            else if(node.opType == BOOL_OP_OR)
            {
                printf("boolean_op: '||'\n");
            }
            else if(node.opType == BOOL_OP_NOT)
            {
                printf("boolean_op: '!'\n");
            }

            PrintNode(ast, node.left, indent);

            if(node.opType != BOOL_OP_NOT)
                PrintNode(ast, node.right, indent);
        }
        break;
        
        case NODE_IDENTIFIER:
        {
            printf("id: '%s'\n", node.identifier);
        }
        break;
        
        case NODE_INTEGER_CONSTANT:
        {
            printf("int const: '%d'\n", node.value);
        }
        break;
        
        case NODE_STRING_CONSTANT:
        {
            printf("str const: '%s'\n", node.stringValue);
        }
        break;
        
        default: 
        {
            printf("not implemented\n");
        }
        break;
    }
}