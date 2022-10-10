#ifndef SYMBOL_H
#define SYMBOL_H

typedef struct {
    char *id;
    unsigned int size;
} Type;

typedef struct {
    Type *types;
    unsigned int count;
} TypeTable;

typedef struct {
    char *name;
    unsigned int type;
} Symbol;

typedef struct {
    Symbol *symbols;
    unsigned int count;
} SymbolTable;

void PushType(TypeTable *table, Type type);
void PushSymbol(SymbolTable *table, Symbol symbol);

#endif