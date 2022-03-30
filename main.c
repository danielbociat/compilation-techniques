#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

#define SAFEALLOC(var, Type) if((var = (Type *)malloc(sizeof(Type)))==NULL) err("not enough memory")

#define BLOCK_SIZE 512

void err(const char *fmt, ...){
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}

const char* codes[] = {"END", "ID",
    "BREAK", "CHAR", "DOUBLE", "ELSE", "FOR", "IF", "INT", "RETURN", "STRUCT", "VOID", "WHILE",
    "CT_INT", "CT_REAL", "CT_CHAR", "CT_STRING",
    "COMMA", "SEMICOLON", "LPAR", "RPAR", "LBRACKET", "RBRACKET", "LACC", "RACC",
    "ADD", "SUB", "MUL", "DIV", "DOT", "AND", "OR", "NOT", "ASSIGN", "EQUAL", "NOTEQ", "LESS", "LESSEQ", "GREATER", "GREATEREQ",
    "SPACE", "LINECOMMENT", "COMMENT"
};
enum {END, ID,
    BREAK, CHAR, DOUBLE, ELSE, FOR, IF, INT, RETURN, STRUCT, VOID, WHILE,
    CT_INT, CT_REAL, CT_CHAR, CT_STRING,
    COMMA, SEMICOLON, LPAR, RPAR, LBRACKET, RBRACKET, LACC, RACC,
    ADD, SUB, MUL, DIV, DOT, AND, OR, NOT, ASSIGN, EQUAL, NOTEQ, LESS, LESSEQ, GREATER, GREATEREQ,
    SPACE, LINECOMMENT, COMMENT
};


typedef struct _Token{
    int code;
    union{
        char c;
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

    printf("Token added, code: %s\n", codes[code]);

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
                else if(ch == '{'){
                    pCrtCh++;
                    state = 26;
                }
                else if(ch == '}'){
                    pCrtCh++;
                    state = 27;
                }
                else if(ch == '('){
                    pCrtCh++;
                    state = 28;
                }
                else if(ch == ')'){
                    pCrtCh++;
                    state = 29;
                }
                else if(ch == '<'){
                    pCrtCh++;
                    state = 30;
                }
                else if(ch == ';'){
                    pCrtCh++;
                    state = 31;
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
                else if(ch == '\''){
                    pCrtCh++;
                    state = 22; /// Start of CT_CHAR;
                }
                else if(ch == '\"'){
                    pCrtCh++;
                    state = -1; /// Start of CT_STRING;
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
                else{
                    pCrtCh++; //Just consume for now
                    //tkerr(addTk(END), "invalid character");
                }

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
                else if(nCh == 4 && !memcmp(pStartCh, "char", 4)){
                    tk=addTk(CHAR);
                }
                else if(nCh == 6 && !memcmp(pStartCh, "struct", 6)){
                    tk=addTk(STRUCT);
                }
                else if(nCh == 3 && !memcmp(pStartCh, "int", 3)){
                    tk=addTk(INT);
                }
                else if(nCh == 2 && !memcmp(pStartCh, "if", 2)){
                    tk=addTk(IF);
                }
                else if(nCh == 4 && !memcmp(pStartCh, "else", 4)){
                    tk=addTk(ELSE);
                }
                else if(nCh == 6 && !memcmp(pStartCh, "double", 6)){
                    tk=addTk(DOUBLE);
                }
                else if(nCh == 4 && !memcmp(pStartCh, "void", 4)){
                    tk=addTk(DOUBLE);
                }
                else{
                    tk = addTk(ID);

                    tk->text = (char*)malloc(nCh+1);
                    memcpy(tk->text, pStartCh, nCh);

                    tk->text[nCh] = 0;
                    printf("%s", tk->text);
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
                else if(ch == '.'){
                    pCrtCh++;
                    state = 15;
                }
                else if(ch == 'e' || ch == 'E'){
                    pCrtCh++;
                    state = 17;
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
                else if(isdigit(ch)){
                    pCrtCh++;
                    state = 15;
                }
                else if(ch == '.'){
                    pCrtCh++;
                    state = 16;
                }
                else if(ch == 'e' || ch == 'E'){
                    pCrtCh++;
                    state = 18;
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
                else if(ch == '/'){
                    while(*pCrtCh != '\n' && *pCrtCh != 0)
                        pCrtCh++;
                    line++;
                    addTk(LINECOMMENT);
                    return LINECOMMENT;
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
                break;
            }

            case 14:{ /// Final state for comment
                state = 0; /// comment fully ignored go to the beginning, TBD if need to be modified
                addTk(COMMENT);
                return COMMENT;
            }

            case 15:{
                if(isdigit(ch)){
                    pCrtCh++;
                }
                else if(ch == '.'){
                    pCrtCh++;
                    state = 16;
                }
                else if(ch == 'e' || ch == 'E'){
                    pCrtCh++;
                    state = 18;
                }
                else{
                    state = 21;
                }

                break;
            }

            case 16:{
                if(isdigit(ch)){
                    pCrtCh++;
                    state = 17;
                }
                else
                    tkerr(addTk(END), "invalid character s16");

                break;
            }

            case 17:{
                if(isdigit(ch)){
                    pCrtCh++;
                }
                else if(ch == 'e' || ch == 'E'){
                    pCrtCh++;
                    state = 18;
                }
                else{
                    state = 21;
                }
                break;
            }

            case 18:{
                if(ch == '-' || ch == '+'){
                    pCrtCh++;
                    state = 19;
                }
                else if(isdigit(ch)){
                    pCrtCh++;
                    state = 20;
                }
                else
                    tkerr(addTk(END), "invalid character s18");

                break;
            }

            case 19:{
                if(isdigit(ch)){
                    pCrtCh++;
                    state = 20;
                }
                else
                    tkerr(addTk(END), "invalid character s19");

                break;
            }

            case 20:{
                if(isdigit(ch))
                    pCrtCh++;
                else
                    state = 21;

                break;
            }

            case 21:{
                tk = addTk(CT_REAL);
                tk->r = strtod(pStartCh, &ptr);

                return tk->code;
            }

            case 22:{
                if(ch == '\\'){
                    pCrtCh++;
                    state = 23;
                }
                else{
                    pCrtCh++;
                    state = 24;
                }
                break;
            }

            case 23:{
                break;
            }

            case 24:{
                if(ch == '\''){
                    pCrtCh++;
                    state = 25;
                }
                break;
            }

            case 25:{
                tk = addTk(CT_CHAR);
                //tk->text = memccpy(pStartCh, &ptr);

                return tk->code;

            }

            case 26:{
                addTk(LACC);
                return LACC;
            }

            case 27:{
                addTk(RACC);
                return RACC;
            }
            case 28:{
                addTk(LBRACKET);
                return LBRACKET;
            }

            case 29:{
                addTk(RBRACKET);
                return RBRACKET;
            }

            case 30:{
                if(ch == '='){
                    pCrtCh++;
                    addTk(LESSEQ);
                    return LESSEQ;
                }
                else{
                    addTk(LESS);
                    return LESS;
                }
            }
            case 31:{
                addTk(SEMICOLON);
                return SEMICOLON;
            }
        }

    }

}

void printTokens(Token *head){
    Token *current = head;

    while(current != NULL){
        switch(current->code){
            case CT_INT:{
                printf("CT_INT: %d\n", current->i);
                break;
            }
            case CT_REAL:{
                printf("CT_REAL: %f\n", current->r);
                break;
            }
            default:{
                printf("Code: %s\n", codes[current->code]);
                break;
            }
        }

        current = current->next;
    }
}

int main()
{
    char *text = malloc(BLOCK_SIZE);
    FILE *f = fopen("input.txt", "r");


    while(fread(text,1, BLOCK_SIZE, f)){
        pCrtCh = text;
        while(*pCrtCh != 0)
            getNextToken();
    }
    addTk(END);

    printTokens(tokens);

    fclose(f);
    return 0;
}
