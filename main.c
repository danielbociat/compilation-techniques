#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

#define SAFEALLOC(var, Type) if((var = (Type *)malloc(sizeof(Type)))==NULL) err("not enough memory")

void err(const char *fmt, ...){
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}

enum {ID, END, CT_INT, ASSIGN, SEMICOLON, BREAK, EQUAL, CT_CHAR};


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

void tkerr(const Token *tk, const char *fmt, ...){
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error on line %d:", tk->line);
    fprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}

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

char *pCrtCh;

int getNextToken(){
    int state = 0, nCh;
    char ch;
    const char *pStartCh;
    Token *tk;

    char *ptr;
    int base;

    for(;;){
        ch = *pCrtCh; /// read the current character, ch = fread(ch,1)

        switch(state){
            case 0:{
                if(isalpha(ch) || ch == '_'){
                    pStartCh = pCrtCh; // memorizes the beginning of the ID
                    pCrtCh++; // consume the character
                    state = 1; // set the new state
                }
                else if(ch == '='){
                    pCrtCh++;
                    state = 3;
                }
                else if(ch == ' ' || ch == '\r' || ch == '\t'){
                    pCrtCh++; // consume the character and remains in state 0
                }
                else if(ch == '\n'){ // handled separately in order to update the current line
                    line++;
                    pCrtCh++;
                }
                else if(ch == 0){
                    addTk(END);
                    return END;
                }
                else if(ch == '/'){ // Start comment
                    pCrtCh++;
                    state = 11;
                }
                else if(isdigit(ch)){ /// INT or REAL
                    pStartCh = pCrtCh; /// set start
                    if(ch != '0'){ /// base 10
                        pCrtCh++; ///consume
                        state = 6;
                    }
                    else{ /// base 8 or 16
                        pCrtCh++;
                        state = 8;
                    }
                }
                else tkerr(addTk(END), "invalid character");
                break;
            }

            case 1:{
                if(isalnum(ch) || ch=='_')
                    pCrtCh++;
                else
                    state = 2;
                break;
            }

            case 2:{
                nCh = pCrtCh - pStartCh; /// the length

                if(nCh == 5 && !memcmp(pStartCh, "break", 5)){
                    tk=addTk(BREAK);
                }
                else if(nCh == 7 && !memcmp(pStartCh, "ct_char", 4)){
                    tk=addTk(CT_CHAR);
                }
                else{
                    tk = addTk(ID);
                    memcpy(tk->text, pStartCh, nCh);
                    //tk->text = createString(pStartCh, pCrtCh);
                }

                return tk->code;
            }

            case 3:{
                if(ch == '='){
                    pCrtCh++;
                    state=4;
                }
                else
                    state = 5;
                break;
            }

            case 4:{
                addTk(EQUAL);
                return EQUAL;
            }

            case 5:{
                addTk(ASSIGN);
                return ASSIGN;
            }

            case 6:{
                if(isdigit(ch)){
                    pCrtCh++; ///consume and don't change state
                }
                else{
                    base = 10;
                    state = 7;
                }

                break;
            }

            case 7:{ ///final state CT_INT

                tk = addTk(CT_INT);
                tk->i = strtol(pStartCh, &ptr, base);

                return tk->code;
            }

            case 8:{
                if(isdigit(ch) && ch < '8'){
                    pCrtCh++;
                    state = 9;
                }
                else if(ch == 'x'){
                    pCrtCh++;
                    state = 10;
                }
                else{ /// number is just "0"
                    base = 8;
                    state = 7;
                }
                break;
            }

            case 9:{
                if(isdigit(ch) && ch < '8'){
                    pCrtCh++;
                }
                else{
                    base = 8;
                    state = 7;
                }
                break;
            }
            case 10:{
                if(isdigit(ch) || (isalpha(ch) && (tolower(ch) >= 'a' && tolower(ch) <= 'f'))){
                    pCrtCh++;
                }
                else{
                    base = 16;
                    state = 7;
                }
                break;
            }

            case 11: {
                if(ch == '*'){
                    pCrtCh++;
                    state = 12;
                }
                /// TBD Else statement
                break;
            }

            case 12:{
                if(ch == '*'){
                    pCrtCh++;
                    state = 13;
                }
                else{
                    pCrtCh++; /// just consume
                }

                break;
            }

            case 13:{
                if(ch == '/'){
                    pCrtCh++;
                    state = 14;
                }
                else if(ch == '*'){
                    pCrtCh++;
                }
                else{
                    pCrtCh++;
                    state = 12;
                }
            }

            case 14:{ /// Final state for comment
                state = 0; /// comment fully ignored go to the beginning, TBD if need to be modified
            }
        }

    }

}

int main()
{
    return 0;
}
