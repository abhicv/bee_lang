#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

enum NodeType
{
    NODE_PROGRAM = 1,
    NODE_PROC_DEF,
    NODE_STRUCT_DEF,
    NODE_VAR_DECL,
    NODE_FIELD,
    NODE_OPERATOR,
    NODE_STATEMENT_LIST,
    NODE_ASSIGN_STATEMENT,
    NODE_IF_STATEMENT,
    NODE_WHILE_STATEMENT,
    NODE_RETURN_STATEMENT,
    NODE_PROC_CALL,
    NODE_IDENTIFIER,
    NODE_INTEGER_CONSTANT,
    NODE_STRING_CONSTANT,    
};

enum Operators
{
    ARITHMETIC_OP_ADD = 1,
    ARITHMETIC_OP_SUB,
    ARITHMETIC_OP_MUL,
    ARITHMETIC_OP_DIV,
    ARITHMETIC_OP_MOD,

    COMPARE_OP_LT,
    COMPARE_OP_GT,
    COMPARE_OP_EQ_EQ,
    COMPARE_OP_NOT_EQ,
    COMPARE_OP_LT_EQ,
    COMPARE_OP_GT_EQ,

    BOOL_OP_AND,
    BOOL_OP_OR,
    BOOL_OP_NOT,
};

typedef unsigned int Index;

typedef struct Node Node;
struct Node
{
    unsigned int type;
    
    union
    {
        struct //program
        {
            Index *indexList;
            unsigned int indexCount;
        };
        
        struct //struct def
        {
            const char *structName;
            Index *fieldIndexList;
            unsigned int fieldIndexCount;
        };
        
        struct // procedure def
        {
            const char *procName;
            Index *parameterIndexList;
            unsigned int parameterIndexCount;
            Index *returnIndexList;
            unsigned int returnIndexCount;
            Index procBody;
        };

        struct// procedure call
        {
            const char *procId;
            Index *argumentsIndexList;
            unsigned int argumentsListCount;
        };

        struct // statement block
        {
            Index *statementIndexList;
            unsigned statementIndexCount;
        };

        struct // binary operator or assignemnt statement
        {
            unsigned int opType;
            Index left;
            Index right;
        };
        
        struct // while and if-else statement
        {
            Index conditionExpr;
            Index trueBlock;
            bool falseBlockExist;
            Index falseBlock;
        };
        
        struct //return statement
        {
            Index returnExpr;
            bool returnExprExist;
        };

        struct //identifier or var decl
        {
            const char *identifier;
            const char *typeId;
            bool isTypeDeclared;
        };

        struct //integer constant
        {
            int value;
        };

        struct // string constant
        {
            const char *stringValue;
        };
    };
};

typedef struct AST AST;
struct AST
{
    Node *nodeList;
    unsigned int nodeCount;
};

void InitAST(AST *ast);
Index PushNode(AST *ast, Node node);
void PushIndex(Index **indexList, unsigned int *indexCount, Index index);

#endif