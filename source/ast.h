#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "parser.h"

enum NodeType
{
    NODE_PROGRAM = 1,
    NODE_FUNCTION_DEFINITION,
    NODE_STRUCT_DEFINITION,
    NODE_VARIABLE_DECLARATION,
    NODE_FIELD,
    NODE_PARAM,
    NODE_L_VALUE,
    NODE_ARRAY_ACCESS,
    NODE_OPERATOR,
    NODE_STATEMENT_LIST,
    NODE_ASSIGN_STATEMENT,
    NODE_IF_STATEMENT,
    NODE_WHILE_STATEMENT,
    NODE_RETURN_STATEMENT,
    NODE_FUNCTION_CALL,
    NODE_IDENTIFIER,
    NODE_INTEGER_CONSTANT,
    NODE_STRING_CONSTANT,
    NODE_CHARACTER_CONSTANT,
    NODE_BOOLEAN_CONSTANT,
    NODE_TYPE_ANNOTATION,

    NODE_STRUCT_INITIALIZATION,
    NODE_STRUCT_FIELD_INITIALIZATION,
    NODE_ARRAY_INTITIALIZATION
};

enum OperatorType
{
    NONE_OP = 0, // not an operator type
    ARITHMETIC_OP_ADD,
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

typedef int Index;

typedef struct {
    unsigned int type;
    unsigned int typeTableIndex;
    
    union
    {
        struct
        {
            Index *definitions;
            unsigned int defCount;
        } program;
    
        struct
        {
            const char *name;
            Index *fields;
            unsigned int fieldCount;
        } structDef;
        
        struct
        {
            const char *name; 
            Index *parameters;
            unsigned int parameterCount;
            Index returnType;
            bool isReturnTypeDeclared;
            Index body;
        } functionDef;

        struct
        {
            const char *id;
            Index *arguments;
            unsigned int argumentCount;
        } functionCall;
 
        struct
        {
            Index *statements;
            unsigned int statementCount;
        } statementList;

        struct 
        {
            unsigned int opType;
            Index left;
            Index right;
        } operator;

        struct 
        {
            Index lValue;
            Index expression;
        } assignStmt;

        struct 
        {
            Index conditionExpr;
            Index block;
        } whileStmt;

        struct 
        {
            Index conditionExpr;
            Index trueBlock;
            bool falseBlockExist;
            Index falseBlock;
        } ifStmt;

        struct 
        {
            Index expression;
            bool exprExist;
        } returnStmt;
        
        struct         
        {
            const char* id;
            unsigned int arrayDim;
            bool isArrayType;
        } typeAnnotation;

        struct 
        {
            Index id;
            Index type;
            bool isTypeAnnotated;
        } varDecl, param, field;

        struct 
        {
            Index *simpleLValues;
            unsigned int simpleLValueCount;
        } lValue;

        struct 
        {
            Index id;
            Index expr;            
        } arrayAccess;

        struct
        {
            int value;
        } integer;

        struct
        {
            char value;
        } character;

        struct
        {
            bool isTrue;
        } boolean;

        struct
        {
            const char* value;
        } string, identifier;
    };
} Node;

typedef struct {
    Node *nodeList;
    unsigned int nodeCount;
    Parser parser;
} AST;

void InitAST(AST *ast);
Index PushNode(AST *ast, Node node);
void PushIndex(Index **indexList, unsigned int *indexCount, Index index);

#endif