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
        // TODO: resize list of nodes stored in AST struct
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
            
            for(int n = 0; n < node.program.defCount; n++)
            {
                PrintNode(ast, node.program.definitions[n], indent);
            }
        };
        break;
        
        case NODE_STRUCT_DEF:
        {
            printf("struct def: '%s'\n", node.structDef.name);
            
            for(int n = 0; n < node.structDef.fieldCount; n++)
            {
                PrintNode(ast, node.structDef.fields[n], indent);
            }
        };
        break;
        
        case NODE_FUNC_DEF:
        {
            printf("function def: '%s'\n", node.functionDef.name);
            
            for(int n = 0; n < node.functionDef.parameterCount; n++)
            {
                PrintNode(ast, node.functionDef.parameters[n], indent);
            }

            PrintNode(ast, node.functionDef.returnType, indent);
            PrintNode(ast, node.functionDef.body, indent);
        }
        break;

        case NODE_VAR_DECL:
        {
            printf("var decl : \n");
            PrintNode(ast, node.varDecl.id, indent);
            PrintNode(ast, node.varDecl.type, indent);
        }
        break;

        case NODE_FIELD:
        {
            printf("field: \n");
            PrintNode(ast, node.field.id, indent);
            PrintNode(ast, node.field.type, indent);
        }
        break;

        case NODE_PARAM:
        {
            printf("param: \n");
            PrintNode(ast, node.param.id, indent);
            PrintNode(ast, node.param.type, indent);
        }
        break;

        case NODE_TYPE_ANNOTATION: 
        {
            printf("type: id: '%s', is_array: %s, dim: %d\n", node.typeAnnotation.id, node.typeAnnotation.isArrayType ? "true" : "false", node.typeAnnotation.arrayDim);
        }
        break;

        case NODE_L_VALUE:
        {
            printf("l value:\n");
            for(int n = 0; n < node.lValue.simpleLValueCount; n++) 
            {
                PrintNode(ast, node.lValue.simpleLValues[n], indent);
            }
        }
        break;

        case NODE_ARRAY_ACCESS:
        {
            printf("array access:\n");
            PrintNode(ast, node.arrayAccess.id, indent);
            PrintNode(ast, node.arrayAccess.expr, indent);
        }
        break;

        case NODE_FUNC_CALL:
        {
            printf("function call: '%s()'\n", node.functionCall.id);

            for(int n = 0; n < node.functionCall.argumentCount; n++)
            {
                PrintNode(ast, node.functionCall.arguments[n], indent);                
            }
        }
        break;

        case NODE_STATEMENT_LIST:
        {
            printf("statement block: '%s'\n", node.statementList.statementCount == 0 ? "{empty}" : "{}");

            for(int n = 0; n < node.statementList.statementCount; n++)
            {
                PrintNode(ast, node.statementList.statements[n], indent);
            }
        }
        break;

        case NODE_ASSIGN_STATEMENT:
        {
            printf("assignment statement: '='\n");
            PrintNode(ast, node.assignStmt.lValue, indent);
            PrintNode(ast, node.assignStmt.expression, indent);
        }
        break;

        case NODE_IF_STATEMENT:
        {
            printf("if statement:\n");
            PrintNode(ast, node.ifStmt.conditionExpr, indent);
            PrintNode(ast, node.ifStmt.trueBlock, indent);

            if(node.ifStmt.falseBlockExist)
            {
                PrintNode(ast, node.ifStmt.falseBlock, indent);
            }
        }
        break;

        case NODE_WHILE_STATEMENT:
        {
            printf("while statement:\n");
            PrintNode(ast, node.whileStmt.conditionExpr, indent);
            PrintNode(ast, node.whileStmt.block, indent);
        }
        break;

        case NODE_RETURN_STATEMENT:
        {
            printf("return statement:\n");
            if(node.returnStmt.exprExist) PrintNode(ast, node.returnStmt.expression, indent);
        }   
        break;
        
        case NODE_OPERATOR:
        {
            if(node.operator.opType == ARITHMETIC_OP_ADD)
            {
                printf("math_op: '+'\n");
            }
            else if(node.operator.opType == ARITHMETIC_OP_SUB)
            {
                printf("math_op: '-'\n");
            }
            else if(node.operator.opType == ARITHMETIC_OP_MUL)
            {
                printf("math_op: '*'\n");
            }
            else if(node.operator.opType == ARITHMETIC_OP_DIV)
            {
                printf("math_op: '/'\n");
            }
            else if(node.operator.opType == ARITHMETIC_OP_MOD)
            {
                printf("math_op: '%%'\n");
            }
            else if(node.operator.opType == COMPARE_OP_LT)
            {
                printf("compare_op: '<'\n");
            }
            else if(node.operator.opType == COMPARE_OP_GT)
            {
                printf("compare_op: '>'\n");
            }
            else if(node.operator.opType == COMPARE_OP_LT_EQ)
            {
                printf("compare_op: '<='\n");
            }
            else if(node.operator.opType == COMPARE_OP_GT_EQ)
            {
                printf("compare_op: '>='\n");
            }
            else if(node.operator.opType == COMPARE_OP_EQ_EQ)
            {
                printf("compare_op: '=='\n");
            }
            else if(node.operator.opType == COMPARE_OP_NOT_EQ)
            {
                printf("compare_op: '!='\n");
            }
            else if(node.operator.opType == BOOL_OP_AND)
            {
                printf("boolean_op: '&&'\n");
            }
            else if(node.operator.opType == BOOL_OP_OR)
            {
                printf("boolean_op: '||'\n");
            }
            else if(node.operator.opType == BOOL_OP_NOT)
            {
                printf("boolean_op: '!'\n");
            }

            PrintNode(ast, node.operator.left, indent);

            if(node.operator.opType != BOOL_OP_NOT) 
            {
                PrintNode(ast, node.operator.right, indent);
            }
        }
        break;
        
        case NODE_IDENTIFIER:
        {
            printf("id: '%s'\n", node.identifier.value);
        }
        break;
        
        case NODE_INTEGER_CONSTANT:
        {
            printf("integer const: '%d'\n", node.integer.value);
        }
        break;
        
        case NODE_STRING_CONSTANT:
        {
            printf("string const: '%s'\n", node.string.value);
        }
        break;

        default: 
        {
            printf("not implemented\n");
        }
        break;
    }
}