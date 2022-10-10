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
        printf("%s:%u:%u: error: expected '%s' but found '%s'\n", parser->fileName, token.line+1, token.column+1, TokenTypeToString(tokenType), TokenTypeToString(token.type));
        exit(1);
    }
}

enum OperatorAssociativity {
    LEFT_ASSOCIATIVE = 1,
    RIGHT_ASSOCIATIVE,
};

typedef struct OpInfo OpInfo;
struct OpInfo {
    unsigned int precedence;
    unsigned int associatvity;
};

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
        node.opType = next.opType;
        node.left = left;

        if(opInfoTable[next.opType].associatvity == LEFT_ASSOCIATIVE)
        {
            node.right = ParseExpression(ast, parser, prec + 1);
        }
        else
        {
            node.right = ParseExpression(ast, parser, prec);
        }

        left = PushNode(ast, node);
    }

    return left;
}

Index ParseAtom(AST *ast, Parser *parser)
{
    if(AcceptToken(parser, TOKEN_IDENTIFIER))
    {
        // function call
        if(AcceptToken(parser, TOKEN_OPEN_BRACKET))
        {
            Node funcCall = {0};
            funcCall.type = NODE_FUNC_CALL;
            funcCall.funcId = parser->tokenList.tokens[parser->tokenIndex - 2].identifier;

            // function arguments
            while(true)
            {
                Token token = PeekNextToken(parser);

                if(token.type == TOKEN_CLOSE_BRACKET) break;
                else if(token.type == TOKEN_PROGRAM_END) break;

                if(funcCall.argumentsListCount > 0)
                {
                    ExpectToken(parser, TOKEN_COMMA);
                }

                Index argIndex = ParseExpression(ast, parser, 1);
                PushIndex(&funcCall.argumentsIndexList, &funcCall.argumentsListCount, argIndex);
            }

            ExpectToken(parser, TOKEN_CLOSE_BRACKET);

            return PushNode(ast, funcCall);
        }
        else
        {
            Node node = {0};
            node.type = NODE_IDENTIFIER;
            node.identifier = parser->tokenList.tokens[parser->tokenIndex - 1].identifier;
            return PushNode(ast, node);
        }
    }
    else if(AcceptToken(parser, TOKEN_NOT))
    {
        Node node = {0};
        node.type = NODE_OPERATOR;
        node.opType = BOOL_OP_NOT;
        node.left = ParseAtom(ast, parser);
        return PushNode(ast, node);
    }
    else if(AcceptToken(parser, TOKEN_INTEGER_CONSTANT))
    {
        Node node = {0};
        node.type = NODE_INTEGER_CONSTANT;
        node.value = parser->tokenList.tokens[parser->tokenIndex - 1].integerValue;
        return PushNode(ast, node);
    }
    else if(AcceptToken(parser, TOKEN_STRING_CONSTANT))
    {
        Node node = {0};
        node.type = NODE_STRING_CONSTANT;
        node.stringValue = parser->tokenList.tokens[parser->tokenIndex - 1].stringValue;
        return PushNode(ast, node);
    }
    else if(AcceptToken(parser, TOKEN_OPEN_BRACKET))
    {
        Index index = ParseExpression(ast, parser, 1);
        ExpectToken(parser, TOKEN_CLOSE_BRACKET);
        return index;
    }

    // an atom is always required
    Token next = PeekNextToken(parser);
    printf("%s:%u:%u: error: expecting an expression before '%s'\n", parser->fileName, next.line+1, next.column+1, TokenTypeToString(next.type));
    exit(1);
}

Index ParseSructFieldAccess(AST *ast, Parser *parser) 
{
    Node fieldAccessNode = {0};
    fieldAccessNode.type = NODE_FIELD_ACCESS;

    while(true)
    {
        Token first = GetNextToken(parser);
        Token second = GetNextToken(parser);

        if(first.type == TOKEN_IDENTIFIER && second.type == TOKEN_DOT)
        {
            Node id = {0};
            id.type = NODE_IDENTIFIER;
            id.identifier = first.identifier;

            Index index = PushNode(ast, id);
            PushIndex(&fieldAccessNode.fieldIndexList, &fieldAccessNode.fieldIndexCount, index);
        } 
        else 
        {
            parser->tokenIndex -= 2;
            break;
        }
    }

    Token token = ExpectToken(parser, TOKEN_IDENTIFIER);

    Node id = {0};
    id.type = NODE_IDENTIFIER;
    id.identifier = token.identifier;

    Index index = PushNode(ast, id);

    PushIndex(&fieldAccessNode.fieldIndexList, &fieldAccessNode.fieldIndexCount, index);

    return PushNode(ast, fieldAccessNode);
}

Index ParseStatement(AST *ast, Parser *parser);
Index ParseStatementList(AST *ast, Parser *parser);

// TODO: parse multiple l values
Index ParseLValue(AST *ast, Parser *parser)
{
    Token id = ExpectToken(parser, TOKEN_IDENTIFIER);
    Node node = {0};
    node.type = NODE_IDENTIFIER;
    node.identifier = id.identifier;
    return PushNode(ast, node);
}

Index ParseAssignmentStatement(AST *ast, Parser *parser)
{
    Index lvalueIndex = ParseLValue(ast, parser);
    ExpectToken(parser, TOKEN_EQUAL);
    Index exprIndex = ParseExpression(ast, parser, 1);

    ExpectToken(parser, TOKEN_SEMICOLON);

    Node assignNode = {0};
    assignNode.type = NODE_ASSIGN_STATEMENT;
    assignNode.left = lvalueIndex;
    assignNode.right = exprIndex;

    return PushNode(ast, assignNode);
}

Index ParseVarDeclStatement(AST *ast, Parser *parser)
{
    ExpectToken(parser, TOKEN_KEYWORD_LET);

    Token id = ExpectToken(parser, TOKEN_IDENTIFIER);

    ExpectToken(parser, TOKEN_COLON);

    Token typeID = ExpectToken(parser, TOKEN_IDENTIFIER);

    ExpectToken(parser, TOKEN_SEMICOLON);

    Node node = {0};
    node.type = NODE_VAR_DECL;
    node.identifier = id.identifier;
    node.typeId = typeID.identifier;
    node.isTypeDeclared = true;

    return PushNode(ast, node);
}

Index ParseIfStatement(AST *ast, Parser *parser)
{
    ExpectToken(parser, TOKEN_KEYWORD_IF);
    ExpectToken(parser, TOKEN_OPEN_BRACKET);

    Index exprIndex = ParseExpression(ast, parser, 1);

    ExpectToken(parser, TOKEN_CLOSE_BRACKET);

    Node ifNode = {0};
    ifNode.type = NODE_IF_STATEMENT;
    ifNode.conditionExpr = exprIndex;
    ifNode.trueBlock = ParseStatementList(ast, parser);
    ifNode.falseBlockExist = false;

    // looking for else block
    if(AcceptToken(parser, TOKEN_KEYWORD_ELSE))
    {
        if(AcceptToken(parser, TOKEN_KEYWORD_IF))
        {
            ifNode.falseBlockExist = true;    
            parser->tokenIndex -= 1;
            ifNode.falseBlock = ParseIfStatement(ast, parser);
        }             
        else
        {            
            ifNode.falseBlock = ParseStatementList(ast, parser);    
            ifNode.falseBlockExist = true;    
        }
    }

    return PushNode(ast, ifNode);
}

Index ParseWhileStatement(AST *ast, Parser *parser)
{
    ExpectToken(parser, TOKEN_KEYWORD_WHILE);
    ExpectToken(parser, TOKEN_OPEN_BRACKET);

    Node node = {0};
    node.type = NODE_WHILE_STATEMENT;
    node.conditionExpr = ParseExpression(ast, parser, 1);

    ExpectToken(parser, TOKEN_CLOSE_BRACKET);

    node.trueBlock = ParseStatementList(ast, parser);

    return PushNode(ast, node);
}

Index ParseReturnStatement(AST *ast, Parser *parser)
{
    ExpectToken(parser, TOKEN_KEYWORD_RETURN);

    Node node = {0};
    node.type = NODE_RETURN_STATEMENT;
    node.returnExprExist = false;

    Token next = PeekNextToken(parser);
    if(next.type == TOKEN_SEMICOLON)
    {
        GetNextToken(parser);
        return PushNode(ast, node);
    }

    node.returnExprExist = true;
    node.returnExpr = ParseExpression(ast, parser, 1);
    
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
        GetNextToken(parser);
        Token next = PeekNextToken(parser);

        parser->tokenIndex -= 1;
        
        if(next.type == TOKEN_EQUAL) {            
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
    ExpectToken(parser, TOKEN_OPEN_CURLY_BRACKET);
    
    Node stmtList = {0};
    stmtList.type = NODE_STATEMENT_LIST;

    while(true) 
    {
        Token token = PeekNextToken(parser);
        if(token.type == TOKEN_PROGRAM_END) break;
        else if(token.type == TOKEN_CLOSE_CURLY_BRACKET) break;
        else if(token.type == TOKEN_SEMICOLON) { GetNextToken(parser); continue;}

        Index statement = ParseStatement(ast, parser);
        PushIndex(&stmtList.statementIndexList, &stmtList.statementIndexCount, statement);
    }
    
    ExpectToken(parser, TOKEN_CLOSE_CURLY_BRACKET);

    return PushNode(ast, stmtList);
}

Index ParseFunction(AST *ast, Parser *parser)
{
    ExpectToken(parser, TOKEN_KEYWORD_FN);

    Token funcId = ExpectToken(parser, TOKEN_IDENTIFIER);
    
    ExpectToken(parser, TOKEN_OPEN_BRACKET);

    Node node = {0};
    node.type = NODE_FUNC_DEF;
    node.funcName = funcId.identifier;
    node.parameterIndexList = 0;
    node.parameterIndexCount = 0;
        
    //parse parameters
    while(true)
    {
        Token token = PeekNextToken(parser);
        if(token.type == TOKEN_PROGRAM_END) break;
        else if(token.type == TOKEN_CLOSE_BRACKET) break;
        
        if(node.parameterIndexCount > 0)
        {
            ExpectToken(parser, TOKEN_COMMA);
        }
        
        Token paramId = ExpectToken(parser, TOKEN_IDENTIFIER);
        ExpectToken(parser, TOKEN_COLON);
        Token paramType = ExpectToken(parser, TOKEN_IDENTIFIER);

        Node field = {0};
        field.type = NODE_FIELD;
        field.identifier = paramId.identifier;
        field.typeId = paramType.identifier;
        
        Index index = PushNode(ast, field);
        PushIndex(&node.parameterIndexList, &node.parameterIndexCount, index);
    }
    
    ExpectToken(parser, TOKEN_CLOSE_BRACKET);
    
    //parse return types
    if(AcceptToken(parser, TOKEN_COLON))
    {
        while(true)
        {
            Token token = PeekNextToken(parser);
            if(token.type == TOKEN_PROGRAM_END) break;
            else if(token.type == TOKEN_OPEN_CURLY_BRACKET) break;
            
            if(node.returnIndexCount > 0)
            {
                ExpectToken(parser, TOKEN_COMMA);
            }
            
            Token returnId = ExpectToken(parser, TOKEN_IDENTIFIER);
            
            Node id = {0};
            id.identifier = returnId.identifier;
            id.type = NODE_IDENTIFIER;
            
            Index index = PushNode(ast, id);
            PushIndex(&node.returnIndexList, &node.returnIndexCount, index);
        }
    }

    // function body    
    node.funcBody = ParseStatementList(ast, parser);

    return PushNode(ast, node);
}

Index ParseStruct(AST *ast, Parser *parser)
{
    ExpectToken(parser, TOKEN_KEYWORD_STRUCT);

    Token structId = ExpectToken(parser, TOKEN_IDENTIFIER);

    ExpectToken(parser, TOKEN_OPEN_CURLY_BRACKET);
    
    Node node = {0};
    node.type = NODE_STRUCT_DEF;
    node.structName = structId.identifier;
    node.fieldIndexCount = 0;
    node.fieldIndexList = 0;
    
    // parsing struct fields
    while(true)
    {
        Token token = PeekNextToken(parser);
        if(token.type == TOKEN_CLOSE_CURLY_BRACKET) break;
        else if(token.type == TOKEN_PROGRAM_END) break;
        
        Token fieldId = ExpectToken(parser, TOKEN_IDENTIFIER);
        ExpectToken(parser, TOKEN_COLON);
        Token typeID = ExpectToken(parser, TOKEN_IDENTIFIER);
        ExpectToken(parser, TOKEN_SEMICOLON);
        
        Node field = {0};
        field.type = NODE_FIELD;
        field.identifier = fieldId.identifier;
        field.typeId = typeID.identifier;
        
        Index fieldIndex = PushNode(ast, field);
        
        PushIndex(&node.fieldIndexList, &node.fieldIndexCount, fieldIndex);
    }
    
    ExpectToken(parser, TOKEN_CLOSE_CURLY_BRACKET);
    
    return PushNode(ast, node);
}

Index ParseProgram(AST *ast, Parser *parser)
{
    Node node = {0};
    node.type = NODE_PROGRAM;
    node.indexList = 0;
    node.indexCount = 0;
    
    while(true)
    {
        Token token = PeekNextToken(parser);

        if(token.type == TOKEN_PROGRAM_END) break;

        if(token.type == TOKEN_KEYWORD_STRUCT)
        {
            Index index = ParseStruct(ast, parser);
            PushIndex(&node.indexList, &node.indexCount, index);
        }
        else if(token.type == TOKEN_KEYWORD_FN)
        {
            Index index = ParseFunction(ast, parser);
            PushIndex(&node.indexList, &node.indexCount, index);
        }
        else
        {
            GetNextToken(parser);
        }
    }
    
    return PushNode(ast, node);
}
