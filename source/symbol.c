#include "symbol.h"
#include "stdlib.h"

void PushType(TypeTable *table, Type type) 
{
    if(table->count == 0) 
    {
        table->count++;
        table->types = (Type*)malloc(sizeof(Type));
    }
    else 
    {
        table->count++;
        table->types = (Type*)realloc(table->types, sizeof(Type) * table->count);
    }

    table->types[table->count - 1] = type;
}

void PushSymbol(SymbolTable *table, Symbol symbol) 
{
    if(table->count == 0) 
    {
        table->count++;
        table->symbols = (Symbol*)malloc(sizeof(Type));
    }
    else 
    {
        table->count++;
        table->symbols = (Symbol*)realloc(table->symbols, sizeof(Type) * table->count);
    }

    table->symbols[table->count - 1] = symbol;
}