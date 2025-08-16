#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdbool.h>

typedef struct {
    const char *name;
    unsigned int typeTableIndex;
    bool isArray;
    unsigned int arraySize;
} Symbol, Parameter, StructField;

typedef struct {
    Symbol *symbols;
    unsigned int count;
} SymbolTable, ParameterList, StructFieldList, LocalSymbolList;

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

    LocalSymbolList localSymbolList;

    Index astIndex;

} Type;

typedef struct {
    Type *types;
    unsigned int count;
} TypeTable;

void PushType(TypeTable *table, Type type);
void PushSymbol(SymbolTable *table, Symbol symbol);

int GetTypeTableIndexForId(TypeTable *typeTable, const char *id);
int GetTypeTableIndexForFunctionId(TypeTable *typeTable, const char *id) ;

int GetSymbolTableIndexForId(SymbolTable *symbolTable, const char *symbolName);

bool BuildTypeTable(AST *ast, Index rootIndex, TypeTable *globalTypeTable);

void PrintType(Type type);
void PrintTypeTable(TypeTable typeTable);

#endif