#include "parser.h"

Token GetNextToken(Parser *parser)
{
    return parser->tokenList.tokens[parser->tokenIndex++];
}

Token PeekNextToken(Parser *parser)
{
    return parser->tokenList.tokens[parser->tokenIndex];
}

bool AcceptToken(Parser *parser, unsigned int tokenType)
{
    Token token = PeekNextToken(parser);
    
    if(token.type == tokenType)
    {
        GetNextToken(parser);
        return true;
    }
    
    return false;
}

Token ExpectToken(Parser *parser, unsigned int tokenType)
{
    Token token = PeekNextToken(parser);

    if(token.type == tokenType)
    {
        return GetNextToken(parser);
    }
    else
    {
        char errorMsg[500] = {0};
        sprintf(errorMsg, "expected '%s' but found '%s'", TokenTypeToString(tokenType), TokenTypeToString(token.type));
        PrintErrorLocationInSource(parser->loadedFile.data, token.pos - token.size, token.line + 1, token.column + 1, errorMsg);

        Token dummy = {0};
        token.type = tokenType;
        token.identifier = "this_is_a_dummy";
        return token;

        // exit(1);
    }
}

enum OperatorAssociativity {
    LEFT_ASSOCIATIVE = 1,
    RIGHT_ASSOCIATIVE,
};

typedef struct OpInfo {
    unsigned int precedence;
    unsigned int associatvity;
} OpInfo;

OpInfo opInfoTable[] = {
    [ARITHMETIC_OP_ADD] = {.precedence = 4, .associatvity = LEFT_ASSOCIATIVE},
    [ARITHMETIC_OP_SUB] = {.precedence = 4, .associatvity = LEFT_ASSOCIATIVE},
    
    [ARITHMETIC_OP_MUL] = {.precedence = 5, .associatvity = LEFT_ASSOCIATIVE},
    [ARITHMETIC_OP_DIV] = {.precedence = 5, .associatvity = LEFT_ASSOCIATIVE},
    [ARITHMETIC_OP_MOD] = {.precedence = 5, .associatvity = LEFT_ASSOCIATIVE},
    
    [COMPARE_OP_LT] = {.precedence = 3, .associatvity = LEFT_ASSOCIATIVE},
    [COMPARE_OP_GT] = {.precedence = 3, .associatvity = LEFT_ASSOCIATIVE},
    
    [COMPARE_OP_EQ_EQ] = {.precedence = 2, .associatvity = LEFT_ASSOCIATIVE},
    [COMPARE_OP_NOT_EQ] = {.precedence = 2, .associatvity = LEFT_ASSOCIATIVE},
    
    [COMPARE_OP_LT_EQ] = {.precedence = 3, .associatvity = LEFT_ASSOCIATIVE},
    [COMPARE_OP_GT_EQ] = {.precedence = 3, .associatvity = LEFT_ASSOCIATIVE},
    
    [BOOL_OP_AND] = {.precedence = 1, .associatvity = LEFT_ASSOCIATIVE},
    [BOOL_OP_OR] = {.precedence = 1, .associatvity = LEFT_ASSOCIATIVE},
};

bool IsBinOpToken(unsigned int type)
{
    if(type == TOKEN_PLUS ||
       type == TOKEN_MINUS ||
       type == TOKEN_MULTIPLY ||
       type == TOKEN_DIVIDE ||
       type == TOKEN_MODULUS ||
       type == TOKEN_LT ||
       type == TOKEN_GT ||
       type == TOKEN_EQ_EQ ||
       type == TOKEN_NOT_EQ ||
       type == TOKEN_LT_EQ ||
       type == TOKEN_GT_EQ ||
       type == TOKEN_AND ||
       type == TOKEN_OR)
    {
        return true;
    }
    
    return false;
}

Index ParseExpression(AST *ast, Parser *parser, int minPrec);
Index ParseAtom(AST *ast, Parser *parser);
Index ParseStatement(AST *ast, Parser *parser);
Index ParseStatementList(AST *ast, Parser *parser);

// precedence climbing - https://eli.thegreenplace.net/2012/08/02/parsing-expressions-by-precedence-climbing
Index ParseExpression(AST *ast, Parser *parser, int minPrec)
{
    Index left = ParseAtom(ast, parser);
    
    while(true)
    {
        Token next = PeekNextToken(parser);
        if(!IsBinOpToken(next.type)) break;
        
        int prec = opInfoTable[next.opType].precedence; 
        if(prec < minPrec) break;
        
        GetNextToken(parser);
        
        Node node = {0};
        node.type = NODE_OPERATOR;
        node.operator.opType = next.opType;
        node.operator.left = left;
        
        if(opInfoTable[next.opType].associatvity == LEFT_ASSOCIATIVE)
        {
            node.operator.right = ParseExpression(ast, parser, prec + 1);
        }
        else
        {
            node.operator.right = ParseExpression(ast, parser, prec);
        }
        
        left = PushNode(ast, node);
    }
    
    return left;
}

Index ParseFunctionCall(AST *ast, Parser *parser) 
{
    Node node = {0};
    node.type = NODE_FUNCTION_CALL;
    node.functionCall.id = parser->tokenList.tokens[parser->tokenIndex - 2].identifier;
    
    // function arguments
    while(true)
    {
        Token token = PeekNextToken(parser);
        
        if(token.type == TOKEN_RIGHT_PAREN) break;
        else if(token.type == TOKEN_PROGRAM_END) break;
        
        if(node.functionCall.argumentCount > 0) ExpectToken(parser, TOKEN_COMMA);
        
        Index argIndex = ParseExpression(ast, parser, 1);
        PushIndex(&node.functionCall.arguments, &node.functionCall.argumentCount, argIndex);
    }
    
    ExpectToken(parser, TOKEN_RIGHT_PAREN);
    
    return PushNode(ast, node);
}

Index ParseLValue(AST *ast, Parser *parser);

Index ParseAtom(AST *ast, Parser *parser)
{
    if(AcceptToken(parser, TOKEN_IDENTIFIER))
    {
        if(AcceptToken(parser, TOKEN_LEFT_PAREN)) // function call
        {
            return ParseFunctionCall(ast, parser);
        }
        else // l_value
        {
            parser->tokenIndex -= 1;
            return ParseLValue(ast, parser);
        }
    }
    else if(AcceptToken(parser, TOKEN_NOT))
    {
        Node node = {0};
        node.type = NODE_OPERATOR;
        node.operator.opType = BOOL_OP_NOT;
        node.operator.left = ParseAtom(ast, parser);
        return PushNode(ast, node);
    }
    else if(AcceptToken(parser, TOKEN_INTEGER_CONSTANT))
    {
        Node node = {0};
        node.type = NODE_INTEGER_CONSTANT;
        node.integer.value = parser->tokenList.tokens[parser->tokenIndex - 1].integerValue;
        return PushNode(ast, node);
    }
    else if(AcceptToken(parser, TOKEN_STRING_CONSTANT))
    {
        Node node = {0};
        node.type = NODE_STRING_CONSTANT;
        node.string.value = parser->tokenList.tokens[parser->tokenIndex - 1].stringValue;
        return PushNode(ast, node);
    }
    else if(AcceptToken(parser, TOKEN_KEYWORD_TRUE))
    {
        Node node = {0};
        node.type = NODE_BOOLEAN_CONSTANT;
        node.boolean.isTrue = true;
        return PushNode(ast, node);
    }
    else if(AcceptToken(parser, TOKEN_KEYWORD_FALSE))
    {
        Node node = {0};
        node.type = NODE_BOOLEAN_CONSTANT;
        node.boolean.isTrue = false;
        return PushNode(ast, node);
    }
    else if(AcceptToken(parser, TOKEN_LEFT_PAREN))
    {
        Index index = ParseExpression(ast, parser, 1);
        ExpectToken(parser, TOKEN_RIGHT_PAREN);
        return index;
    }
    
    // an atom is always required
    Token next = PeekNextToken(parser);

    char errorMsg[500] = {0};
    sprintf(errorMsg, "expecting an expression before '%s'\n", TokenTypeToString(next.type));
    PrintErrorLocationInSource(parser->loadedFile.data, next.pos, next.line + 1, next.column + 1, errorMsg);
    exit(1);
}

Index ParseArrayAccess(AST *ast, Parser *parser) 
{
    Token id = ExpectToken(parser, TOKEN_IDENTIFIER);

    ExpectToken(parser, TOKEN_LEFT_BRACKET);

    Index expr = ParseExpression(ast, parser, 1);

    ExpectToken(parser, TOKEN_RIGHT_BRACKET);

    Node idNode = {0};
    idNode.type = NODE_IDENTIFIER;
    idNode.identifier.value = id.identifier;

    Node node = {0};
    node.type = NODE_ARRAY_ACCESS;
    node.arrayAccess.id = PushNode(ast, idNode);
    node.arrayAccess.expr = expr;

    return PushNode(ast, node);
}

Index ParseSimpleLValue(AST *ast, Parser *parser) 
{
    Token id = ExpectToken(parser, TOKEN_IDENTIFIER);

    Token next = PeekNextToken(parser);

    if(next.type == TOKEN_LEFT_BRACKET) {
        parser->tokenIndex -= 1;
        return ParseArrayAccess(ast, parser);
    } 
    else 
    {
        Node idNode = {0};
        idNode.type = NODE_IDENTIFIER;
        idNode.identifier.value = id.identifier;
        return PushNode(ast, idNode);
    }
}

Index ParseLValue(AST *ast, Parser *parser)
{
    Node node = {0};
    node.type = NODE_L_VALUE;

    while(true) 
    {
        Index simpleLValueIndex = ParseSimpleLValue(ast, parser);
        PushIndex(&node.lValue.simpleLValues, &node.lValue.simpleLValueCount, simpleLValueIndex);
        Token next = PeekNextToken(parser);
        if(next.type == TOKEN_DOT) GetNextToken(parser);
        else break;
    }

    return PushNode(ast, node);
}

Index ParseAssignmentStatement(AST *ast, Parser *parser)
{
    Index lvalueIndex = ParseLValue(ast, parser);
    ExpectToken(parser, TOKEN_EQUAL);
    Index exprIndex = ParseExpression(ast, parser, 1);
    
    ExpectToken(parser, TOKEN_SEMICOLON);
    
    Node node = {0};
    node.type = NODE_ASSIGN_STATEMENT;
    node.assignStmt.lValue = lvalueIndex;
    node.assignStmt.expression = exprIndex;
    
    return PushNode(ast, node);
}

Index ParseType(AST *ast, Parser *parser) 
{
    Node node = {0};
    node.type = NODE_TYPE_ANNOTATION;
    node.typeAnnotation.isArrayType = false;

    Token next = PeekNextToken(parser);

    if(next.type == TOKEN_IDENTIFIER) 
    {
        node.typeAnnotation.id = next.identifier;
        GetNextToken(parser);
    }
    else if(next.type == TOKEN_LEFT_BRACKET) 
    {
        GetNextToken(parser);
        Token typeId = ExpectToken(parser, TOKEN_IDENTIFIER);
        ExpectToken(parser, TOKEN_RIGHT_BRACKET);
        
        node.typeAnnotation.id = typeId.identifier;
        node.typeAnnotation.isArrayType = true;
        node.typeAnnotation.arrayDim = 0;
    } 
    else {
        char errorMsg[500] = {0};
        sprintf(errorMsg, "expected type, but found %s", TokenTypeToString(next.type));
        PrintErrorLocationInSource(parser->loadedFile.data, next.pos, next.line + 1, next.column + 1, errorMsg);
        exit(1);
    }
    
    return PushNode(ast, node);
}

Index ParseVarDeclStatement(AST *ast, Parser *parser)
{
    ExpectToken(parser, TOKEN_KEYWORD_LET);
    
    Token id = ExpectToken(parser, TOKEN_IDENTIFIER);
    
    Node idNode = {0};
    idNode.type = NODE_IDENTIFIER;
    idNode.identifier.value = id.identifier;
    
    ExpectToken(parser, TOKEN_COLON);
    
    Index typeAnnoIndex = ParseType(ast, parser);
    
    Node node = {0};
    node.type = NODE_VARIABLE_DECLARATION;
    node.varDecl.id = PushNode(ast, idNode);
    node.varDecl.type = typeAnnoIndex;
    
    Token next = PeekNextToken(parser);
    
    // initialization of variable
    if(next.type == TOKEN_EQUAL) {
        GetNextToken(parser);
        
        Index left = PushNode(ast, node);
        Index right = ParseExpression(ast, parser, 1);
        
        ExpectToken(parser, TOKEN_SEMICOLON);
        
        Node assignNode = {0};
        assignNode.type = NODE_ASSIGN_STATEMENT;
        assignNode.assignStmt.lValue = left;
        assignNode.assignStmt.expression = right;
        return PushNode(ast, assignNode);
        
    }
    
    ExpectToken(parser, TOKEN_SEMICOLON);
    
    return PushNode(ast, node);
}

Index ParseIfStatement(AST *ast, Parser *parser)
{
    ExpectToken(parser, TOKEN_KEYWORD_IF);
    
    Index exprIndex = ParseExpression(ast, parser, 1);
    
    Node node = {0};
    node.type = NODE_IF_STATEMENT;
    node.ifStmt.conditionExpr = exprIndex;
    node.ifStmt.trueBlock = ParseStatementList(ast, parser);
    node.ifStmt.falseBlockExist = false;
    
    // looking for else block
    if(AcceptToken(parser, TOKEN_KEYWORD_ELSE))
    {
        if(AcceptToken(parser, TOKEN_KEYWORD_IF))
        {
            node.ifStmt.falseBlockExist = true;
            parser->tokenIndex -= 1;
            node.ifStmt.falseBlock = ParseIfStatement(ast, parser);
        }             
        else
        {            
            node.ifStmt.falseBlock = ParseStatementList(ast, parser);
            node.ifStmt.falseBlockExist = true;    
        }
    }
    
    return PushNode(ast, node);
}

Index ParseWhileStatement(AST *ast, Parser *parser)
{
    ExpectToken(parser, TOKEN_KEYWORD_WHILE);
    
    Node node = {0};
    node.type = NODE_WHILE_STATEMENT;
    node.whileStmt.conditionExpr = ParseExpression(ast, parser, 1);
        
    node.whileStmt.block = ParseStatementList(ast, parser);
    
    return PushNode(ast, node);
}

Index ParseReturnStatement(AST *ast, Parser *parser)
{
    ExpectToken(parser, TOKEN_KEYWORD_RETURN);
    
    Node node = {0};
    node.type = NODE_RETURN_STATEMENT;
    node.returnStmt.exprExist = false;
    
    Token next = PeekNextToken(parser);
    if(next.type == TOKEN_SEMICOLON)
    {
        GetNextToken(parser);
        return PushNode(ast, node);
    }
    
    node.returnStmt.exprExist = true;
    node.returnStmt.expression = ParseExpression(ast, parser, 1);
    
    ExpectToken(parser, TOKEN_SEMICOLON);
    
    return PushNode(ast, node);
}

Index ParseStatement(AST *ast, Parser *parser)
{
    Token token = PeekNextToken(parser);
    
    if(token.type == TOKEN_KEYWORD_LET)
    {
        return ParseVarDeclStatement(ast, parser);
    }
    else if(token.type == TOKEN_IDENTIFIER)
    {
        int startIndex = parser->tokenIndex;

        bool equalTokenFound = false;

        // peeking forward to see if the statement contains TOKEN_EQUAL
        while(true) 
        {
            Token token = GetNextToken(parser);

            if(token.type == TOKEN_EQUAL) {
                equalTokenFound = true;
            } else if(token.type == TOKEN_SEMICOLON) {
                break;
            } else if(token.type == TOKEN_PROGRAM_END) {
                break;
            }
        }

        parser->tokenIndex = startIndex;

        if(equalTokenFound) {            
            return ParseAssignmentStatement(ast, parser);
        } else {
            Index index = ParseExpression(ast, parser, 1);
            ExpectToken(parser, TOKEN_SEMICOLON);
            return index;
        }
    }
    else if(token.type == TOKEN_KEYWORD_IF)
    {
        return ParseIfStatement(ast, parser);
    }
    else if(token.type == TOKEN_KEYWORD_WHILE)
    {
        return ParseWhileStatement(ast, parser);
    }
    else if(token.type == TOKEN_KEYWORD_RETURN)
    {
        return ParseReturnStatement(ast, parser);
    }
    else
    {
        Index index = ParseExpression(ast, parser, 1);
        ExpectToken(parser, TOKEN_SEMICOLON);
        return index;
    }
}

Index ParseStatementList(AST *ast, Parser *parser) 
{
    ExpectToken(parser, TOKEN_LEFT_BRACE);
    
    Node node = {0};
    node.type = NODE_STATEMENT_LIST;
    
    while(true) 
    {
        Token token = PeekNextToken(parser);
        if(token.type == TOKEN_PROGRAM_END) break;
        else if(token.type == TOKEN_RIGHT_BRACE) break;
        else if(token.type == TOKEN_SEMICOLON) { GetNextToken(parser); continue;}
        
        Index statement = ParseStatement(ast, parser);
        PushIndex(&node.statementList.statements, &node.statementList.statementCount, statement);
    }
    
    ExpectToken(parser, TOKEN_RIGHT_BRACE);
    
    return PushNode(ast, node);
}

Index ParseFunction(AST *ast, Parser *parser)
{
    ExpectToken(parser, TOKEN_KEYWORD_FN);
    
    Token funcId = ExpectToken(parser, TOKEN_IDENTIFIER);
        
    Node node = {0};
    node.type = NODE_FUNCTION_DEFINITION;
    node.functionDef.name = funcId.identifier;
    node.functionDef.parameters = 0;
    node.functionDef.parameterCount = 0;
    node.functionDef.returnType = 0;
    node.functionDef.isReturnTypeDeclared = false;
    
    ExpectToken(parser, TOKEN_LEFT_PAREN);

    // parameters
    while(true)
    {
        Token token = PeekNextToken(parser);

        if(token.type == TOKEN_PROGRAM_END) break;
        else if(token.type == TOKEN_RIGHT_PAREN) break;
        
        if(node.functionDef.parameterCount > 0) ExpectToken(parser, TOKEN_COMMA);
        
        Token paramId = ExpectToken(parser, TOKEN_IDENTIFIER);
        
        Node idNode = {0};
        idNode.type = NODE_IDENTIFIER;
        idNode.identifier.value = paramId.identifier;
        
        ExpectToken(parser, TOKEN_COLON);
        
        Index typeAnnoIndex = ParseType(ast, parser);
        
        Node paramNode = {0};
        paramNode.type = NODE_PARAM;
        paramNode.param.id = PushNode(ast, idNode);
        paramNode.param.type = typeAnnoIndex;
        
        Index index = PushNode(ast, paramNode);
        PushIndex(&node.functionDef.parameters, &node.functionDef.parameterCount, index);
    }
    
    ExpectToken(parser, TOKEN_RIGHT_PAREN);
    
    // return type
    if(AcceptToken(parser, TOKEN_COLON))
    {
        node.functionDef.returnType = ParseType(ast, parser);
        node.functionDef.isReturnTypeDeclared = false;
    }
    
    // body    
    node.functionDef.body = ParseStatementList(ast, parser);
    
    return PushNode(ast, node);
}

Index ParseStruct(AST *ast, Parser *parser)
{
    ExpectToken(parser, TOKEN_KEYWORD_STRUCT);
    
    Token structId = ExpectToken(parser, TOKEN_IDENTIFIER);
    ExpectToken(parser, TOKEN_LEFT_BRACE);
    
    Node node = {0};
    node.type = NODE_STRUCT_DEFINITION;
    node.structDef.name = structId.identifier;
    node.structDef.fields = 0;
    node.structDef.fieldCount = 0;
    
    // parsing struct fields
    while(true)
    {
        Token token = PeekNextToken(parser);
        if(token.type == TOKEN_RIGHT_BRACE) break;
        else if(token.type == TOKEN_PROGRAM_END) break;
        
        Token fieldId = ExpectToken(parser, TOKEN_IDENTIFIER);
        
        Node idNode = {0};
        idNode.type = NODE_IDENTIFIER;
        idNode.identifier.value = fieldId.identifier;
        
        ExpectToken(parser, TOKEN_COLON);
        
        Index typeAnnoIndex = ParseType(ast, parser);
        
        ExpectToken(parser, TOKEN_SEMICOLON);
        
        Node fieldNode = {0};
        fieldNode.type = NODE_FIELD;
        fieldNode.field.id = PushNode(ast, idNode);
        fieldNode.field.type = typeAnnoIndex;
        
        Index fieldIndex = PushNode(ast, fieldNode);
        
        PushIndex(&node.structDef.fields, &node.structDef.fieldCount, fieldIndex);
    }
    
    ExpectToken(parser, TOKEN_RIGHT_BRACE);
    
    return PushNode(ast, node);
}

Index ParseProgram(AST *ast, Parser *parser)
{
    Node node = {0};
    node.type = NODE_PROGRAM;
    node.program.definitions = 0;
    node.program.defCount = 0;
    
    while(true)
    {
        Token token = PeekNextToken(parser);
        
        if(token.type == TOKEN_PROGRAM_END) break;
        
        if(token.type == TOKEN_KEYWORD_STRUCT)
        {
            Index index = ParseStruct(ast, parser);
            PushIndex(&node.program.definitions, &node.program.defCount, index);
        }
        else if(token.type == TOKEN_KEYWORD_FN)
        {
            Index index = ParseFunction(ast, parser);
            PushIndex(&node.program.definitions, &node.program.defCount, index);
        }
        else
        {
            GetNextToken(parser);
        }
    }
    
    return PushNode(ast, node);
}
