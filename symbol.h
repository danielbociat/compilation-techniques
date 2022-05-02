#include <bits/stdc++.h>

#ifndef SYMBOL_H_INCLUDED
#define SYMBOL_H_INCLUDED



#endif // SYMBOL_H_INCLUDED

typedef struct _Type Type;
typedef struct _Symbol Symbol;
typedef struct _Symbols Symbols;

struct _Type{
    int typeBase;
    struct _Symbol *s;
    int nElements;
};

struct _Symbols{
    Symbol **begin;
    Symbol **end;
    Symbol **after;
};

struct _Symbol{
    const char *name;
    int cls;
    int mem;
    Type type;
    int depth;
    union{
        std::vector<Symbol*> args; // used only of functions
        std::vector<Symbol*> members; // used only for structs
    };

    _Symbol(){
    }
};

