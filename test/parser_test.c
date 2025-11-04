#include "..\source\error.c"
#include "..\source\lexer.c"
#include "..\source\ast.c"
#include "..\source\parser.c"

#include <assert.h>

void Test_ParseEmptyFunction()
{
    printf("\n======== %s =======\n", __func__);

    char *source = "fn main() {}";
    LoadedFile file = {.source = {.data = source, .length = strlen(source)}, .isLoaded = true};

    AST ast = {0};
    InitAST(&ast);

    Parser parser = {0};
    parser.loadedFile = file;
    parser.tokenList = TokenizeSource(file);
    ast.parser = parser;

    Index rootIndex = ParseProgram(&ast, &parser);

    Node rootNode = ast.nodeList[rootIndex];
    assert(rootNode.type == NODE_PROGRAM);
    assert(rootNode.program.defCount == 1);

    Node funcDefNode = ast.nodeList[rootNode.program.definitions[0]];
    assert(funcDefNode.type == NODE_FUNCTION_DEFINITION);
    assert(strcmp(funcDefNode.functionDef.name, "main") == 0);
    assert(funcDefNode.functionDef.parameterCount == 0);
    assert(funcDefNode.functionDef.isReturnTypeDeclared == false);

    Node bodyNode = ast.nodeList[funcDefNode.functionDef.body];
    assert(bodyNode.type == NODE_STATEMENT_LIST);
    assert(bodyNode.statementList.statementCount == 0);

    printf("%s: OK\n", __func__);
}

void Test_ParseStructDefinition()
{
    printf("\n======== %s =======\n", __func__);

    char *source = "struct Point { x: int; y: int; }";
    LoadedFile file = {.source = {.data = source, .length = strlen(source)}, .isLoaded = true};

    AST ast = {0};
    InitAST(&ast);

    Parser parser = {0};
    parser.loadedFile = file;
    parser.tokenList = TokenizeSource(file);
    ast.parser = parser;

    Index rootIndex = ParseProgram(&ast, &parser);

    Node rootNode = ast.nodeList[rootIndex];
    assert(rootNode.type == NODE_PROGRAM);
    assert(rootNode.program.defCount == 1);

    Node structDefNode = ast.nodeList[rootNode.program.definitions[0]];
    assert(structDefNode.type == NODE_STRUCT_DEFINITION);
    assert(strcmp(structDefNode.structDef.name, "Point") == 0);
    assert(structDefNode.structDef.fieldCount == 2);

    Node field1 = ast.nodeList[structDefNode.structDef.fields[0]];
    assert(field1.type == NODE_FIELD);
    Node field1_id = ast.nodeList[field1.field.id];
    Node field1_type = ast.nodeList[field1.field.type];
    assert(strcmp(field1_id.identifier.value, "x") == 0);
    assert(strcmp(field1_type.typeAnnotation.id, "int") == 0);

    Node field2 = ast.nodeList[structDefNode.structDef.fields[1]];
    assert(field2.type == NODE_FIELD);
    Node field2_id = ast.nodeList[field2.field.id];
    Node field2_type = ast.nodeList[field2.field.type];
    assert(strcmp(field2_id.identifier.value, "y") == 0);
    assert(strcmp(field2_type.typeAnnotation.id, "int") == 0);

    printf("%s: OK\n", __func__);
}

void Test_ParseVariableDeclarationAndAssignment()
{
    printf("\n======== %s =======\n", __func__);

    char *source = "fn main() { let x: int = 10; y = x + 5; }";
    LoadedFile file = {.source = {.data = source, .length = strlen(source)}, .isLoaded = true};

    AST ast = {0};
    InitAST(&ast);

    Parser parser = {0};
    parser.loadedFile = file;
    parser.tokenList = TokenizeSource(file);
    ast.parser = parser;

    Index rootIndex = ParseProgram(&ast, &parser);
    Node funcDefNode = ast.nodeList[ast.nodeList[rootIndex].program.definitions[0]];
    Node bodyNode = ast.nodeList[funcDefNode.functionDef.body];
    assert(bodyNode.type == NODE_STATEMENT_LIST);
    assert(bodyNode.statementList.statementCount == 2);

    // Statement 1: let x: int = 10;
    Node stmt1 = ast.nodeList[bodyNode.statementList.statements[0]];
    assert(stmt1.type == NODE_ASSIGN_STATEMENT);
    Node varDecl = ast.nodeList[stmt1.assignStmt.lValue];
    assert(varDecl.type == NODE_VARIABLE_DECLARATION);
    assert(varDecl.varDecl.isTypeAnnotated == true);
    Node varId = ast.nodeList[varDecl.varDecl.id];
    assert(strcmp(varId.identifier.value, "x") == 0);
    Node varType = ast.nodeList[varDecl.varDecl.type];
    assert(strcmp(varType.typeAnnotation.id, "int") == 0);
    Node expr1 = ast.nodeList[stmt1.assignStmt.expression];
    assert(expr1.type == NODE_INTEGER_CONSTANT);
    assert(expr1.integer.value == 10);

    // Statement 2: y = x + 5;
    Node stmt2 = ast.nodeList[bodyNode.statementList.statements[1]];
    assert(stmt2.type == NODE_ASSIGN_STATEMENT);
    Node lval2 = ast.nodeList[stmt2.assignStmt.lValue];
    assert(lval2.type == NODE_L_VALUE);
    Node lval2_id = ast.nodeList[lval2.lValue.simpleLValues[0]];
    assert(strcmp(lval2_id.identifier.value, "y") == 0);

    Node expr2 = ast.nodeList[stmt2.assignStmt.expression];
    assert(expr2.type == NODE_OPERATOR);
    assert(expr2.operator.opType == ARITHMETIC_OP_ADD);

    printf("%s: OK\n", __func__);
}

void Test_ParseFunctionWithParamsAndReturn()
{
    printf("\n======== %s =======\n", __func__);

    char *source = "fn add(a: int, b: int): int { return a + b; }";
    LoadedFile file = {.source = {.data = source, .length = strlen(source)}, .isLoaded = true};

    AST ast = {0};
    InitAST(&ast);

    Parser parser = {0};
    parser.loadedFile = file;
    parser.tokenList = TokenizeSource(file);
    ast.parser = parser;

    Index rootIndex = ParseProgram(&ast, &parser);
    Node rootNode = ast.nodeList[rootIndex];
    assert(rootNode.program.defCount == 1);

    Node funcDefNode = ast.nodeList[rootNode.program.definitions[0]];
    assert(funcDefNode.type == NODE_FUNCTION_DEFINITION);
    assert(strcmp(funcDefNode.functionDef.name, "add") == 0);
    assert(funcDefNode.functionDef.parameterCount == 2);
    assert(funcDefNode.functionDef.isReturnTypeDeclared == true);

    // Check parameters
    Node param1 = ast.nodeList[funcDefNode.functionDef.parameters[0]];
    assert(param1.type == NODE_PARAM);
    assert(strcmp(ast.nodeList[param1.param.id].identifier.value, "a") == 0);
    assert(strcmp(ast.nodeList[param1.param.type].typeAnnotation.id, "int") == 0);

    Node param2 = ast.nodeList[funcDefNode.functionDef.parameters[1]];
    assert(param2.type == NODE_PARAM);
    assert(strcmp(ast.nodeList[param2.param.id].identifier.value, "b") == 0);
    assert(strcmp(ast.nodeList[param2.param.type].typeAnnotation.id, "int") == 0);

    // Check return type
    Node returnType = ast.nodeList[funcDefNode.functionDef.returnType];
    assert(returnType.type == NODE_TYPE_ANNOTATION);
    assert(strcmp(returnType.typeAnnotation.id, "int") == 0);

    // Check body
    Node bodyNode = ast.nodeList[funcDefNode.functionDef.body];
    assert(bodyNode.statementList.statementCount == 1);
    Node returnNode = ast.nodeList[bodyNode.statementList.statements[0]];
    assert(returnNode.type == NODE_RETURN_STATEMENT);
    assert(returnNode.returnStmt.exprExist == true);

    printf("%s: OK\n", __func__);
}

void Test_ParseIfElseStatement()
{
    printf("\n======== %s =======\n", __func__);

    char *source = "fn main() { if (a > b) { x = 1; } else { x = 2; } }";
    LoadedFile file = {.source = {.data = source, .length = strlen(source)}, .isLoaded = true};

    AST ast = {0};
    InitAST(&ast);

    Parser parser = {0};
    parser.loadedFile = file;
    parser.tokenList = TokenizeSource(file);
    ast.parser = parser;

    Index rootIndex = ParseProgram(&ast, &parser);
    Node funcDefNode = ast.nodeList[ast.nodeList[rootIndex].program.definitions[0]];
    Node bodyNode = ast.nodeList[funcDefNode.functionDef.body];
    assert(bodyNode.statementList.statementCount == 1);

    Node ifNode = ast.nodeList[bodyNode.statementList.statements[0]];
    assert(ifNode.type == NODE_IF_STATEMENT);
    assert(ifNode.ifStmt.falseBlockExist == true);

    // Condition
    Node condExpr = ast.nodeList[ifNode.ifStmt.conditionExpr];
    assert(condExpr.type == NODE_OPERATOR);
    assert(condExpr.operator.opType == COMPARE_OP_GT);

    // True block
    Node trueBlock = ast.nodeList[ifNode.ifStmt.trueBlock];
    assert(trueBlock.type == NODE_STATEMENT_LIST);
    assert(trueBlock.statementList.statementCount == 1);

    // False block
    Node falseBlock = ast.nodeList[ifNode.ifStmt.falseBlock];
    assert(falseBlock.type == NODE_STATEMENT_LIST);
    assert(falseBlock.statementList.statementCount == 1);

    printf("%s: OK\n", __func__);
}

void Test_ParseWhileStatement()
{
    printf("\n======== %s =======\n", __func__);

    char *source = "fn main() { while (i < 10) { i = i + 1; } }";
    LoadedFile file = {.source = {.data = source, .length = strlen(source)}, .isLoaded = true};

    AST ast = {0};
    InitAST(&ast);

    Parser parser = {0};
    parser.loadedFile = file;
    parser.tokenList = TokenizeSource(file);
    ast.parser = parser;

    Index rootIndex = ParseProgram(&ast, &parser);
    Node funcDefNode = ast.nodeList[ast.nodeList[rootIndex].program.definitions[0]];
    Node bodyNode = ast.nodeList[funcDefNode.functionDef.body];
    assert(bodyNode.statementList.statementCount == 1);

    Node whileNode = ast.nodeList[bodyNode.statementList.statements[0]];
    assert(whileNode.type == NODE_WHILE_STATEMENT);

    // Condition
    Node condExpr = ast.nodeList[whileNode.whileStmt.conditionExpr];
    assert(condExpr.type == NODE_OPERATOR);
    assert(condExpr.operator.opType == COMPARE_OP_LT);

    // Block
    Node block = ast.nodeList[whileNode.whileStmt.block];
    assert(block.type == NODE_STATEMENT_LIST);
    assert(block.statementList.statementCount == 1);

    printf("%s: OK\n", __func__);
}

void Test_ParseComplexExpression()
{
    printf("\n======== %s =======\n", __func__);

    // Test for operator precedence (a + b * c)
    char *source = "fn main() { x = a + b * c; }";
    LoadedFile file = {.source = {.data = source, .length = strlen(source)}, .isLoaded = true};

    AST ast = {0};
    InitAST(&ast);

    Parser parser = {0};
    parser.loadedFile = file;
    parser.tokenList = TokenizeSource(file);
    ast.parser = parser;

    Index rootIndex = ParseProgram(&ast, &parser);
    Node funcDefNode = ast.nodeList[ast.nodeList[rootIndex].program.definitions[0]];
    Node bodyNode = ast.nodeList[funcDefNode.functionDef.body];
    Node assignNode = ast.nodeList[bodyNode.statementList.statements[0]];
    Node exprNode = ast.nodeList[assignNode.assignStmt.expression];

    // Root of expression should be '+'
    assert(exprNode.type == NODE_OPERATOR);
    assert(exprNode.operator.opType == ARITHMETIC_OP_ADD);

    // Left of '+' should be 'a'
    Node leftPlus = ast.nodeList[exprNode.operator.left];
    assert(leftPlus.type == NODE_L_VALUE);

    // Right of '+' should be the '*' expression
    Node rightPlus = ast.nodeList[exprNode.operator.right];
    assert(rightPlus.type == NODE_OPERATOR);
    assert(rightPlus.operator.opType == ARITHMETIC_OP_MUL);

    printf("%s: OK\n", __func__);
}

void Test_ParseFunctionCallWithArgs()
{
    printf("\n======== %s =======\n", __func__);

    char *source = "fn main() { add(1, 2); }";
    LoadedFile file = {.source = {.data = source, .length = strlen(source)}, .isLoaded = true};

    AST ast = {0};
    InitAST(&ast);

    Parser parser = {0};
    parser.loadedFile = file;
    parser.tokenList = TokenizeSource(file);
    ast.parser = parser;

    Index rootIndex = ParseProgram(&ast, &parser);
    Node funcDefNode = ast.nodeList[ast.nodeList[rootIndex].program.definitions[0]];
    Node bodyNode = ast.nodeList[funcDefNode.functionDef.body];
    Node funcCallNode = ast.nodeList[bodyNode.statementList.statements[0]];

    assert(funcCallNode.type == NODE_FUNCTION_CALL);
    assert(strcmp(funcCallNode.functionCall.id, "add") == 0);
    assert(funcCallNode.functionCall.argumentCount == 2);

    Node arg1 = ast.nodeList[funcCallNode.functionCall.arguments[0]];
    assert(arg1.type == NODE_INTEGER_CONSTANT);
    assert(arg1.integer.value == 1);

    Node arg2 = ast.nodeList[funcCallNode.functionCall.arguments[1]];
    assert(arg2.type == NODE_INTEGER_CONSTANT);
    assert(arg2.integer.value == 2);

    printf("%s: OK\n", __func__);
}

void Test_ParseArrayAndMemberAccess()
{
    printf("\n======== %s =======\n", __func__);

    char *source = "fn main() { let arr: [10]int; x = p.arr[i + 1]; }";
    LoadedFile file = {.source = {.data = source, .length = strlen(source)}, .isLoaded = true};

    AST ast = {0};
    InitAST(&ast);

    Parser parser = {0};
    parser.loadedFile = file;
    parser.tokenList = TokenizeSource(file);
    ast.parser = parser;

    Index rootIndex = ParseProgram(&ast, &parser);
    Node funcDefNode = ast.nodeList[ast.nodeList[rootIndex].program.definitions[0]];
    Node bodyNode = ast.nodeList[funcDefNode.functionDef.body];
    assert(bodyNode.statementList.statementCount == 2);

    // Test array declaration: let arr: [10]int;
    Node varDeclAssign = ast.nodeList[bodyNode.statementList.statements[0]];
    assert(varDeclAssign.type == NODE_VARIABLE_DECLARATION);
    Node typeNode = ast.nodeList[varDeclAssign.varDecl.type];
    assert(typeNode.type == NODE_TYPE_ANNOTATION);
    assert(typeNode.typeAnnotation.isArrayType == true);
    assert(typeNode.typeAnnotation.arrayDim == 10);
    assert(strcmp(typeNode.typeAnnotation.id, "int") == 0);

    // Test member and array access: x = p.arr[i + 1];
    Node assignNode = ast.nodeList[bodyNode.statementList.statements[1]];
    Node lvalNode = ast.nodeList[assignNode.assignStmt.expression];
    assert(lvalNode.type == NODE_L_VALUE);
    assert(lvalNode.lValue.simpleLValueCount == 2);

    Node member1 = ast.nodeList[lvalNode.lValue.simpleLValues[0]];
    assert(member1.type == NODE_IDENTIFIER);
    assert(strcmp(member1.identifier.value, "p") == 0);

    Node member2 = ast.nodeList[lvalNode.lValue.simpleLValues[1]];
    assert(member2.type == NODE_ARRAY_ACCESS);
    assert(strcmp(ast.nodeList[member2.arrayAccess.id].identifier.value, "arr") == 0);

    Node indexExpr = ast.nodeList[member2.arrayAccess.expr];
    assert(indexExpr.type == NODE_OPERATOR);
    assert(indexExpr.operator.opType == ARITHMETIC_OP_ADD);

    printf("%s: OK\n", __func__);
}

int main(int argc, char **argv)
{
    Test_ParseEmptyFunction();
    Test_ParseStructDefinition();
    Test_ParseVariableDeclarationAndAssignment();
    Test_ParseFunctionWithParamsAndReturn();
    Test_ParseIfElseStatement();
    Test_ParseWhileStatement();
    Test_ParseComplexExpression();
    Test_ParseFunctionCallWithArgs();
    Test_ParseArrayAndMemberAccess();
    return 0;
}