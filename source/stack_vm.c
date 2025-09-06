#include "stdio.h"
#include "assert.h"
#include "stdbool.h"

#define INSTR(_type, _operand) (Instruction){.type = _type, .operand = _operand}

enum InstructionType {
    NOP = 0,

    PUSH,
    POP,
    STORE,
    LOAD,    

    ADD,
    SUB,
    MUL,
    DIV,
    MOD,

    EQ,
    LT,
    GT,
    GE,
    LE,
    NEQ,

    JMP,
    JZ,
    JNZ,

    CALL,
    RET,

    PRINT,

    HALT
};

typedef struct {
    enum InstructionType type;
    int operand;
    char *label;
} Instruction;

typedef struct {
    int start;
    int returnAddr;
} StackFrame;

typedef struct {
    char *name;
    int startAddress;
    int paramsCount;
    int localsCount;
} Function;

typedef struct {
    Function *functions;
    int count;
} FunctionTable;

typedef struct {

    int instrPointer;    

    int stack[1024];
    int stackPointer;

    int heap[1024];
    int heapPointer;

    StackFrame frames[100];
    int framePointer;

} StackVM;

int GetFunctionByAddress(FunctionTable table, int address) {
    for(int n = 0; n < table.count; n++) {
        if(address == table.functions[n].startAddress) {
            return n;
        }
    }
    return -1;
}

int GetFunctionByName(FunctionTable table, char *name) {
    for(int n = 0; n < table.count; n++) {
        if(!strcmp(table.functions[n].name, name)) {
            return n;
        }
    }
    return -1;
}

void execute(StackVM vm, Instruction *instructions, int instrCount, FunctionTable functionTable) 
{
    bool stop = false;

    while(!stop) {

        if (vm.instrPointer == instrCount) break;

        // printf("IP: %d\n", vm.instrPointer);

        Instruction instruction = instructions[vm.instrPointer];
        
        switch (instruction.type)
        {

        case PUSH:
        {
            vm.stackPointer++;
            vm.stack[vm.stackPointer] = instruction.operand;
            vm.instrPointer++;
        }
        break;

        case POP:
        {
            vm.stackPointer--;
            vm.instrPointer++;
        }
        break;

        case LOAD:
        {
            assert(instruction.operand > -1);
            assert(vm.framePointer > -1);
            StackFrame frame = vm.frames[vm.framePointer];
            vm.stackPointer++;
            vm.stack[vm.stackPointer] = vm.stack[frame.start + instruction.operand];
            vm.instrPointer++;
        }
        break;

        case STORE:
        {
            assert(vm.framePointer > -1);
            StackFrame frame = vm.frames[vm.framePointer];
            vm.stack[frame.start + instruction.operand]  = vm.stack[vm.stackPointer];
            vm.stackPointer--;
            vm.instrPointer++;
        }
        break;

        case ADD:
        {
            assert(vm.stackPointer > 0);
            int result = vm.stack[vm.stackPointer - 1] + vm.stack[vm.stackPointer];
            vm.stackPointer -= 2;
            vm.stack[++vm.stackPointer] = result;    
            vm.instrPointer++;
        }
        break;

        case SUB: 
        {
            assert(vm.stackPointer > 0);
            int result = vm.stack[vm.stackPointer - 1] - vm.stack[vm.stackPointer];
            vm.stackPointer -= 2;
            vm.stack[++vm.stackPointer] = result;            
            vm.instrPointer++;
        }
        break;

        case MUL: 
        {
            assert(vm.stackPointer > 0);
            int result = vm.stack[vm.stackPointer - 1] * vm.stack[vm.stackPointer];
            vm.stackPointer -= 2;
            vm.stack[++vm.stackPointer] = result;            
            vm.instrPointer++;
        }
        break;

        case DIV: 
        {
            assert(vm.stackPointer > 0);
            int result = vm.stack[vm.stackPointer - 1] / vm.stack[vm.stackPointer];
            vm.stackPointer -= 2;
            vm.stack[++vm.stackPointer] = result;            
            vm.instrPointer++;
        }
        break;

        case MOD: 
        {
            assert(vm.stackPointer > 0);
            int result = vm.stack[vm.stackPointer - 1] % vm.stack[vm.stackPointer];
            vm.stackPointer -= 2;
            vm.stack[++vm.stackPointer] = result;            
            vm.instrPointer++;
        }
        break;

        case EQ: 
        {
            assert(vm.stackPointer > 0);
            int result = vm.stack[vm.stackPointer - 1] == vm.stack[vm.stackPointer];
            vm.stackPointer -= 2;
            vm.stack[++vm.stackPointer] = result;            
            vm.instrPointer++;
        }
        break;

        case NEQ: 
        {
            assert(vm.stackPointer > 0);
            int result = vm.stack[vm.stackPointer - 1] != vm.stack[vm.stackPointer];
            vm.stackPointer -= 2;
            vm.stack[++vm.stackPointer] = result;            
            vm.instrPointer++;
        }
        break;

        case LT: 
        {
            assert(vm.stackPointer > 0);
            int result = vm.stack[vm.stackPointer - 1] < vm.stack[vm.stackPointer];
            vm.stackPointer -= 2;
            vm.stack[++vm.stackPointer] = result;            
            vm.instrPointer++;
        }
        break;

        case GT: 
        {
            assert(vm.stackPointer > 0);
            int result = vm.stack[vm.stackPointer - 1] > vm.stack[vm.stackPointer];
            vm.stackPointer -= 2;
            vm.stack[++vm.stackPointer] = result;            
            vm.instrPointer++;
        }
        break;

        case LE: 
        {
            assert(vm.stackPointer > 0);
            int result = vm.stack[vm.stackPointer - 1] <= vm.stack[vm.stackPointer];
            vm.stackPointer -= 2;
            vm.stack[++vm.stackPointer] = result;            
            vm.instrPointer++;
        }
        break;

        case GE: 
        {
            assert(vm.stackPointer > 0);
            int result = vm.stack[vm.stackPointer - 1] >= vm.stack[vm.stackPointer];
            vm.stackPointer -= 2;
            vm.stack[++vm.stackPointer] = result;            
            vm.instrPointer++;
        }
        break;

        case JMP:
        {
            assert(instruction.operand < instrCount);
            assert(instruction.operand > -1);
            vm.instrPointer = instruction.operand;
        }
        break;

        case JZ:
        {
            assert(instruction.operand > -1);
            int value = vm.stack[vm.stackPointer--];
            if (value == 0) {
                assert(instruction.operand < instrCount);
                assert(instruction.operand > -1);
                vm.instrPointer = instruction.operand;
            } else {
                vm.instrPointer++;
            }
        }
        break;

        case JNZ:
        {
            assert(instruction.operand > -1);
            int value = vm.stack[vm.stackPointer--];
            if (value != 0) {
                assert(instruction.operand < instrCount);
                assert(instruction.operand > -1);
                vm.instrPointer = instruction.operand;
            } else {
                vm.instrPointer++;
            }
        }
        break;

        case CALL: 
        {
            assert(instruction.operand > -1);

            int index = GetFunctionByAddress(functionTable, instruction.operand);

            if(index != -1) {

                Function function = functionTable.functions[index]; 
                int paramsCount = function.paramsCount;                
                
                StackFrame frame = {0};
                frame.returnAddr = vm.instrPointer + 1;
                frame.start = vm.stackPointer - paramsCount + 1;
                
                int localsCount = function.localsCount;
                vm.stackPointer += localsCount;

                vm.framePointer++;
                assert(vm.framePointer < 100);
                vm.frames[vm.framePointer] = frame;

                vm.instrPointer = instruction.operand;

            } else {
                printf("unable to call function %d at IP: %d\n", instruction.operand, vm.instrPointer);
                vm.instrPointer++;
            }
        }
        break;

        case RET:
        {
            assert(vm.framePointer > -1);
            StackFrame frame = vm.frames[vm.framePointer];
            vm.stack[frame.start] = vm.stack[vm.stackPointer];
            vm.stackPointer = frame.start;
            vm.instrPointer = frame.returnAddr;
            assert(frame.returnAddr > -1);
            vm.framePointer--;
        }
        break;

        case PRINT:
        {
            assert(vm.stackPointer > -1);
            printf("%d\n", vm.stack[vm.stackPointer]);
            vm.instrPointer++;
        }
        break;

        case HALT:
        {
            stop = true;
        }
        break;

        case NOP:
        {
            vm.instrPointer++;
        }
        break;
        
        default:
        {
            printf("Unknown instruction type %d\n", instruction.type);
            stop = true;
        }
        break;  

        }
    
    }
}

StackVM InitVM() {
    StackVM vm = {0};
    vm.instrPointer = 0;
    vm.stackPointer = -1;
    vm.heapPointer = -1;
    vm.framePointer = -1;
    return vm;
}

void PrintInstruction(FILE *file, Instruction *instructions, int count, bool showLineNumbers) {
    for (int n = 0; n < count; n++) {
        Instruction instruction = instructions[n];

        if (showLineNumbers) {
            fprintf(file, "%03d: ", n);
        }

        switch (instruction.type) {
        case NOP:   fprintf(file, "NOP\n"); break;

        case PUSH:  fprintf(file, "PUSH %d\n", instruction.operand); break;
        case POP:   fprintf(file, "POP\n"); break;
        case LOAD:  fprintf(file, "LOAD %d\n", instruction.operand); break;
        case STORE: fprintf(file, "STORE %d\n", instruction.operand); break;

        case ADD:   fprintf(file, "ADD\n"); break;
        case SUB:   fprintf(file, "SUB\n"); break;
        case MUL:   fprintf(file, "MUL\n"); break;
        case DIV:   fprintf(file, "DIV\n"); break;
        case MOD:   fprintf(file, "MOD\n"); break;

        case EQ:    fprintf(file, "EQ\n"); break;
        case LT:    fprintf(file, "LT\n"); break;
        case GT:    fprintf(file, "GT\n"); break;
        case GE:    fprintf(file, "GE\n"); break;
        case LE:    fprintf(file, "LE\n"); break;
        case NEQ:    fprintf(file, "NEQ\n"); break;

        case JMP:   fprintf(file, "JMP %d\n", instruction.operand); break;
        case JZ:    fprintf(file, "JZ %d\n", instruction.operand); break;
        case JNZ:   fprintf(file, "JNZ %d\n", instruction.operand); break;

        case CALL:   fprintf(file, "CALL %d\n", instruction.operand); break;
        case RET:   fprintf(file, "RET\n"); break;

        case PRINT: fprintf(file, "PRINT\n"); break;
        case HALT:  fprintf(file, "HALT\n"); break;

        default:    fprintf(file, "UNKNOWN %d %d\n", instruction.type, instruction.operand); break;
        }
    }
}