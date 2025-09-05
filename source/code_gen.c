#include "ast.h"
#include "symbol.h"
#include "stack_vm.c"
#include "assert.h"

#define MAX_INSTR_COUNT 2000
static Instruction instructions[MAX_INSTR_COUNT] = {0};
static int instrCount = 0;

void AddInstr(Instruction instruction) 
{
    assert(instrCount < MAX_INSTR_COUNT);
    instructions[instrCount++] = instruction;
}

int GetVarIndexForName(TypeTable typeTable, int functionTypeTableIndex, char *name) 
{
    Type functionType = typeTable.types[functionTypeTableIndex];

    int index = -1;

    index = GetSymbolTableIndexForId(&functionType.paramList, name);

    if (index != -1) return index;

    index = GetSymbolTableIndexForId(&functionType.localSymbolList, name);

    if (index != -1) return (functionType.paramList.count + index);

    return index;
}

int GenerateCode(AST ast, Index index, TypeTable typeTable, int currentFunctionTypeTableIndex, bool isStore) 
{
    Node node = ast.nodeList[index];

    switch(node.type) {

        case NODE_PROGRAM: {
            for(int n = 0; n < node.program.defCount; n++) {
                GenerateCode(ast, node.program.definitions[n], typeTable, currentFunctionTypeTableIndex, isStore);
            }
        }
        break;

        case NODE_FUNCTION_DEFINITION: {
            int functionTypeTableIndex = GetTypeTableIndexForFunctionId(&typeTable, node.functionDef.name);
            GenerateCode(ast, node.functionDef.body, typeTable, functionTypeTableIndex, isStore);
        }
        break;

        case NODE_STATEMENT_LIST: {
            for(int n = 0; n < node.statementList.statementCount; n++) {
                GenerateCode(ast, node.statementList.statements[n], typeTable, currentFunctionTypeTableIndex, isStore);
            }
        }
        break;

        case NODE_L_VALUE: {
            return GenerateCode(ast, node.lValue.simpleLValues[0], typeTable, currentFunctionTypeTableIndex, isStore);
        }
        break;

        case NODE_ASSIGN_STATEMENT: {
            GenerateCode(ast, node.assignStmt.expression, typeTable, currentFunctionTypeTableIndex, false);            
            int varIndex = GenerateCode(ast, node.assignStmt.lValue, typeTable, currentFunctionTypeTableIndex, true);            
            AddInstr(INSTR(STORE, varIndex));
        }
        break;

        case NODE_VARIABLE_DECLARATION: {
            int varIndex = GenerateCode(ast, node.varDecl.id, typeTable, currentFunctionTypeTableIndex, true);            
            assert(varIndex != -1);
            return varIndex;
        }
        break;

        case NODE_IF_STATEMENT: {

            GenerateCode(ast, node.ifStmt.conditionExpr, typeTable, currentFunctionTypeTableIndex, false);

            AddInstr(INSTR(JZ, -1));

            Instruction *condJmpInstr = &instructions[instrCount - 1];

            GenerateCode(ast, node.ifStmt.trueBlock, typeTable, currentFunctionTypeTableIndex, false);

            AddInstr(INSTR(JMP, -1));

            Instruction *unCondJmpInstr = &instructions[instrCount - 1];

            condJmpInstr->operand = instrCount;

            if (node.ifStmt.falseBlockExist) {
                GenerateCode(ast, node.ifStmt.falseBlock, typeTable, currentFunctionTypeTableIndex, false);            
            }

            unCondJmpInstr->operand = instrCount;
        }
        break;

        case NODE_WHILE_STATEMENT: {

            int unCondJumpAddress = instrCount;

            GenerateCode(ast, node.whileStmt.conditionExpr, typeTable, currentFunctionTypeTableIndex, false);
            
            AddInstr(INSTR(JZ, -1));

            Instruction *condJmpInstr = &instructions[instrCount - 1];

            GenerateCode(ast, node.whileStmt.block, typeTable, currentFunctionTypeTableIndex, false);

            AddInstr(INSTR(JMP, unCondJumpAddress));

            condJmpInstr->operand = instrCount;
        }
        break;

        case NODE_IDENTIFIER: {
            int varIndex = GetVarIndexForName(typeTable, currentFunctionTypeTableIndex, (char*)node.identifier.value);
            assert(varIndex != -1);
            if (isStore) {
                return varIndex;
            }
            AddInstr(INSTR(LOAD, varIndex));
        }
        break;

        case NODE_OPERATOR: {
            GenerateCode(ast, node.operator.left, typeTable, currentFunctionTypeTableIndex, false);            
            GenerateCode(ast, node.operator.right, typeTable, currentFunctionTypeTableIndex, false);
            
            switch (node.operator.opType)
            {
            case ARITHMETIC_OP_ADD:
                AddInstr(INSTR(ADD, 0));
                break;

            case ARITHMETIC_OP_SUB:
                AddInstr(INSTR(SUB, 0));
                break;

            case ARITHMETIC_OP_MUL:
                AddInstr(INSTR(MUL, 0));
                break;

            case ARITHMETIC_OP_DIV:
                AddInstr(INSTR(DIV, 0));
                break;

            case COMPARE_OP_LT:
                AddInstr(INSTR(LT, 0));
                break;

            case COMPARE_OP_GT:
                AddInstr(INSTR(GT, 0));
                break;

            case COMPARE_OP_LT_EQ:
                AddInstr(INSTR(LE, 0));
                break;

            case COMPARE_OP_GT_EQ:
                AddInstr(INSTR(GE, 0));
                break;

            case COMPARE_OP_EQ_EQ:
                AddInstr(INSTR(EQ, 0));
                break;

            case COMPARE_OP_NOT_EQ:
                AddInstr(INSTR(NEQ, 0));
                break;
                
            default:
                printf("Unsupported operator for code generation\n");
                break;
            }
        }
        break;

        case NODE_INTEGER_CONSTANT: {
            AddInstr(INSTR(PUSH, node.integer.value));
        }
        break;
        
        case NODE_BOOLEAN_CONSTANT: {
            if (node.boolean.isTrue) {
                AddInstr(INSTR(PUSH, 1));
            } else {
                AddInstr(INSTR(PUSH, 0));
            }
        }
        break;

        case NODE_RETURN_STATEMENT: {
            GenerateCode(ast, node.returnStmt.expression, typeTable, currentFunctionTypeTableIndex, false);            
            AddInstr(INSTR(RET, 0));
        }
        break;
    }

    return -1;
}

