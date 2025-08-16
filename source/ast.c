#include "ast.h"

// TODO: need to increase based on requirement
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

void PrintRawString(AST ast, Location location) {
    int length = location.end.index - location.start.index + 1;

    char *sourceSpan = (char *)malloc(length + 1); 
    sourceSpan[length] = 0;

    for(int n = location.start.index; n <= location.end.index; n++) {
        char c = ast.parser.loadedFile.source.data[n];
        if (c == '\n') c = ' ';
        sourceSpan[n - location.start.index] = c;
    }

    printf("span: '%s'\n", sourceSpan);
}

void PrintNode(AST ast, Index index, int indent)
{
    if(index == -1) return;

    Node node = ast.nodeList[index];

    for(int n = 0; n < indent; n++) printf("   ");
    PrintRawString(ast, node.location);

    for(int n = 0; n < indent; n++) printf("   ");
    printf("+- ");
    indent++;
        
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
        
        case NODE_STRUCT_DEFINITION:
        {
            printf("struct def: '%s'\n", node.structDef.name);
            
            for(int n = 0; n < node.structDef.fieldCount; n++)
            {
                PrintNode(ast, node.structDef.fields[n], indent);
            }
        };
        break;
        
        case NODE_FUNCTION_DEFINITION:
        {
            printf("function def: '%s'\n", node.functionDef.name);
            
            for(int n = 0; n < node.functionDef.parameterCount; n++)
            {
                PrintNode(ast, node.functionDef.parameters[n], indent);
            }

            if(node.functionDef.isReturnTypeDeclared)
            {
                PrintNode(ast, node.functionDef.returnType, indent);
            }

            PrintNode(ast, node.functionDef.body, indent);
        }
        break;

        case NODE_VARIABLE_DECLARATION:
        {
            printf("var decl : \n");
            PrintNode(ast, node.varDecl.id, indent);

            if(node.varDecl.isTypeAnnotated)
            {
                PrintNode(ast, node.varDecl.type, indent);
            }
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

        case NODE_FUNCTION_CALL:
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

        case NODE_CHARACTER_CONSTANT:
        {
            printf("character const: '%c'\n", node.character.value);
        }
        break;

        case NODE_BOOLEAN_CONSTANT:
        {
            printf("boolean const: '%s'\n", node.boolean.isTrue ? "true" : "false");
        }
        break;

        default: 
        {
            printf("not implemented\n");
        }
        break;
    }
}

char* NodeTypeToString(unsigned int nodeType) {
    switch (nodeType)
    {
        case NODE_PROGRAM: return "NODE_PROGRAM";
        case NODE_FUNCTION_DEFINITION: return "NODE_FUNCTION_DEFINITION";
        case NODE_STRUCT_DEFINITION: return "NODE_STRUCT_DEFINITION";
        case NODE_VARIABLE_DECLARATION: return "NODE_VARIABLE_DECLARATION";
        case NODE_FIELD: return "NODE_FIELD";
        case NODE_PARAM: return "NODE_PARAM";
        case NODE_L_VALUE: return "NODE_L_VALUE";
        case NODE_ARRAY_ACCESS: return "NODE_ARRAY_ACCESS";
        case NODE_OPERATOR: return "NODE_OPERATOR";
        case NODE_STATEMENT_LIST: return "NODE_STATEMENT_LIST";
        case NODE_ASSIGN_STATEMENT: return "NODE_ASSIGN_STATEMENT";
        case NODE_IF_STATEMENT: return "NODE_IF_STATEMENT";
        case NODE_WHILE_STATEMENT: return "NODE_WHILE_STATEMENT";
        case NODE_RETURN_STATEMENT: return "NODE_RETURN_STATEMENT";
        case NODE_FUNCTION_CALL: return "NODE_FUNCTION_CALL";
        case NODE_IDENTIFIER: return "NODE_IDENTIFIER";
        case NODE_INTEGER_CONSTANT: return "NODE_INTEGER_CONSTANT";
        case NODE_STRING_CONSTANT: return "NODE_STRING_CONSTANT";
        case NODE_CHARACTER_CONSTANT: return "NODE_CHARACTER_CONSTANT";
        case NODE_BOOLEAN_CONSTANT: return "NODE_BOOLEAN_CONSTANT";
        case NODE_TYPE_ANNOTATION: return "NODE_TYPE_ANNOTATION";
        case NODE_STRUCT_INITIALIZATION: return "NODE_STRUCT_INITIALIZATION";
        case NODE_STRUCT_FIELD_INITIALIZATION: return "NODE_STRUCT_FIELD_INITIALIZATION";
        case NODE_ARRAY_INTITIALIZATION: return "NODE_ARRAY_INTITIALIZATION";
        default: return "unknown_node";
    }
}

char* OperatorToString(unsigned int operatorType) {
    switch (operatorType) {
        case NONE_OP: return "none"; 
        case ARITHMETIC_OP_ADD: return "+"; 
        case ARITHMETIC_OP_SUB: return "-"; 
        case ARITHMETIC_OP_MUL: return "*"; 
        case ARITHMETIC_OP_DIV: return "/"; 
        case ARITHMETIC_OP_MOD: return "%%"; 
        case COMPARE_OP_LT: return "<"; 
        case COMPARE_OP_GT: return ">"; 
        case COMPARE_OP_EQ_EQ: return "=="; 
        case COMPARE_OP_NOT_EQ: return "!="; 
        case COMPARE_OP_LT_EQ: return "<="; 
        case COMPARE_OP_GT_EQ: return ">="; 
        case BOOL_OP_AND: return "&&"; 
        case BOOL_OP_OR: return "||"; 
        case BOOL_OP_NOT: return "!"; 
        default: return "unknown_operator";
    }
}

void GetNodeLabel(Node node, char *label, int labelMaxLength) {

    switch (node.type)
    {
        case NODE_PROGRAM: sprintf_s(label, labelMaxLength, "program"); break;
        case NODE_FUNCTION_DEFINITION: sprintf_s(label, labelMaxLength, "function: %s", node.functionDef.name); break;
        case NODE_STRUCT_DEFINITION: sprintf_s(label, labelMaxLength, "struct: %s", node.structDef.name); break;
        case NODE_VARIABLE_DECLARATION: sprintf_s(label, labelMaxLength, "var decl"); break;
        case NODE_FIELD: sprintf_s(label, labelMaxLength, "field"); break;
        case NODE_PARAM: sprintf_s(label, labelMaxLength, "parameter"); break;
        case NODE_L_VALUE: sprintf_s(label, labelMaxLength, "l value"); break;
        case NODE_ARRAY_ACCESS: sprintf_s(label, labelMaxLength, "array access"); break;
        case NODE_OPERATOR: sprintf_s(label, labelMaxLength, "op: %s", OperatorToString(node.operator.opType)); break;
        case NODE_STATEMENT_LIST: sprintf_s(label, labelMaxLength, "statement list"); break;
        case NODE_ASSIGN_STATEMENT: sprintf_s(label, labelMaxLength, "assign stmt"); break;
        case NODE_IF_STATEMENT: sprintf_s(label, labelMaxLength, "if stmt"); break;
        case NODE_WHILE_STATEMENT: sprintf_s(label, labelMaxLength, "while stmt"); break;
        case NODE_RETURN_STATEMENT: sprintf_s(label, labelMaxLength, "return stmt"); break;
        case NODE_FUNCTION_CALL: sprintf_s(label, labelMaxLength, "function call: %s()", node.functionCall.id); break;
        case NODE_IDENTIFIER: sprintf_s(label, labelMaxLength, "id: %s", node.identifier.value); break;
        case NODE_INTEGER_CONSTANT: sprintf_s(label, labelMaxLength, "integer: %d", node.integer.value); break;
        case NODE_STRING_CONSTANT: sprintf_s(label, labelMaxLength, "str: %s", node.string.value); break;
        case NODE_CHARACTER_CONSTANT: sprintf_s(label, labelMaxLength, "char: %c", node.character.value); break;
        case NODE_BOOLEAN_CONSTANT: sprintf_s(label, labelMaxLength, "bool: %s", node.boolean.isTrue ? "true" : "false"); break;
        case NODE_TYPE_ANNOTATION: sprintf_s(label, labelMaxLength, "type: %s", node.typeAnnotation.id); break;
        case NODE_STRUCT_INITIALIZATION: sprintf_s(label, labelMaxLength, "struct init"); break;
        case NODE_STRUCT_FIELD_INITIALIZATION: sprintf_s(label, labelMaxLength, "struct field init"); break;
        case NODE_ARRAY_INTITIALIZATION: sprintf_s(label, labelMaxLength, "array init"); break;
        default: sprintf_s(label, labelMaxLength, "unknown_node"); break;
    }
}

void CreateEdge(FILE *file, AST ast, Index parentIndex,  Index childIndex) {
    
    if (childIndex == -1) return;

    Node childNode = ast.nodeList[childIndex];

    if (parentIndex != -1) {
        Node parentNode = ast.nodeList[parentIndex];
        fprintf(file, "\t%s_%d -> %s_%d\n",  NodeTypeToString(parentNode.type), parentIndex, NodeTypeToString(childNode.type), childIndex);
    }

    Node node = childNode;

    switch(childNode.type)
    {
        case NODE_PROGRAM:
        {            
            for(int n = 0; n < node.program.defCount; n++)
            {
                CreateEdge(file, ast, childIndex, node.program.definitions[n]);
            }
        };
        break;
        
        case NODE_STRUCT_DEFINITION:
        {            
            for(int n = 0; n < node.structDef.fieldCount; n++)
            {
                CreateEdge(file, ast, childIndex, node.structDef.fields[n]);
            }
        };
        break;
        
        case NODE_FUNCTION_DEFINITION:
        {            
            for(int n = 0; n < node.functionDef.parameterCount; n++)
            {
                CreateEdge(file, ast, childIndex, node.functionDef.parameters[n]);
            }

            if(node.functionDef.isReturnTypeDeclared)
            {
                CreateEdge(file, ast, childIndex, node.functionDef.returnType);
            }

            CreateEdge(file, ast, childIndex, node.functionDef.body);
        }
        break;

        case NODE_VARIABLE_DECLARATION:
        {
            CreateEdge(file, ast, childIndex, node.varDecl.id);

            if(node.varDecl.isTypeAnnotated)
            {
                CreateEdge(file, ast, childIndex, node.varDecl.type);
            }
        }
        break;

        case NODE_FIELD:
        {
            CreateEdge(file, ast, childIndex, node.field.id);
            CreateEdge(file, ast, childIndex, node.field.type);
        }
        break;

        case NODE_PARAM:
        {
            CreateEdge(file, ast, childIndex, node.param.id);
            CreateEdge(file, ast, childIndex, node.param.type);
        }
        break;

        case NODE_TYPE_ANNOTATION: 
        {
        }
        break;

        case NODE_L_VALUE:
        {
            for(int n = 0; n < node.lValue.simpleLValueCount; n++) 
            {
                CreateEdge(file, ast, childIndex, node.lValue.simpleLValues[n]);
            }
        }
        break;

        case NODE_ARRAY_ACCESS:
        {
            CreateEdge(file, ast, childIndex, node.arrayAccess.id);
            CreateEdge(file, ast, childIndex, node.arrayAccess.expr);
        }
        break;

        case NODE_FUNCTION_CALL:
        {
            for(int n = 0; n < node.functionCall.argumentCount; n++)
            {
                CreateEdge(file, ast, childIndex, node.functionCall.arguments[n]);
            }
        }
        break;

        case NODE_STATEMENT_LIST:
        {
            for(int n = 0; n < node.statementList.statementCount; n++)
            {
                CreateEdge(file, ast, childIndex, node.statementList.statements[n]);
            }
        }
        break;

        case NODE_ASSIGN_STATEMENT:
        {
            CreateEdge(file, ast, childIndex, node.assignStmt.lValue);
            CreateEdge(file, ast, childIndex, node.assignStmt.expression);
        }
        break;

        case NODE_IF_STATEMENT:
        {
            CreateEdge(file, ast, childIndex, node.ifStmt.conditionExpr);
            CreateEdge(file, ast, childIndex, node.ifStmt.trueBlock);

            if(node.ifStmt.falseBlockExist)
            {
                CreateEdge(file, ast, childIndex, node.ifStmt.falseBlock);
            }
        }
        break;

        case NODE_WHILE_STATEMENT:
        {
            CreateEdge(file, ast, childIndex, node.whileStmt.conditionExpr);
            CreateEdge(file, ast, childIndex, node.whileStmt.block);
        }
        break;

        case NODE_RETURN_STATEMENT:
        {
            if(node.returnStmt.exprExist) 
                CreateEdge(file, ast, childIndex, node.returnStmt.expression);            
        }   
        break;
        
        case NODE_OPERATOR:
        {
            CreateEdge(file, ast, childIndex, node.operator.left);            
            
            if(node.operator.opType != BOOL_OP_NOT) 
            {
                CreateEdge(file, ast, childIndex, node.operator.right);            
            }
        }
        break;
        
        case NODE_IDENTIFIER:
        {
            // printf("id: '%s'\n", node.identifier.value);
        }
        break;
        
        case NODE_INTEGER_CONSTANT:
        {
            // printf("integer const: '%d'\n", node.integer.value);
        }
        break;
        
        case NODE_STRING_CONSTANT:
        {
            // printf("string const: '%s'\n", node.string.value);
        }
        break;

        case NODE_CHARACTER_CONSTANT:
        {
            // printf("character const: '%c'\n", node.character.value);
        }
        break;

        case NODE_BOOLEAN_CONSTANT:
        {
            // printf("boolean const: '%s'\n", node.boolean.isTrue ? "true" : "false");
        }
        break;

        default: 
        {
            // printf("not implemented\n");
        }
        break;
    }

}

void WriteAsDotFile(AST ast, Index rootIndex, char *fileName) {

    FILE *file = fopen(fileName, "w+");

    if (file) {
        fprintf(file,"digraph AST {\n");

        for(int n = 0; n < ast.nodeCount; n++) {
            Node node = ast.nodeList[n];
            char label[250] = {0};
            GetNodeLabel(node, label, 250);
            fprintf(file, "\t%s_%d [shape=rectangle, label=\"%s\"]\n", NodeTypeToString(node.type), n, label);
        }

        CreateEdge(file, ast, -1, rootIndex);

        fprintf(file,"}");
        fclose(file);
    }
}