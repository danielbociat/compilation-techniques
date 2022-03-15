#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define SAFEALLOC(var, Type) if((var=(Type*)malloc(sizeof(Type))==NULL)) err("not enough memory")

void err(const char *fmt, ...){
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}

enum {ID, END, CT_INT, ASSIGN, SEMICOLON};

typedef struct _Token{
    int code;
    union{
        char *text;
        long int i;
        double r;
    };
    int line;
    struct _Token *next;
}Token;

int line = 0;
Token *lastToken = NULL;
Token *tokens = NULL;

Token *addTk(int code){
    Token *tk;
    SAFEALLOC(tk, Token);

    tk->code = code;
    tk->line = line;
    tk->next = NULL;

    if(lastToken)
        lastToken->next = tk;
    else
        tokens = tk;

    lastToken = tk;
    return tk;

}

int main()
{
    printf("Hello world!\n");
    return 0;
}
