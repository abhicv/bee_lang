#ifndef SYMBOL_H
#define SYMBOL_H

typedef struct {
    const char *name;
    unsigned int typeTableIndex;
    bool isArray;
    unsigned int arraySize;    
} Symbol, Parameter, StructField;

typedef struct {
    Symbol *symbols;
    unsigned int count;
} SymbolTable, ParameterList, StructFieldList;

typedef struct {
    const char *id;
    unsigned int size;

    bool isStruct;
    bool isFunction;

    int returnTypeIndex;

    union {
        ParameterList paramList;
        StructFieldList fieldList;
    };

} Type;

typedef struct {
    Type *types;
    unsigned int count;
} TypeTable;

void PushType(TypeTable *table, Type type);
void PushSymbol(SymbolTable *table, Symbol symbol);

int GetTypeTableIndexForId(TypeTable *typeTable, const char *id);
int GetSymbolTableIndexForId(SymbolTable *symbolTable, const char *symbolName);

void BuildTypeTable(AST *ast, Index rootIndex, TypeTable *globalTypeTable);

void PrintType(Type type);
void PrintTypeTable(TypeTable typeTable);

#endif