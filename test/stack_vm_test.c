#include "..\source\error.c"
#include "..\source\lexer.c"
#include "..\source\ast.c"
#include "..\source\parser.c"
#include "..\source\symbol.c"
#include "..\source\stack_vm.c"

#include <assert.h>

void Test_PushPop() {
    printf("\n======== %s =======\n", __func__);
    Instruction instructions[] = {
        INSTR(PUSH, 10),
        INSTR(PUSH, 20),
        INSTR(POP, 0),
        INSTR(HALT, 0),
    };
    int instrCount = sizeof(instructions) / sizeof(instructions[0]);

    StackVM vm = InitVM();
    execute(&vm, instructions, instrCount, (FunctionTable){0}, (TypeTable){0});

    assert(vm.stackPointer == 0);
    assert(vm.stack[vm.stackPointer] == 10);

    printf("%s: OK\n", __func__);
}

void Test_Arithmetic() {
    printf("\n======== %s =======\n", __func__);
    Instruction instructions[] = {
        INSTR(PUSH, 10),
        INSTR(PUSH, 5),
        INSTR(ADD, 0), // stack: [15]
        INSTR(PUSH, 3),
        INSTR(SUB, 0), // stack: [12]
        INSTR(PUSH, 2),
        INSTR(MUL, 0), // stack: [24]
        INSTR(PUSH, 4),
        INSTR(DIV, 0), // stack: [6]
        INSTR(PUSH, 4),
        INSTR(MOD, 0), // stack: [2]
        INSTR(HALT, 0),
    };
    int instrCount = sizeof(instructions) / sizeof(instructions[0]);

    StackVM vm = InitVM();
    execute(&vm, instructions, instrCount, (FunctionTable){0}, (TypeTable){0});

    assert(vm.stackPointer == 0);
    assert(vm.stack[vm.stackPointer] == 2);

    printf("%s: OK\n", __func__);
}

void Test_Jumps() {
    printf("\n======== %s =======\n", __func__);
    Instruction instructions[] = {
        INSTR(PUSH, 0),   // 0
        INSTR(JZ, 4),     // 1: jump to PUSH 10 if top is 0
        INSTR(PUSH, 99),  // 2: should be skipped
        INSTR(JMP, 5),    // 3: should be skipped
        INSTR(PUSH, 10),  // 4
        INSTR(PUSH, 1),   // 5
        INSTR(JNZ, 8),    // 6: jump to PUSH 20 if top is not 0
        INSTR(PUSH, 99),  // 7: should be skipped
        INSTR(PUSH, 20),  // 8
        INSTR(HALT, 0),   // 9
    };
    int instrCount = sizeof(instructions) / sizeof(instructions[0]);

    StackVM vm = InitVM();
    execute(&vm, instructions, instrCount, (FunctionTable){0}, (TypeTable){0});

    assert(vm.stackPointer == 1);
    assert(vm.stack[0] == 10);
    assert(vm.stack[1] == 20);

    printf("%s: OK\n", __func__);
}

void Test_FunctionCallAndStoreLoad() {
    printf("\n======== %s =======\n", __func__);

    // Simulates calling a function `fn add(a, b)` which returns `a+b`
    // and stores the result in a local variable.
    Instruction instructions[] = {
        // main function starts here
        INSTR(PUSH, 7),   // 0: push arg b
        INSTR(PUSH, 5),   // 1: push arg a
        INSTR(CALL, 5),   // 2: call add function
        INSTR(STORE, 0),  // 3: store result in local var `c`
        INSTR(HALT, 0),   // 4
        // add function starts here (address 5)
        INSTR(LOAD, 0),   // 5: load param a
        INSTR(LOAD, 1),   // 6: load param b
        INSTR(ADD, 0),    // 7
        INSTR(RET, 0),    // 8
    };
    int instrCount = sizeof(instructions) / sizeof(instructions[0]);

    Function funcs[] = {
        {.name = "main", .startAddress = 0, .paramsCount = 0, .localsCount = 1},
        {.name = "add", .startAddress = 5, .paramsCount = 2, .localsCount = 0},
    };
    FunctionTable ft = {.functions = funcs, .count = 2};

    StackVM vm = InitVM();
    // Manually set up for calling main
    vm.framePointer = 0;
    vm.frames[0] = (StackFrame){.start = 0, .returnAddr = instrCount};
    vm.stackPointer = funcs[0].localsCount -1; // for local var c

    execute(&vm, instructions, instrCount, ft, (TypeTable){0});

    // After HALT, the stack should contain the result of the add function (12)
    // stored in the first local variable slot of main.
    assert(vm.stack[0] == 12);

    printf("%s: OK\n", __func__);
}

void Test_Comparison() {
    printf("\n======== %s =======\n", __func__);
    Instruction instructions[] = {
        INSTR(PUSH, 5),
        INSTR(PUSH, 5),
        INSTR(EQ, 0),     // stack: [1]
        INSTR(PUSH, 5),
        INSTR(PUSH, 6),
        INSTR(NEQ, 0),    // stack: [1, 1]
        INSTR(PUSH, 5),
        INSTR(PUSH, 6),
        INSTR(LT, 0),     // stack: [1, 1, 1]
        INSTR(PUSH, 6),
        INSTR(PUSH, 5),
        INSTR(GT, 0),     // stack: [1, 1, 1, 1]
        INSTR(PUSH, 5),
        INSTR(PUSH, 5),
        INSTR(LE, 0),     // stack: [1, 1, 1, 1, 1]
        INSTR(PUSH, 6),
        INSTR(PUSH, 5),
        INSTR(GE, 0),     // stack: [1, 1, 1, 1, 1, 1]
        INSTR(HALT, 0),
    };
    int instrCount = sizeof(instructions) / sizeof(instructions[0]);

    StackVM vm = InitVM();
    execute(&vm, instructions, instrCount, (FunctionTable){0}, (TypeTable){0});

    assert(vm.stackPointer == 5);
    for(int i=0; i <= vm.stackPointer; i++) {
        assert(vm.stack[i] == 1); // All comparisons should be true
    }

    printf("%s: OK\n", __func__);
}

void Test_HeapOperations() {
    printf("\n======== %s =======\n", __func__);

    // Create a dummy type table for a struct Point {x: int, y: int}
    TypeTable tt = {0};
    PushType(&tt, (Type){.id = "int", .size = 1}); // index 0
    Type pointType = {.id = "Point", .size = 2, .isStruct = true};
    PushField(&pointType.fieldList, (StructField){.name="x", .typeTableIndex=0});
    PushField(&pointType.fieldList, (StructField){.name="y", .typeTableIndex=0});
    PushType(&tt, pointType); // index 1

    Instruction instructions[] = {
        INSTR(NEWSTRUCT, 1), // 0: new Point, pushes heap address (1) to stack
        INSTR(DUP, 0),       // 1: duplicate heap address for PUTFIELD
        INSTR(PUSH, 100),    // 2: push value for x
        INSTR(PUTFIELD, 0),  // 3: put 100 into field 0 (x). Consumes value and address.
        INSTR(DUP, 0),       // 4: duplicate heap address for PUTFIELD
        INSTR(PUSH, 200),    // 5: push value for y
        INSTR(PUTFIELD, 1),  // 6: put 200 into field 1 (y).
        INSTR(GETFIELD, 0),  // 7: get field 0 (x), leaves address on stack, pushes field value
        INSTR(HALT, 0),      // 8
    };
    int instrCount = sizeof(instructions) / sizeof(instructions[0]);

    StackVM vm = InitVM();
    execute(&vm, instructions, instrCount, (FunctionTable){0}, tt);

    assert(vm.heap[1] == 100); // heap pos 1 is x
    assert(vm.heap[2] == 200); // heap pos 2 is y
    assert(vm.stack[vm.stackPointer] == 100); // GETFIELD result
    assert(vm.stack[vm.stackPointer - 1] == 1); // Heap address should still be on stack

    // Cleanup
    free(tt.types[1].fieldList.symbols);
    free(tt.types);

    printf("%s: OK\n", __func__);
}

int main(int argc, char **argv)
{
    Test_PushPop();
    Test_Arithmetic();
    Test_Jumps();
    Test_FunctionCallAndStoreLoad();
    Test_Comparison();
    Test_HeapOperations();
    return 0;
}