#include <bits/stdc++.h>
#include "symbol.h"

using namespace std;

#define SAFEALLOC(var, Type) if((var = (Type *)malloc(sizeof(Type)))==NULL) err("not enough memory")

#define BLOCK_SIZE 512
#define STACK_SIZE (32 * 1024)
#define GLOBAL_SIZE (32 * 1024)


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
    BREAK, CHAR, DOUBLE, ELSE, FOR, IF, INT, RETURN, STRUCT, VOID, WHILE, ///KEYWORDS
    CT_INT, CT_REAL, CT_CHAR, CT_STRING, /// CONSTANTS
    COMMA, SEMICOLON, LPAR, RPAR, LBRACKET, RBRACKET, LACC, RACC, /// DELIMITERS
    ADD, SUB, MUL, DIV, DOT, AND, OR, NOT, ASSIGN, EQUAL, NOTEQ, LESS, LESSEQ, GREATER, GREATEREQ, /// OPERATORS
    SPACE, LINECOMMENT, COMMENT
};

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
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}

int line = 0, offset, sizeArgs;
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
string ct_str;
int getNextToken(){
    int state = 0, nCh;
    char ch;
    long int c;
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


                /// VVVVVVVVVV ///
                /// DELIMITERS ///
                /// VVVVVVVVVV ///

                else if(ch == ','){
                    pCrtCh++;
                    state = 200;
                }
                else if(ch == ';'){
                    pCrtCh++;
                    state = 201;
                }
                else if(ch == '('){
                    pCrtCh++;
                    state = 202;
                }
                else if(ch == ')'){
                    pCrtCh++;
                    state = 203;
                }
                else if(ch == '['){
                    pCrtCh++;
                    state = 204;
                }
                else if(ch == ']'){
                    pCrtCh++;
                    state = 205;
                }
                else if(ch == '{'){
                    pCrtCh++;
                    state = 206;
                }
                else if(ch == '}'){
                    pCrtCh++;
                    state = 207;
                }


                /// VVVVVVVVV ///
                /// OPERATORS ///
                /// VVVVVVVVV ///

                else if(ch == '+'){
                    pCrtCh++;
                    state = 300;
                }
                else if(ch == '-'){
                    pCrtCh++;
                    state = 301;
                }
                else if(ch == '*'){
                    pCrtCh++;
                    state = 302;
                }
                else if(ch == '/'){
                    pCrtCh++;
                    state = 11; /// intermediate state to differ COMMENTS and DIV
                }
                else if(ch == '.'){
                    pCrtCh++;
                    state = 304;
                }
                else if(ch == '&'){
                    pCrtCh++;

                    if(*pCrtCh == '&'){
                        pCrtCh++;
                        state = 305;
                    }
                    else{
                        tkerr(addTk(END), "invalid character");
                    }

                }
                else if(ch == '|'){
                    pCrtCh++;

                    if(*pCrtCh == '|'){
                        pCrtCh++;
                        state = 306;
                    }
                    else{
                        tkerr(addTk(END), "invalid character");
                    }
                }
                else if(ch == '!'){
                    pCrtCh++;
                    state = 307;
                }
                else if(ch == '='){
                    pCrtCh++;
                    state = 310;
                }
                else if(ch == '<'){
                    pCrtCh++;
                    state = 313;
                }
                else if(ch == '>'){
                    pCrtCh++;
                    state = 316;
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

                else if(ch == '\''){
                    pCrtCh++;
                    state = 22; /// Start of CT_CHAR;
                }
                else if(ch == '"'){
                    pCrtCh++;
                    ct_str = "";
                    state = 31; /// Start of CT_STRING;
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
                else if(nCh == 6 && !memcmp(pStartCh, "double", 6)){
                    tk=addTk(DOUBLE);
                }
                else if(nCh == 4 && !memcmp(pStartCh, "else", 4)){
                    tk=addTk(ELSE);
                }
                else if(nCh == 3 && !memcmp(pStartCh, "for", 3)){
                    tk=addTk(FOR);
                }
                else if(nCh == 2 && !memcmp(pStartCh, "if", 2)){
                    tk=addTk(IF);
                }
                else if(nCh == 3 && !memcmp(pStartCh, "int", 3)){
                    tk=addTk(INT);
                }
                else if(nCh == 6 && !memcmp(pStartCh, "return", 6)){
                    tk=addTk(RETURN);
                }
                else if(nCh == 6 && !memcmp(pStartCh, "struct", 6)){
                    tk=addTk(STRUCT);
                }
                else if(nCh == 4 && !memcmp(pStartCh, "void", 4)){
                    tk=addTk(VOID);
                }
                else if(nCh == 5 && !memcmp(pStartCh, "while", 5)){
                    tk=addTk(WHILE);
                }
                else{
                    tk = addTk(ID);

                    tk->text = (char*)malloc(nCh+1);
                    memcpy(tk->text, pStartCh, nCh);
                    tk->text[nCh] = 0;
                }

                return tk->code;
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
                else if (ch == '.'){
                    pCrtCh++;
                    state = 16;
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
                else{
                    state = 303;
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
                    c = ch;
                    pCrtCh++;
                    state = 24;
                }
                break;
            }

            case 23:{
                switch(ch){
                    case 'a':{
                        pCrtCh++;
                        c = '\a';
                        state = 24;
                        break;
                    }
                    case 'b':{
                        pCrtCh++;
                        c = '\b';
                        state = 24;
                        break;
                    }
                    case 'f':{
                        pCrtCh++;
                        c = '\f';
                        state = 24;
                        break;
                    }
                    case 'n':{
                        pCrtCh++;
                        c = '\n';
                        state = 24;
                        break;
                    }
                    case 'r':{
                        pCrtCh++;
                        c = '\r';
                        state = 24;
                        break;
                    }
                    case 't':{
                        pCrtCh++;
                        c = '\t';
                        state = 24;
                        break;
                    }
                    case 'v':{
                        pCrtCh++;
                        c = '\v';
                        state = 24;
                        break;
                    }
                    case '\'':{
                        pCrtCh++;
                        c = '\'';
                        state = 24;
                        break;
                    }
                    case '?':{
                        pCrtCh++;
                        c = '\?';
                        state = 24;
                        break;
                    }
                    case '\"':{
                        pCrtCh++;
                        c = '\"';
                        state = 24;
                        break;
                    }
                    case '\\':{
                        pCrtCh++;
                        c = '\\';
                        state = 24;
                        break;
                    }
                    case '0':{
                        pCrtCh++;
                        c = '\0';
                        state = 24;
                        break;
                    }
                    default: {
                        tkerr(addTk(END), "invalid character s23");
                        break;
                    }
                }
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
                tk->i = c;
                return tk->code;
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
                if(ch == '\\'){
                    pCrtCh++;
                    state = 32;
                }
                else if(ch == '\"'){

                    pCrtCh++;
                    state = 33;
                }
                else {
                    ct_str.push_back(ch);
                    pCrtCh++;
                }

                break;
            }

            case 32:{
                switch(ch){
                    case 'a':{
                        pCrtCh++;
                        ct_str.push_back('\a');
                        state = 31;
                        break;
                    }
                    case 'b':{
                        pCrtCh++;
                        ct_str.push_back('\b');
                        state = 31;
                        break;
                    }
                    case 'f':{
                        pCrtCh++;
                        ct_str.push_back('\f');
                        state = 31;
                        break;
                    }
                    case 'n':{
                        pCrtCh++;
                        ct_str.push_back('\n');
                        state = 31;
                        break;
                    }
                    case 'r':{
                        pCrtCh++;
                        ct_str.push_back('\r');
                        state = 31;
                        break;
                    }
                    case 't':{
                        pCrtCh++;
                        ct_str.push_back('\t');
                        state = 31;
                        break;
                    }
                    case 'v':{
                        pCrtCh++;
                        ct_str.push_back('\v');
                        state = 31;
                        break;
                    }
                    case '\'':{
                        pCrtCh++;
                        ct_str.push_back('\'');
                        state = 31;
                        break;
                    }
                    case '?':{
                        pCrtCh++;
                        ct_str.push_back('\?');
                        state = 31;
                        break;
                    }
                    case '\"':{
                        pCrtCh++;
                        ct_str.push_back('\"');
                        state = 31;
                        break;
                    }
                    case '\\':{
                        pCrtCh++;
                        ct_str.push_back('\\');
                        state = 31;
                        break;
                    }
                    case '0':{
                        pCrtCh++;
                        ct_str.push_back('\0');
                        state = 31;
                        break;
                    }
                    default: {
                        tkerr(addTk(END), "invalid character s32");
                        break;
                    }
                }
                break;
            }

            case 33:{
                tk = addTk(CT_STRING);

                tk->text = (char*)malloc(ct_str.length());
                strcpy(tk->text, ct_str.c_str());

                cout << tk->text << endl;

                return CT_STRING;
            }

            /// VVVVVVVVVV ///
            /// DELIMITORS ///
            /// VVVVVVVVVV ///

            case 200:{
                addTk(COMMA);
                return COMMA;
            }
            case 201:{
                addTk(SEMICOLON);
                return SEMICOLON;
            }
            case 202:{
                addTk(LPAR);
                return LPAR;
            }
            case 203:{
                addTk(RPAR);
                return RPAR;
            }
            case 204:{
                addTk(LBRACKET);
                return LBRACKET;
            }
            case 205:{
                addTk(RBRACKET);
                return RBRACKET;
            }
            case 206:{
                addTk(LACC);
                return LACC;
            }
            case 207:{
                addTk(RACC);
                return RACC;
            }

            /// VVVVVVVVV ///
            /// OPERATORS ///
            /// VVVVVVVVV ///

            case 300:{
                addTk(ADD);
                return ADD;
            }
            case 301:{
                addTk(SUB);
                return SUB;
            }
            case 302:{
                addTk(MUL);
                return MUL;
            }
            case 303:{
                addTk(DIV);
                return DIV;
            }
            case 304:{
                addTk(DOT);
                return DOT;
            }
            case 305:{
                addTk(ADD);
                return ADD;
            }
            case 306:{
                addTk(OR);
                return OR;
            }
            case 307:{
                if(ch == '='){
                    pCrtCh++;
                    state=308;
                }
                else
                    state = 309;
                break;
            }
             case 308:{
                addTk(NOTEQ);
                return NOTEQ;
            }
             case 309:{
                addTk(NOT);
                return NOT;
            }
            case 310:{
                if(ch == '='){
                    pCrtCh++;
                    state=311;
                }
                else
                    state = 312;
                break;
            }
            case 311:{
                addTk(EQUAL);
                return EQUAL;
            }
            case 312:{
                addTk(ASSIGN);
                return ASSIGN;
            }
            case 313:{
                if(ch == '='){
                    pCrtCh++;
                    state=314;
                }
                else
                    state = 315;
                break;
            }
            case 314:{
                addTk(LESSEQ);
                return LESSEQ;
            }
            case 315:{
                addTk(LESS);
                return LESS;
            }
            case 316:{
                if(ch == '='){
                    pCrtCh++;
                    state=317;
                }
                else
                    state = 318;
                break;
            }
            case 317:{
                addTk(GREATEREQ);
                return GREATEREQ;
            }
            case 318:{
                addTk(GREATER);
                return GREATER;
            }
        }
    }
}

Token *crtTk;
Token *consumedTk;
int consume(int code){
    //printf("current code %s\n", codes[crtTk->code]);

    while(crtTk->code == LINECOMMENT || crtTk->code == COMMENT)
        crtTk = crtTk->next;

    if(crtTk->code == code){
        consumedTk = crtTk;
        crtTk = crtTk->next;

        printf("consumed %s\n", codes[code]);

        return 1;
    }
    return 0;
}

typedef union{
    long int i; // int, char
    double d; // double
    const char *str; // char[]
}CtVal;
typedef struct{
    Type type; // type of the result
    int isLVal; // if it is a LVal
    int isCtVal; // if it is a constant value (int, real, char, char[])
    CtVal ctVal; // the constat value
}RetVal;





char globals[GLOBAL_SIZE];
int nGlobals;

void *allocGlobal(int size){
    void *p = globals+nGlobals;

    if(nGlobals+size > GLOBAL_SIZE) err("insufficient globals space");

    nGlobals += size;
    return p;
}


/// Symbols ///
enum {TB_INT, TB_DOUBLE, TB_CHAR, TB_STRUCT, TB_VOID};
const char* TB[] = {"TB_INT", "TB_DOUBLE", "TB_CHAR", "TB_STRUCT", "TB_VOID"};

enum {CLS_VAR, CLS_FUNC, CLS_EXTFUNC, CLS_STRUCT};
const char* CLS[] = {"CLS_VAR", "CLS_FUNC", "CLS_EXTFUNC", "CLS_STRUCT"};

enum {MEM_GLOBAL, MEM_ARG, MEM_LOCAL};
const char* MEM[] = {"MEM_GLOBAL", "MEM_ARG", "MEM_LOCAL"};

vector <Symbol*> symbols;
int crtDepth = 0;

Symbol* addSymbol(vector <Symbol*>& symbols, const char *name, int cls){

    Symbol *s = new _Symbol();

    s -> name = name;
    s -> cls = cls;
    s -> depth = crtDepth;

    symbols.push_back(s);

    return s;
}


Symbol* findSymbol(vector <Symbol*>& symbols, const char *name){
    Symbol* found;
    for(int i = symbols.size() - 1; i >= 0; i--){

        cout << "sym " << symbols[i]->name << "\n";

        found = symbols[i];
        if(strcmp(found->name, name) == 0)
            return found;
    }
    return NULL;
}

Symbol *requireSymbol(vector <Symbol*>& symbols, const char *name){

    if(symbols.size() == 0)
        return NULL;

    for(int i = symbols.size() - 1; i >= 0; i--){
        Symbol* s = symbols[i];
        if(strcmp(s -> name, name) == 0)
            return s;
    }
    err("Symbol %s not found", name);
    return NULL;
}

void deleteSymbolsAfter(vector <Symbol*>& symbols, Symbol* symbol){
    while(symbols.size() > 0 && symbols.back() != symbol){
        Symbol* s = symbols.back();
        symbols.pop_back();
        free(s);
    }
}

void initSymbols(vector <Symbol*>& symbols){
    symbols.clear();
}

/// END OF - Symbols ///

/// TYPE ///

int typeBaseSize(Type *type);
int typeFullSize(Type *type);
int typeArgSize(Type *type);

Type createType(int typeBase, int nElements){
    Type t;
    t.typeBase = typeBase;
    t.nElements = nElements;
    return t;
}

int typeBaseSize(Type *type){
    int size=0;
    vector <Symbol*> :: iterator is;
    switch(type->typeBase){
        case TB_INT:size=sizeof(long int);break;
        case TB_DOUBLE:size=sizeof(double);break;
        case TB_CHAR:size=sizeof(char);break;
        case TB_STRUCT:
            for(is=type->s->members.begin();is!=type->s->members.end();is++){
                size+=typeFullSize(&(*is)->type);
            }
            break;
        case TB_VOID:size=0;break;
        default:err("invalid typeBase: %d",type->typeBase);
    }
    return size;
}

int typeFullSize(Type *type){
    return typeBaseSize(type)*(type->nElements>0?type->nElements:1);
}

int typeArgSize(Type *type){
    if(type->nElements>=0)
        return sizeof(void*);
    return typeBaseSize(type);
}

void cast(Type *dst,Type *src)
{
    if(src->nElements>-1){
        if(dst->nElements>-1){
                if(src->typeBase!=dst->typeBase)
                    tkerr(crtTk,"an array cannot be converted to an array of another type");
        }
        else{
            tkerr(crtTk,"an array cannot be converted to a non-array");
        }
    }
    else{
        if(dst->nElements>-1){
            tkerr(crtTk,"a non-array cannot be converted to an array");
        }
    }
    switch(src->typeBase){
        case TB_CHAR:
        case TB_INT:
        case TB_DOUBLE:
        switch(dst->typeBase){
            case TB_CHAR:
            case TB_INT:
            case TB_DOUBLE:
            return;
        }
        case TB_STRUCT:
            if(dst->typeBase==TB_STRUCT){
                if(src->s!=dst->s)
                    tkerr(crtTk,"a structure cannot be converted to another one");
                return;
            }
    }
    tkerr(crtTk,"incompatible types");
}

Symbol* addExtFunc(const char *name, Type type, void* addr){
    Symbol *s=addSymbol(symbols, name, CLS_EXTFUNC);
    s->type=type;
    s->addr = addr;
    initSymbols(s->args);
    return s;
}
Symbol *addFuncArg(Symbol *func, const char *name, Type type){
    Symbol *a;
    a=addSymbol(func->args,name,CLS_VAR);

    a->type=type;
    return a;
}

Type getArithType(Type *s1,Type *s2){
    Type t;
    t.nElements = -1;
    if(s1 -> typeBase == TB_DOUBLE || s2 -> typeBase == TB_DOUBLE){
        t.typeBase = TB_DOUBLE;
        return t;
    }

    if(s1 -> typeBase == TB_INT || s2 -> typeBase == TB_INT){
        t.typeBase = TB_INT;
        return t;
    }

    t.typeBase = TB_CHAR;
    return t;
}

int typeName(Type& ret);
int typeBase(Type& ret);

/// /TYPE ///


/// DEBUG FUNCTIONS ///

int DEBUG = 0, SUCCESS = 0;
/// prints when entering a state with current code
void print(string s){
    if(DEBUG){
        cout << s << '\n';
        cout << "CRT CODE: " << codes[crtTk->code] << "\n";
    }
}

/// prints when a state returns 1
void success(string s){
    if(SUCCESS){
        cout << "\nSUCCESSFULY READ: " << s;
    }
}

void printTokens(Token *head){
    Token *current = head;

    while(current != NULL){
        switch(current->code){
            case CT_INT:{
                printf("CT_INT: %ld\n", current->i);
                break;
            }
            case CT_REAL:{
                printf("CT_REAL: %f\n", current->r);
                break;
            }
            case CT_STRING:{
                printf("CT_STRING: %s\n", current->text);
                break;
            }
            case ID:{
                printf("ID: %s\n", current->text);
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
Symbol *crtFunc, *crtStruct;
void addVar(Token *tkName,Type t){
    Symbol *s;

    if(crtStruct){
        if(findSymbol(crtStruct->members,tkName->text))
            tkerr(crtTk,"symbol redefinition: %s",tkName->text);
        s=addSymbol(crtStruct->members,tkName->text, CLS_VAR);
    }

    else if(crtFunc){
            s=findSymbol(symbols,tkName->text);
            if(s&&s->depth==crtDepth)
                tkerr(crtTk,"symbol redefinition: %s",tkName->text);
            s=addSymbol(symbols,tkName->text,CLS_VAR);
            s->mem=MEM_LOCAL;
        }

    else{
        if(findSymbol(symbols,tkName->text))
            tkerr(crtTk,"symbol redefinition: %s",tkName->text);
        s=addSymbol(symbols,tkName->text,CLS_VAR);
        s->mem=MEM_GLOBAL;
    }

    s->type=t;

    if(crtStruct||crtFunc){
        s->offset=offset;
    }
    else{
        s->addr=allocGlobal(typeFullSize(&s->type));
    }
    offset+=typeFullSize(&s->type);
}



/// MV ///

char stk[STACK_SIZE];
char *SP;
char *stackAfter;

void pushd(double d){
    if(SP + sizeof(double) > stackAfter) err("out of stack");
    *(double*)SP = d;
    SP += sizeof(double);
}

double popd(){
    SP -= sizeof(double);
    if(SP < stk) err("not enough stack bytes for popd");
    return *(double*)SP;
}

void pushi(long int i){
    if(SP + sizeof(long int) > stackAfter) err("out of stack");
    *(long int*)SP =  i;
    SP += sizeof(long int);
}

int popi(){
    SP -= sizeof(long int);
    if(SP < stk) err("not enough stack bytes for popi");
    return *(int*)SP;
}

void pushc(char c){
    if(SP + sizeof(char) > stackAfter) err("out of stack");
    *(char*)SP = c;
    SP += sizeof(char);
}

char popc(){
    SP -= sizeof(char);
    if(SP < stk) err("not enough stack bytes for popc");
    return *(char*)SP;
}


void pusha(void* a){
    if(SP + sizeof(void *) > stackAfter) err("out of stack");
    *(void**)SP = a;
    SP += sizeof(void*);
}

void *popa(){
    SP -= sizeof(void*);
    if(SP < stk) err("not enough stack bytes for popa");
    return *(void**)SP;
}

void put_i(){
    printf("put_i -> %d\n",popi());
}

void get_i(){
    int x;
    scanf("%d", &x);
    pushi(x);
}

void put_s(){
    char* s = (char*)popa();
    printf("put_s ->%s", s);
}

void get_str(){
    char* s = (char*)popa();
    scanf("%s", s);
}

void put_d(){
    double d = popd();
    printf("put_d ->%lf\n", d);
}

void get_d(){
    double d;
    scanf("%lf", &d);
    pushd(d);
}

void put_c(){
    char c = popc();
    printf("put_c ->%c\n", c);
}

void get_c(){
    char c;
    scanf("%c", &c);
    pushc(c);
}


//double seconds();

enum {O_ADD_C, O_ADD_D, O_ADD_I,
    O_AND_A, O_AND_C, O_AND_D, O_AND_I,
    O_CALL, O_CALLEXT,
    O_CAST_C_D, O_CAST_C_I, O_CAST_D_C, O_CAST_D_I, O_CAST_I_C, O_CAST_I_D,
    O_DIV_C, O_DIV_D, O_DIV_I,
    O_DROP, O_ENTER,
    O_EQ_A, O_EQ_C, O_EQ_D, O_EQ_I,
    O_GREATER_C, O_GREATER_D, O_GREATER_I,
    O_GREATEREQ_C, O_GREATEREQ_D, O_GREATEREQ_I,
    O_HALT, O_INSERT,
    O_JF_A, O_JF_C, O_JF_D, O_JF_I,
    O_JMP,
    O_JT_A, O_JT_C, O_JT_D, O_JT_I,
    O_LESS_C, O_LESS_D, O_LESS_I,
    O_LESSEQ_C, O_LESSEQ_D, O_LESSEQ_I,
    O_LOAD,
    O_MUL_C, O_MUL_D, O_MUL_I,
    O_NEG_C, O_NEG_D, O_NEG_I,
    O_NOP,
    O_NOT_A, O_NOT_C, O_NOT_D, O_NOT_I,
    O_NOTEQ_A, O_NOTEQ_C, O_NOTEQ_D, O_NOTEQ_I,
    O_OFFSET,
    O_OR_A, O_OR_C, O_OR_D, O_OR_I,
    O_PUSHFPADDR,
    O_PUSHCT_A, O_PUSHCT_C, O_PUSHCT_D, O_PUSHCT_I,
    O_RET, O_STORE,
    O_SUB_C, O_SUB_D, O_SUB_I
};

const char* opcodes[] = {"O_ADD_C", "O_ADD_D", "O_ADD_I",
    "O_AND_A", "O_AND_C", "O_AND_D", "O_AND_I",
    "O_CALL", "O_CALLEXT",
    "O_CAST_C_D", "O_CAST_C_I", "O_CAST_D_C", "O_CAST_D_I", "O_CAST_I_C", "O_CAST_I_D",
    "O_DIV_C", "O_DIV_D", "O_DIV_I",
    "O_DROP", "O_ENTER",
    "O_EQ_A", "O_EQ_C", "O_EQ_D", "O_EQ_I",
    "O_GREATER_C", "O_GREATER_D", "O_GREATER_I",
    "O_GREATEREQ_C", "O_GREATEREQ_D", "O_GREATEREQ_I",
    "O_HALT", "O_INSERT",
    "O_JF_A", "O_JF_C", "O_JF_D", "O_JF_I",
    "O_JMP",
    "O_JT_A", "O_JT_C", "O_JT_D", "O_JT_I",
    "O_LESS_C", "O_LESS_D", "O_LESS_I",
    "O_LESSEQ_C", "O_LESSEQ_D", "O_LESSEQ_I",
    "O_LOAD",
    "O_MUL_C", "O_MUL_D", "O_MUL_I",
    "O_NEG_C", "O_NEG_D", "O_NEG_I",
    "O_NOP",
    "O_NOT_A", "O_NOT_C", "O_NOT_D", "O_NOT_I",
    "O_NOTEQ_A", "O_NOTEQ_C", "O_NOTEQ_D", "O_NOTEQ_I",
    "O_OFFSET",
    "O_OR_A", "O_OR_C", "O_OR_D","O_OR_I",
    "O_PUSHFPADDR",
    "O_PUSHCT_A", "O_PUSHCT_C", "O_PUSHCT_D", "O_PUSHCT_I",
    "O_RET", "O_STORE",
    "O_SUB_C", "O_SUB_D", "O_SUB_I"
};

typedef struct _Instr{
    int opcode;
    union{
        long int i;
        double d;
        void *addr;
    }args[2];
    struct _Instr *last, *next;
}Instr;
Instr *instructions, *lastInstruction, *crtLoopEnd;

void printInstr(){
    Instr *curr = instructions;
    while(curr != NULL){
        cout << curr->opcode << ' ' << opcodes[curr->opcode] << '\n';
        curr = curr->next;
    }
}

Instr *createInstr(int opcode){
    Instr *i;
    SAFEALLOC(i, Instr);
    i->opcode=opcode;
    return i;
}

void insertInstrAfter(Instr *after, Instr *i){
    i->next=after->next;
    i->last=after;
    after->next=i;
    if(i->next==NULL)lastInstruction=i;
}

Instr *addInstr(int opcode){
    Instr *i=createInstr(opcode);
    i->next=NULL;
    i->last=lastInstruction;

    if(lastInstruction)
        lastInstruction->next=i;
    else
        instructions=i;

    lastInstruction=i;
    return i;
}

Instr *addInstrAfter(Instr *after,int opcode){
    Instr *i=createInstr(opcode);
    insertInstrAfter(after,i);

    return i;
}

Instr *addInstrA(int opcode,void *addr){
    Instr *i=createInstr(opcode);

    i->args[0].addr = addr;
    i->next=NULL;
    i->last=lastInstruction;

    if(lastInstruction)
        lastInstruction->next=i;
    else
        instructions=i;

    lastInstruction=i;
    return i;
}

Instr *addInstrI(int opcode, long int val){
    Instr *i=createInstr(opcode);

    i->args[0].i = val;
    i->next=NULL;
    i->last=lastInstruction;

    if(lastInstruction)
        lastInstruction->next=i;
    else
        instructions=i;

    lastInstruction=i;
    return i;
}

Instr *addInstrII(int opcode, long int val1, long int val2){
    Instr *i=createInstr(opcode);

    i->args[0].i = val1;
    i->args[1].i = val2;
    i->next=NULL;
    i->last=lastInstruction;

    if(lastInstruction)
        lastInstruction->next=i;
    else
        instructions=i;

    lastInstruction=i;
    return i;
}

Instr* appendInstr(Instr* i){
    //printf("Added %d\n", i -> opcode);
    i->next=NULL;
    i->last=lastInstruction;
    if(lastInstruction){
        lastInstruction->next=i;
    }else{
        instructions=i;
    }
    lastInstruction=i;
    return i;
}

void deleteInstructionsAfter(Instr* start){
    while(lastInstruction != NULL && lastInstruction != start){
        Instr* aux = lastInstruction;
        lastInstruction = lastInstruction -> last;

        if(lastInstruction != NULL)
            lastInstruction -> next = NULL;

        free(aux);
    }
}



/// END OF MV ///

void addExtFunctions(){
    Symbol* s;

    s=addExtFunc("put_s",createType(TB_VOID,-1), (void*)put_s);
    addFuncArg(s,"s",createType(TB_CHAR,0));

    //printf("Added put_s");

    s = addExtFunc("get_str", createType(TB_VOID, -1), (void*)get_str);
    addFuncArg(s, "s", createType(TB_CHAR, 0));

    //printf("Added get_str");

    s = addExtFunc("put_i", createType(TB_VOID, -1), (void*)put_i);
    addFuncArg(s, "i", createType(TB_INT, -1));

    //printf("Added put_i");

    s = addExtFunc("get_i", createType(TB_INT, -1), (void*)get_i);

    //printf("Added get_i");

    s=addExtFunc("put_d",createType(TB_VOID,-1), (void*)put_d);
    addFuncArg(s,"d",createType(TB_DOUBLE, -1));

    //printf("Added put_d");

    s = addExtFunc("get_d", createType(TB_DOUBLE, -1), (void*)get_d);

    //printf("Added get_d");

    s=addExtFunc("put_c",createType(TB_VOID,-1), (void*)put_c);
    addFuncArg(s,"c",createType(TB_CHAR, -1));

    //printf("Added put_c");

    s = addExtFunc("get_c", createType(TB_CHAR, -1), (void*)get_c);

    //printf("Added get_c");

    //s = addExtFunc("seconds", createType(TB_DOUBLE, -1), (void*)seconds);
}

void run(Instr *IP)
{
    long int iVal1,iVal2;
    double dVal1,dVal2;
    char *aVal1, *aVal2;
    char *FP=0,*oldSP;
    char cVal1, cVal2;
    SP=stk;
    stackAfter=stk+STACK_SIZE;
    while(1){
        //printf("%p/%d\t",IP,SP-stk);
        switch(IP->opcode){
            case O_NOP:
                IP = IP -> next;
                break;
            case O_ADD_C:
                cVal1 = popc();
                cVal2 = popc();
                pushc(cVal1 + cVal2);
                printf("ADD_C %d %d\n", cVal1, cVal2);
                IP = IP -> next;
                break;
            case O_ADD_D:{
                double val1 = popd();
                double val2 = popd();
                pushd(val1 + val2);
                printf("ADD_D %lf %lf\n", (double)val1, (double)val2);
                IP = IP -> next;
                break;
            }
            case O_ADD_I:{
                int val1 = popi();
                int val2 = popi();
                pushi(val1 + val2);
                printf("ADD_I %d %d\n", val1, val2);
                IP = IP -> next;
                break;
            }
            case O_MUL_I:{
                int val1 = popi();
                int val2 = popi();
                pushi(val1 * val2);
                printf("MUL_I %d %d\n", val1, val2);
                IP = IP -> next;
                break;
            }
            case O_MUL_C:{
                char val1 = popc();
                char val2 = popc();
                pushc(val1 * val2);
                printf("MUL_C %d %d\n", (int)val1, (int)val2);
                IP = IP -> next;
                break;
            }
            case O_MUL_D:{
                double val1 = popd();
                double val2 = popd();
                pushd(val1 * val2);
                printf("MUL_D %lf %lf\n", (double)val1, (double)val2);
                IP = IP -> next;
                break;
            }
            case O_NEG_C:{
                char val1 = popc();
                //char val2 = popc();
                pushc(-val1);
                printf("NEG_C %d", (int)val1);
                IP = IP -> next;
                break;
            }
            case O_NEG_D:{
                double val1 = popd();
                //double val2 = popd();
                pushd(-val1);
                printf("NEG_D %lf\n", (double)val1);
                IP = IP -> next;
                break;
            }
            case O_NEG_I:{
                int val1 = popi();
                //int val2 = popi();
                pushi(-val1);
                printf("NEG_I %d\n", val1);
                IP = IP -> next;
                break;
            }
            case O_NOT_C:{
                char val1 = popc();
                //char val2 = popc();
                pushi(!val1);
                printf("NOT_C %d", (int)val1);
                IP = IP -> next;
                break;
            }
            case O_NOT_D:{
                double val1 = popd();
                //double val2 = popd();
                pushi(!val1);
                printf("NOT_D %lf\n", (double)val1);
                IP = IP -> next;
                break;
            }
            case O_NOT_I:{
                int val1 = popi();
                //int val2 = popi();
                pushi(!val1);
                printf("NOT_I %d\n", val1);
                IP = IP -> next;
                break;
            }
            case O_NOT_A:{
                void* val1 = popa();
                //int val2 = popi();
                pushi(!val1);
                printf("NOT_A %p\n", val1);
                IP = IP -> next;
                break;
            }
            case O_AND_C:{
                char val1 = popc();
                char val2 = popc();
                pushi(val1 && val2);
                printf("AND_C %d %d\n", (int)val1, (int)val2);
                IP = IP -> next;
                break;
            }
            case O_AND_I:{
                int val1 = popi();
                int val2 = popi();
                pushi(val1 && val2);
                printf("AND_I %d %d\n", (int)val1, (int)val2);
                IP = IP -> next;
                break;
            }
            case O_AND_A:{
                void* val1 = popa();
                void* val2 = popa();
                pushi(val1 && val2);
                printf("AND_A %p %p\n", val1, val2);
                IP = IP -> next;
                break;
            }
            case O_AND_D:{
                double val1 = popd();
                double val2 = popd();
                pushi(val1 && val2);
                printf("AND_D %lf %lf\n", val1, val2);
                IP = IP -> next;
                break;
            }

            case O_OR_C:{
                char val1 = popc();
                char val2 = popc();
                pushi(val1 || val2);
                printf("OR_C %d %d\n", (int)val1, (int)val2);
                IP = IP -> next;
                break;
            }
            case O_OR_I:{
                int val1 = popi();
                int val2 = popi();
                pushi(val1 || val2);
                printf("OR_I %d %d\n", (int)val1, (int)val2);
                IP = IP -> next;
                break;
            }
            case O_OR_A:{
                void* val1 = popa();
                void* val2 = popa();
                pushi(val1 || val2);
                printf("OR_A %p %p\n", val1, val2);
                IP = IP -> next;
                break;
            }
            case O_OR_D:{
                double val1 = popd();
                double val2 = popd();
                pushi(val1 || val2);
                printf("OR_D %lf %lf\n", val1, val2);
                IP = IP -> next;
                break;
            }
            case O_CALL:{
                aVal1=(char*)IP->args[0].addr;
                printf("CALL\t%p\n",aVal1);
                pusha(IP->next);
                IP=(Instr*)aVal1;
                break;
            }
            case O_CALLEXT:
                printf("CALLEXT\t%p\n",IP->args[0].addr);
                (*(void(*)())IP->args[0].addr)();
                IP=IP->next;
                break;
            case O_CAST_I_D:
                iVal1=popi();
                dVal1=(double)iVal1;
                printf("CAST_I_D\t(%ld -> %g)\n",iVal1,dVal1);
                pushd(dVal1);
                IP=IP->next;
                break;
            case O_CAST_I_C:
                iVal1=popi();
                cVal1=(char)iVal1;
                printf("CAST_I_C\t(%d -> %c)\n",iVal1,dVal1);
                pushc(cVal1);
                IP=IP->next;
                break;
            case O_CAST_D_I:
                dVal1=popd();
                iVal1=(int)dVal1;
                printf("CAST_D_I\t(%lf -> %d)\n",dVal1,iVal1);
                pushi(iVal1);
                IP=IP->next;
                break;
            case O_CAST_D_C:
                dVal1=popd();
                cVal1=(char)dVal1;
                printf("CAST_D_C\t(%lf -> %c)\n",dVal1,cVal1);
                pushc(cVal1);
                IP=IP->next;
                break;
            case O_CAST_C_D:
                cVal1=popc();
                dVal1=(double)cVal1;
                printf("CAST_C_D\t(%c -> %lf)\n",cVal1,dVal1);
                pushd(dVal1);
                IP=IP->next;
                break;
            case O_CAST_C_I:
                cVal1=popc();
                iVal1=(int)cVal1;
                printf("CAST_C_I\t(%c -> %d)\n",cVal1,iVal1);
                pushi(iVal1);
                IP=IP->next;
                break;
            case O_DIV_C:{
                char a = popc();
                char b = popc();
                printf("DIV_C %c %c\n", a, b);
                pushc(b / a);
                break;
            }
            case O_DIV_I:{
                int a = popi();
                int b = popi();
                printf("DIV_I %d %d\n", a, b);
                pushi(b / a);
                break;
            }
            case O_DIV_D:{
                double a = popd();
                double b = popd();
                printf("DIV_D %lf %lf\n", a, b);
                pushd(b / a);
                break;
            }
            case O_DROP:
                iVal1=IP->args[0].i;
                printf("DROP\t%ld\n",iVal1);
                if(SP-iVal1<stk)err("not enough stack bytes");
                SP-=iVal1;
                IP=IP->next;
                break;
            case O_ENTER:
                iVal1=IP->args[0].i;
                printf("ENTER\t%ld\n",iVal1);
                pusha(FP);
                FP=SP;
                SP+=iVal1;
                IP=IP->next;
                break;
            case O_EQ_D:
                dVal1=popd();
                dVal2=popd();
                printf("EQ_D\t(%g==%g -> %ld)\n",dVal2,dVal1,dVal2==dVal1);
                pushi(dVal2==dVal1);
                IP=IP->next;
                break;
            case O_EQ_C:
                cVal1=popc();
                cVal2=popc();
                printf("EQ_C\t(%c==%c -> %ld)\n",cVal2,cVal1,cVal2==cVal1);
                pushi(cVal2==cVal1);
                IP=IP->next;
                break;
            case O_EQ_I:
                iVal1=popi();
                iVal2=popi();
                printf("EQ_I\t(%d==%d -> %ld)\n",iVal2,iVal1,iVal2==iVal1);
                pushi(iVal2==iVal1);
                IP=IP->next;
                break;
            case O_EQ_A:
                aVal1=(char*)popa();
                aVal2=(char*)popa();
                printf("EQ_A\t(%p==%p -> %ld)\n",aVal2,aVal1,aVal2==aVal1);
                pushi(aVal2==aVal1);
                IP=IP->next;
                break;
            case O_NOTEQ_D:
                dVal1=popd();
                dVal2=popd();
                printf("NOTEQ_D\t(%g!=%g -> %ld)\n",dVal2,dVal1,dVal2!=dVal1);
                pushi(dVal2!=dVal1);
                IP=IP->next;
                break;
            case O_NOTEQ_C:
                cVal1=popc();
                cVal2=popc();
                printf("NOTEQ_C\t(%c!=%c -> %ld)\n",cVal2,cVal1,cVal2!=cVal1);
                pushi(cVal2!=cVal1);
                IP=IP->next;
                break;
            case O_NOTEQ_I:
                iVal1=popi();
                iVal2=popi();
                printf("NOTEQ_I\t(%d!=%d -> %ld)\n",iVal2,iVal1,iVal2!=iVal1);
                pushi(iVal2!=iVal1);
                IP=IP->next;
                break;
            case O_NOTEQ_A:
                aVal1=(char*)popa();
                aVal2=(char*)popa();
                printf("NOTEQ_A\t(%p!=%p -> %ld)\n",aVal2,aVal1,aVal2!=aVal1);
                pushi(aVal2!=aVal1);
                IP=IP->next;
                break;
            case O_HALT:
                printf("HALT\n");
                return;
            case O_INSERT:
                iVal1=IP->args[0].i; // iDst
                iVal2=IP->args[1].i; // nBytes
                printf("INSERT\t%ld,%ld\n",iVal1,iVal2);
                if(SP+iVal2>stackAfter)err("out of stack");
                memmove(SP-iVal1+iVal2,SP-iVal1,iVal1); //make room
                memmove(SP-iVal1,SP+iVal2,iVal2); //dup
                SP+=iVal2;
                IP=IP->next;
                break;
            case O_GREATER_D:
                dVal1=popd();
                dVal2=popd();
                printf("GREATER_D\t(%g > %g -> %ld)\n",dVal2,dVal1,dVal2 > dVal1);
                pushi(dVal2 > dVal1);
                IP=IP->next;
                break;
            case O_GREATER_C:
                cVal1=popc();
                cVal2=popc();
                printf("GREATER_C\t(%c > %c -> %ld)\n",cVal2,cVal1,cVal2 > cVal1);
                pushi(cVal2 > cVal1);
                IP=IP->next;
                break;
            case O_GREATER_I:
                iVal1=popi();
                iVal2=popi();
                printf("GREATER_I\t(%d > %d -> %ld)\n",iVal2,iVal1,iVal2 > iVal1);
                pushi(iVal2 > iVal1);
                IP=IP->next;
                break;
            case O_GREATEREQ_D:
                dVal1=popd();
                dVal2=popd();
                printf("GREATEREQ_D\t(%g >= %g -> %ld)\n",dVal2,dVal1,dVal2 >= dVal1);
                pushi(dVal2 >= dVal1);
                IP=IP->next;
                break;
            case O_GREATEREQ_C:
                cVal1=popc();
                cVal2=popc();
                printf("GREATEREQ_C\t(%c >= %c -> %ld)\n",cVal2,cVal1,cVal2 >= cVal1);
                pushi(cVal2 >= cVal1);
                IP=IP->next;
                break;
            case O_GREATEREQ_I:
                iVal1=popi();
                iVal2=popi();
                printf("GREATEREQ_I\t(%d >= %d -> %ld)\n",iVal2,iVal1,iVal2 >= iVal1);
                pushi(iVal2 >= iVal1);
                IP=IP->next;
                break;

            case O_JT_I:
                iVal1=popi();
                printf("JT\t%p\t(%ld)\n",IP->args[0].addr,iVal1);
                IP=iVal1?(Instr*)IP->args[0].addr:IP->next;
                break;
            case O_JT_C:
                cVal1=popc();
                printf("JT\t%p\t(%c)\n",IP->args[0].addr,cVal1);
                IP=cVal1?(Instr*)IP->args[0].addr:IP->next;
                break;
            case O_JT_A:
                aVal1 = (char*)popa();
                printf("JT\t%p\t(%p)\n",IP->args[0].addr,aVal1);
                IP=aVal1?(Instr*)IP->args[0].addr:IP->next;
                break;
            case O_JT_D:
                dVal1 = popd();
                printf("JT\t%p\t(%lf)\n",IP->args[0].addr,dVal1);
                IP=dVal1?(Instr*)IP->args[0].addr:IP->next;
                break;
            case O_JF_I:
                iVal1=popi();
                printf("JF\t%p\t(%ld)\n",IP->args[0].addr,iVal1);
                IP=!iVal1?(Instr*)IP->args[0].addr:IP->next;
                break;
            case O_JF_C:
                cVal1=popc();
                printf("JF\t%p\t(%c)\n",IP->args[0].addr,cVal1);
                IP=!cVal1?(Instr*)IP->args[0].addr:IP->next;
                break;
            case O_JF_A:
                aVal1 = (char*)popa();
                printf("JF\t%p\t(%p)\n",IP->args[0].addr,aVal1);
                IP=!aVal1?(Instr*)IP->args[0].addr:IP->next;
                break;
            case O_JF_D:
                dVal1 = popd();
                printf("JF\t%p\t(%lf)\n",IP->args[0].addr,dVal1);
                IP=dVal1?(Instr*)IP->args[0].addr:IP->next;
                break;
            case O_JMP:
                printf("JMP\t%p\t(%lf)\n",IP->args[0].addr);
                IP=(Instr*)IP->args[0].addr;
                break;
            case O_LESS_D:
                dVal1=popd();
                dVal2=popd();
                printf("LESS_D\t(%g < %g -> %ld)\n",dVal2,dVal1,dVal2 < dVal1);
                pushi(dVal2 < dVal1);
                IP=IP->next;
                break;
            case O_LESS_C:
                cVal1=popc();
                cVal2=popc();
                printf("LESS_C\t(%c < %c -> %ld)\n",cVal2,cVal1,cVal2 < cVal1);
                pushi(cVal2 < cVal1);
                IP=IP->next;
                break;
            case O_LESS_I:
                iVal1=popi();
                iVal2=popi();
                printf("LESS_I\t(%d < %d -> %ld)\n",iVal2,iVal1,iVal2 < iVal1);
                pushi(iVal2 < iVal1);
                IP=IP->next;
                break;
            case O_LESSEQ_D:
                dVal1=popd();
                dVal2=popd();
                printf("LESSEQ_D\t(%g <= %g -> %ld)\n",dVal2,dVal1,dVal2 <= dVal1);
                pushi(dVal2 <= dVal1);
                IP=IP->next;
                break;
            case O_LESSEQ_C:
                cVal1=popc();
                cVal2=popc();
                printf("LESSEQ_C\t(%c <= %c -> %ld)\n",cVal2,cVal1,cVal2 <= cVal1);
                pushi(cVal2 <= cVal1);
                IP=IP->next;
                break;
            case O_LESSEQ_I:
                iVal1=popi();
                iVal2=popi();
                printf("LESSEQ_I\t(%d <= %d -> %ld)\n",iVal2,iVal1,iVal2 <= iVal1);
                pushi(iVal2 <= iVal1);
                IP=IP->next;
                break;
            case O_LOAD:
                iVal1=IP->args[0].i;
                aVal1=(char*)popa();
                printf("LOAD\t%ld\t(%p)\n",iVal1,aVal1);
                if(SP+iVal1>stackAfter)err("out of stack");
                memcpy(SP,aVal1,iVal1);
                SP+=iVal1;
                IP=IP->next;
                break;
            case O_OFFSET:
                iVal1=popi();
                aVal1=(char*)popa();
                printf("OFFSET\t(%p+%ld -> %p)\n",aVal1,iVal1,aVal1+iVal1);
                pusha(aVal1+iVal1);
                IP=IP->next;
                break;
            case O_PUSHFPADDR:
                iVal1=IP->args[0].i;
                printf("PUSHFPADDR\t%ld\t(%p)\n",iVal1,FP+iVal1);
                pusha(FP+iVal1);
                IP=IP->next;
                break;
            case O_PUSHCT_A:
                aVal1=(char*)IP->args[0].addr;
                printf("PUSHCT_A\t%p\n",aVal1);
                pusha(aVal1);
                IP=IP->next;
                break;
            case O_PUSHCT_C:
                cVal1=IP->args[0].i;
                printf("PUSHCT_c\t%c\n",cVal1);
                pushc(cVal1);
                IP=IP->next;
                break;
            case O_PUSHCT_D:
                dVal1=IP->args[0].d;
                printf("PUSHCT_D\t%lf\n",dVal1);
                pushd(dVal1);
                IP=IP->next;
                break;
            case O_PUSHCT_I:
                iVal1=IP->args[0].i;
                printf("PUSHCT_I\t%d\n",iVal1);
                pushi(iVal1);
                IP=IP->next;
                break;
            case O_RET:
                iVal1=IP->args[0].i; // sizeArgs
                iVal2=IP->args[1].i; // sizeof(retType)
                printf("RET\t%ld,%ld\n",iVal1,iVal2);
                oldSP=SP;
                SP=FP;
                FP=(char*)popa();
                IP=(Instr*)popa();
                if(SP-iVal1<stk)err("not enough stack bytes");
                SP-=iVal1;
                memmove(SP,oldSP-iVal2,iVal2);
                SP+=iVal2;
                break;
            case O_STORE:
                iVal1=IP->args[0].i;

                if(SP-(sizeof(void*)+iVal1)<stk)
                    err("not enough stack bytes for SET");

                aVal1=(char*)(*(void**)(SP-((sizeof(void*)+iVal1))));
                printf("STORE\t%ld\t(%p)\n",iVal1,aVal1);
                memcpy(aVal1,SP-iVal1,iVal1);
                SP-=sizeof(void*)+iVal1;
                IP=IP->next;
                break;
            case O_SUB_D:
                dVal1=popd();
                dVal2=popd();
                printf("SUB_D\t(%g-%g -> %g)\n",dVal2,dVal1,dVal2-dVal1);
                pushd(dVal2-dVal1);
                IP=IP->next;
                break;
            case O_SUB_C:
                cVal1=popc();
                cVal2=popc();
                printf("SUB_C\t(%c-%c -> %c)\n",cVal2,cVal1,cVal2-cVal1);
                pushc(cVal2-cVal1);
                IP=IP->next;
                break;
            case O_SUB_I:
                iVal1=popi();
                iVal2=popi();
                printf("SUB_I\t(%ld-%ld -> %ld)\n",iVal2,iVal1,iVal2-iVal1);
                pushi(iVal2-iVal1);
                IP=IP->next;
                break;
            default:
                err("invalid opcode: %d",IP->opcode);
        }
    }
}

void mvTest()
{
    Instr *L1;
    int *v=(int*)allocGlobal(sizeof(long int));
    addInstrA(O_PUSHCT_A,v);
    addInstrI(O_PUSHCT_I,3);
    addInstrI(O_STORE,sizeof(long int));
    L1=addInstrA(O_PUSHCT_A,v);
    addInstrI(O_LOAD,sizeof(long int));
    addInstrA(O_CALLEXT,requireSymbol(symbols,"put_i")->addr);
    addInstrA(O_PUSHCT_A,v);
    addInstrA(O_PUSHCT_A,v);
    addInstrI(O_LOAD,sizeof(long int));
    addInstrI(O_PUSHCT_I,1);
    addInstr(O_SUB_I);
    addInstrI(O_STORE,sizeof(long int));
    addInstrA(O_PUSHCT_A,v);
    addInstrI(O_LOAD,sizeof(long int));
    addInstrA(O_JT_I,L1);
    addInstr(O_HALT);
}

Instr *getRVal(RetVal *rv)
{
    if(rv->isLVal){
        switch(rv->type.typeBase){
                case TB_INT:
                case TB_DOUBLE:
                case TB_CHAR:
                case TB_STRUCT:
                        addInstrI(O_LOAD,typeArgSize(&rv->type));
                        break;
                default:tkerr(crtTk,"unhandled type: %s",TB[rv->type.typeBase]);
            }
    }
    return lastInstruction;
}

void addCastInstr(Instr *after,Type *actualType,Type *neededType)
{
    if(actualType->nElements>=0||neededType->nElements>=0)return;
    switch(actualType->typeBase){
        case TB_CHAR:
                switch(neededType->typeBase){
                        case TB_CHAR:break;
                        case TB_INT:addInstrAfter(after,O_CAST_C_I);break;
                        case TB_DOUBLE:addInstrAfter(after,O_CAST_C_D);break;
                        }
                break;
        case TB_INT:
                switch(neededType->typeBase){
                        case TB_CHAR:addInstrAfter(after,O_CAST_I_C);break;
                        case TB_INT:break;
                        case TB_DOUBLE:addInstrAfter(after,O_CAST_I_D);break;
                        }
                break;
        case TB_DOUBLE:
                switch(neededType->typeBase){
                        case TB_CHAR:addInstrAfter(after,O_CAST_D_C);break;
                        case TB_INT:addInstrAfter(after,O_CAST_D_I);break;
                        case TB_DOUBLE:break;
                        }
                break;
    }
}

Instr *createCondJmp(RetVal *rv)
{
    if(rv->type.nElements>=0){  // arrays
        return addInstr(O_JF_A);
    }
    else{                              // non-arrays
        getRVal(rv);
        switch(rv->type.typeBase){
            case TB_CHAR:return addInstr(O_JF_C);
            case TB_DOUBLE:return addInstr(O_JF_D);
            case TB_INT:return addInstr(O_JF_I);
            default:return NULL;
            }
        }
}


/// Syntactic Analyzer Functions ///
int unit();

int ruleIf(RetVal& rv, Instr* &i1, Instr* &i2);
int ruleWhile(RetVal& rv, Instr* &i1, Instr* &i2);
int ruleFor();
int ruleBreak();
int ruleReturn(Instr* &i);

int declStruct();
int declVar();
int typeBase(Type& type);
int arrayDecl(Type& type);
int typeName();

int declFunc();
int funcArg();

int stm();
int stmCompound();

int expr(RetVal& rv);
int exprAssign(RetVal& rv);
int exprOr(RetVal& rv);
int exprOr1(RetVal& rv);
int exprAnd(RetVal& rv);
int exprAnd1(RetVal& rv);
int exprEq(RetVal& rv);
int exprEq1(RetVal& rv);
int exprRel(RetVal& rv);
int exprRel1(RetVal& rv);
int exprAdd(RetVal& rv);
int exprAdd1(RetVal& rv);
int exprMul(RetVal& rv);
int exprMul1(RetVal& rv);
int exprCast(RetVal& rv);
int exprUnary(RetVal& rv);
int exprPostfix(RetVal& rv);
int exprPostfix1(RetVal& rv);
int exprPrimary(RetVal& rv);
/// END OF - Syntactic Analyzer Functions ///

int main()
{
    addExtFunctions();
    //mvTest();
    //run(instructions);

    FILE *f = fopen("7.c", "rb");



    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *text = (char*)malloc(size);

    fread(text,size, 1, f);
    text[size-1] = 0;
    pCrtCh = text;

    for(;;){
        getNextToken();
        //printf("%s ", codes[lastToken->code]);
        if(lastToken->code == END)
            break;
    }

    printTokens(tokens);

    crtTk = tokens;
    if(!unit()){
        tkerr(crtTk, "Syntax not valid");
    }

    printInstr();

    run(instructions);

    fclose(f);


    return 0;
}

int unit(){
    print("unit\n");

    Token *startTk = crtTk;
    Instr* startInstr = lastInstruction;
    Instr *labelMain=addInstr(O_CALL);
    addInstr(O_HALT);

    for(;;){
        if(declStruct() || declFunc() || declVar()) {}
        else break;
    }

    labelMain->args[0].addr=requireSymbol(symbols, "main")->addr;

    if(!consume(END)){
        crtTk = startTk;
        deleteInstructionsAfter(startInstr);
        tkerr(crtTk, "missing END token in unit");
    }
    success("unit\n");
    return 1;
}

int declStruct(){
    print("declStruct\n");

    Token *startTk = crtTk;


    if(consume(STRUCT)){
        Token *tkName = crtTk;
        if(consume(ID)){
            if(consume(LACC)){
                offset = 0;

                if(findSymbol(symbols,tkName->text))
                    tkerr(crtTk,"symbol redefinition: %s",tkName->text);
                crtStruct=addSymbol(symbols,tkName->text,CLS_STRUCT);
                initSymbols(crtStruct->members);

                for(;;){
                    if(declVar()){

                    }
                    else break;
                }

                if(consume(RACC)){
                    if(consume(SEMICOLON)){
                        crtStruct = NULL;
                        return 1;
                    }
                    else{
                        tkerr(crtTk, "Expected ';' after struct declaration");
                    }
                }
                else{
                    tkerr(crtTk, "Expected }");
                }
            }
            else{
                crtTk = startTk;
                return 0;
            }
        }
        else{
            tkerr(crtTk, "Expected ID");
        }

    }
    crtTk = startTk;
    return 0;
}

///declVar:  typeBase ID arrayDecl? ( COMMA ID arrayDecl? )* SEMICOLON ;
int declVar(){
    print("declVar\n");

    Token *startTk = crtTk;
    Type t;

    if(typeBase(t)){
        Token* tkName = crtTk;

        if(consume(ID)){
            if(arrayDecl(t)){

            }
            else{
                t.nElements = -1;
            }

            addVar(tkName, t);

            for(;;){
                if(consume(COMMA)){
                    Token* tkName = crtTk;
                    if(consume(ID)){
                        if(arrayDecl(t)){

                        }
                        else{
                            t.nElements = -1;
                        }

                        addVar(tkName, t);
                    }
                    else{
                        tkerr(crtTk, "Missing identifier after comma");
                    }
                }
                else
                    break;
            }

            if(consume(SEMICOLON)){
                return 1;
            }
            else{
                tkerr(crtTk, "Missing semicolon after declaration");
            }

        }
        else{
            crtTk = startTk;
            return 0;
        }

    }
    crtTk = startTk;
    return 0;
}


int typeBase(Type& ret){
    print("TypeBase");

    Token* startTk = crtTk;

    if(consume(INT)){
        ret.typeBase = TB_INT;
        return 1;
    }
    if(consume(DOUBLE)){
        ret.typeBase = TB_DOUBLE;
        return 1;
    }
    if(consume(CHAR)){
        ret.typeBase = TB_CHAR;
        return 1;
    }
    if(consume(STRUCT)){
        Token* tkName = crtTk;
        if(consume(ID)){
            Symbol *s=findSymbol(symbols,tkName->text);
            if(s==NULL) tkerr(crtTk,"TypeBase undefined symbol: %s",tkName->text);
            if(s->cls!=CLS_STRUCT) tkerr(crtTk,"%s is not a struct",tkName->text);
            ret.typeBase=TB_STRUCT;
            ret.s=s;
            return 1;
        }
        tkerr(crtTk, "Error typeBase");
    }
    crtTk = startTk;
    return 0;
}

int arrayDecl(Type& ret){
    print("arrayDecl\n");

    Token* startTk = crtTk;
    Instr* startInstr = lastInstruction;
    RetVal rv;

    Instr* instrBeforeExpr;

    if(consume(LBRACKET)){
        instrBeforeExpr = lastInstruction;

        if(expr(rv)){
            if(!rv.isCtVal)tkerr(crtTk,"the array size is not a constant");
            if(rv.type.typeBase!=TB_INT)tkerr(crtTk,"the array size is not an integer");
            ret.nElements=rv.ctVal.i;
            ret.nElements = 0;
        }
        deleteInstructionsAfter(instrBeforeExpr);

        if(consume(RBRACKET)){
            return 1;
        }
        else{
            tkerr(crtTk, "Missing right bracket");
        }

        success("arrayDecl\n");
        return 1;
    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(startInstr);
        return 0;
    }

    return 0;
}

int typeName(Type& ret){
    print("TypeName");

    Token* startTk = crtTk;

    if(typeBase(ret)){
        if(arrayDecl(ret)){

        }
        else{
            ret.nElements = -1;
        }
        return 1;
    }
    else{
        crtTk = startTk;
        return 0;
    }
}


int declFunc(){
    print("declFunc\n");

    Token *startTk = crtTk;
    Instr* startInstr = lastInstruction;

    Type t;
    vector<Symbol*> :: iterator ps;

    if(typeBase(t)){
        if(consume(MUL)){
            t.nElements = 0;
        }
        else{
            t.nElements = -1;
        }
    }
    else if(consume(VOID)){
        t.typeBase = TB_VOID;
    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(startInstr);
        return 0;
    }

    Token* tkName = crtTk;

    if(consume(ID)){
        sizeArgs = offset = 0;
        if(consume(LPAR)){
            if(findSymbol(symbols,tkName->text))
                tkerr(crtTk,"declFunc symbol redefinition: %s",tkName->text);

            crtFunc=addSymbol(symbols,tkName->text,CLS_FUNC);
            initSymbols(crtFunc->args);
            crtFunc->type=t;
            crtDepth++;

            if(funcArg()){
                for(;;){
                    if(consume(COMMA)){
                        if(funcArg()){

                        }
                        else{
                            tkerr(crtTk, "declFunc Function arg expected after comma");
                        }
                    }
                    else
                        break;
                }
            }
            if(consume(RPAR)){
                crtDepth--;

                crtFunc->addr=addInstr(O_ENTER);
                sizeArgs=offset;
                //update args offsets for correct FP indexing
                for(ps=symbols.begin();ps!=symbols.end();ps++){
                    if((*ps)->mem==MEM_ARG){
                            //2*sizeof(void*) == sizeof(retAddr)+sizeof(FP)
                            (*ps)->offset-=sizeArgs+2*sizeof(void*);
                            }
                    }
                offset=0;

                if(stmCompound()){
                    deleteSymbolsAfter(symbols, crtFunc);

                    ((Instr*)crtFunc->addr)->args[0].i=offset;  // setup the ENTER argument
                    if(crtFunc->type.typeBase==TB_VOID){
                        addInstrII(O_RET,sizeArgs,0);
                    }

                    crtFunc = NULL;
                    return 1;
                }
                else{
                    tkerr(crtTk, "declFunc Expected statement body ");
                }
            }
            else{
                tkerr(crtTk, "declFunc Expected ) ");
            }
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 0;
        }
    }
    else{
        crtTk = startTk;
        return 0;
    }

    return 1;
}

///funcArg: typeBase ID arrayDecl? ;
int funcArg(){
    Token *startTk = crtTk;
    Instr* startInstr = lastInstruction;

    Type t;

    if(typeBase(t)){
        Token* tkName = crtTk;
        if(consume(ID)){
            if(arrayDecl(t)){

            }
            else{
                t.nElements = -1;
            }

            Symbol  *s=addSymbol(symbols,tkName->text,CLS_VAR);
            s->mem=MEM_ARG;
            s->type=t;
            s->offset=offset;

            s=addSymbol(crtFunc->args,tkName->text,CLS_VAR);
            s->mem=MEM_ARG;
            s->type=t;
            s->offset=offset;

            offset+=typeArgSize(&s->type);

            return 1;
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 0;
        }

    }

    crtTk = startTk;
    deleteInstructionsAfter(startInstr);
    return 0;

}


int stm(){
    print("stm\n");
    Token *startTk = crtTk;
    RetVal rv, rv1, rv2, rv3;
    Instr *i,*i1,*i2,*i3,*i4,*is,*ib3,*ibs;

    if(stmCompound()){
        return 1;
    } else {crtTk = startTk;}

    if(ruleIf(rv, i1, i2)){
        print("RULE IF\n");
        return 1;
    } else {crtTk = startTk;}

    if(ruleWhile(rv, i1, i2)){
        print("RULE WHILE\n");
        return 1;
    } else {crtTk = startTk;}

    if(ruleFor()){
        print("RULE FOR\n");
        return 1;
    } else {crtTk = startTk;}

    if(ruleBreak()){
        print("RULE BREAK\n");
        return 1;
    } else {crtTk = startTk;}

    if(ruleReturn(i)){
        print("RULE RETURN\n");
        return 1;
    } else {crtTk = startTk;}


    if(expr(rv1)){
        if(typeArgSize(&rv1.type))addInstrI(O_DROP,typeArgSize(&rv1.type));
    }
    if(consume(SEMICOLON)) return 1;

    crtTk = startTk;

    return 0;
}

int stmCompound(){
    print("stmCompound\n");

    Token *startTk = crtTk;
    Symbol* start = symbols.back();

    if(consume(LACC)){
        crtDepth++;
        for(;;){
            if(declVar()){
            }
            else if(stm()){
            }
            else break;
        }

        if(consume(RACC)){
            crtDepth--;
            deleteSymbolsAfter(symbols, start);
            return 1;
        }
        else{
            crtTk = startTk;
            return 0;
        }
    }
    else{
        crtTk = startTk;
        return 0;
    }

    return 0;
}

int ruleIf(RetVal& rv, Instr*& i1, Instr*& i2){
    print("ruleIf\n");

    Token *startTk = crtTk;

    if(consume(IF)){
        if(consume(LPAR)){
            if(expr(rv)){
                if(rv.type.typeBase==TB_STRUCT)
                    tkerr(crtTk,"a structure cannot be logically tested");

                if(consume(RPAR)){
                    i1 = createCondJmp(&rv);

                    if(stm()){
                        if(consume(ELSE)){
                            i2 = addInstr(O_JMP);
                            if(stm()){
                                i1 -> args[0].addr = i2 -> next;
                                i1 = i2;
                                //return 1;
                            }
                            else{
                                tkerr(crtTk, "Invalid ELSE statement");
                            }
                        }
                        i1->args[0].addr=addInstr(O_NOP);
                        return 1;
                    }
                    else{
                        tkerr(crtTk, "Missing or invalid statement");
                    }
                }
                else{
                    tkerr(crtTk, "Missing right paranthesis");
                }
            }
            else{
                tkerr(crtTk, "Invalid expression in IF statement");
            }
        }
        else{
            tkerr(crtTk, "Expected left paranthesis");
        }

    }
    else{
        crtTk = startTk;
        return 0;
    }

    return 1;
}

int ruleWhile(RetVal& rv,  Instr*& i1, Instr*& i2){
    Token *startTk = crtTk;
    Instr* startInstr = lastInstruction;

    print("ruleWhile\n");

    if(consume(WHILE)){

        Instr *oldLoopEnd=crtLoopEnd;
        crtLoopEnd=createInstr(O_NOP);
        i1=lastInstruction;

        if(consume(LPAR)){
            if(expr(rv)){
                if(rv.type.typeBase==TB_STRUCT)
                    tkerr(crtTk,"a structure cannot be logically tested");

                if(consume(RPAR)){
                    i2=createCondJmp(&rv);
                    if(stm()){
                        addInstrA(O_JMP,i1->next);
                        appendInstr(crtLoopEnd);
                        i2->args[0].addr=crtLoopEnd;
                        crtLoopEnd=oldLoopEnd;

                        return 1;
                    }
                    else{
                        tkerr(crtTk,"missing while statement");
                    }
                }
                else{
                    tkerr(crtTk,"missing )");
                }
            }
            else{
                tkerr(crtTk,"invalid expression after (");
            }
        }
        else{
            tkerr(crtTk,"missing ( after while");
        }
    }

    crtTk=startTk;
    deleteInstructionsAfter(startInstr);
    return 0;
}

int ruleFor(){
    print("ruleFor\n");

    Token *startTk = crtTk;
    RetVal rv1, rv2, rv3;

    if(consume(FOR)){
        if(consume(LPAR)){
            if(expr(rv1)){

            }
            if(consume(SEMICOLON)){
                if(expr(rv2)){
                    if(rv2.type.typeBase==TB_STRUCT)
                        tkerr(crtTk,"a structure cannot be logically tested");
                }

                if(consume(SEMICOLON)){
                    if(expr(rv3)){

                    }
                    if(consume(RPAR)){
                        if(stm()){
                            return 1;
                        }
                        else{
                            tkerr(crtTk, "Invalid FOR statement");
                        }
                    }
                    else{
                        tkerr(crtTk, "missing ) in FOR statement");
                    }
                }
                else{
                    tkerr(crtTk, "missing ; in FOR statement");
                }

            }
            else{
                tkerr(crtTk, "missing ; in FOR statement");
            }
        }
        else{
            tkerr(crtTk, "missing ( in FOR statement");
        }

    }

    crtTk = startTk;
    return 0;
}

int ruleBreak(){
    print("ruleBreak\n");

    Token *startTk = crtTk;

    if(consume(BREAK)){
        if(!consume(SEMICOLON)) tkerr(crtTk,"missing ; after break");
        success("ruleBreak\n");
        return 1;
    }

    return 0;
}

int ruleReturn(Instr* &i){
    print("ruleReturn\n");

    Token *startTk = crtTk;
    Instr* startInstr = lastInstruction;
    RetVal rv;


    if(consume(RETURN)){
        if(expr(rv)){
            i=getRVal(&rv);
            addCastInstr(i,&rv.type,&crtFunc->type);

            if(crtFunc->type.typeBase==TB_VOID)
                tkerr(crtTk,"a void function cannot return a value");
            cast(&crtFunc->type,&rv.type);
        }

        if(consume(SEMICOLON)){
            if(crtFunc->type.typeBase==TB_VOID){
                addInstrII(O_RET,sizeArgs,0);
            }else{
                addInstrII(O_RET,sizeArgs,typeArgSize(&crtFunc->type));
            }

            return 1;
        }
        else{
            tkerr(crtTk, "Expected semicolon");
        }
    }

    crtTk = startTk;
    deleteInstructionsAfter(startInstr);
    return 0;
}

/// expr: exprAssign
int expr(RetVal& rv){
    Token* startTk = crtTk;

    if(exprAssign(rv)){
        return 1;
    }

    else{
        crtTk = startTk;
        return 0;
    }
}

/// exprAssign: exprUnary ASSIGN exprAssign | exprOr
int exprAssign(RetVal& rv){
    print("exprAssign\n");

    Token *startTk = crtTk;
    RetVal rve;
    Instr* startInstr = lastInstruction;
    Instr *i,*oldLastInstr=lastInstruction;

    if(exprUnary(rv)){
        if(consume(ASSIGN)){
            if(exprAssign(rve)){
                if(!rv.isLVal) tkerr(crtTk,"cannot assign to a non-lval");
                if(rv.type.nElements>-1||rve.type.nElements>-1)
                    tkerr(crtTk,"the arrays cannot be assigned");

                cast(&rv.type,&rve.type);
                i=getRVal(&rve);
                addCastInstr(i,&rve.type,&rv.type);
                //duplicate the value on top before the dst addr
                addInstrII(O_INSERT, sizeof(void*)+typeArgSize(&rv.type), typeArgSize(&rv.type));
                addInstrI(O_STORE,typeArgSize(&rv.type));
                rv.isCtVal=rv.isLVal=0;

                return 1;
            }
            else{
                crtTk = startTk;
                deleteInstructionsAfter(oldLastInstr);
            }
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(oldLastInstr);
        }
    }

    crtTk = startTk;
    deleteInstructionsAfter(oldLastInstr);
    if(exprOr(rv)){
        success("exprAssign\n");
        return 1;
    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(oldLastInstr);
        return 0;
    }
    return 0;
}



/// exprOr: exprOr OR exprAnd | exprAnd =>
/// => exprOr: exprAnd exprOr1; exprOr1: OR exprAnd exprOr1 | e
int exprOr(RetVal& rv){
    print("exprOr\n");
    Token *startTk = crtTk;
    Instr* startInstr = lastInstruction;

    if(exprAnd(rv)){
        if(exprOr1(rv)){
            return 1;
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 0;
        }
    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(startInstr);
        return 0;
    }


    return 0;
}
int exprOr1(RetVal& rv){
    print("exprOr1\n");

    Token *startTk = crtTk;
    RetVal rve;
    Instr *i1,*i2;Type t,t1,t2;
    Instr* startInstr = lastInstruction;

    if(consume(OR)){

        i1=rv.type.nElements<0?getRVal(&rv):lastInstruction;
        t1=rv.type;

        if(exprAnd(rve)){
            if(rv.type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                    tkerr(crtTk,"a structure cannot be logically tested");

            if(rv.type.nElements>=0){      // vectors
                addInstr(O_OR_A);
            }else{  // non-vectors
                i2=getRVal(&rve);t2=rve.type;
                t=getArithType(&t1,&t2);
                addCastInstr(i1,&t1,&t);
                addCastInstr(i2,&t2,&t);
                switch(t.typeBase){
                    case TB_INT:addInstr(O_OR_I);break;
                    case TB_DOUBLE:addInstr(O_OR_D);break;
                    case TB_CHAR:addInstr(O_OR_C);break;
                }
            }

            rv.type=createType(TB_INT,-1);
            rv.isCtVal=rv.isLVal=0;

            if(exprOr1(rv)){
                return 1;
            }
            else{
                crtTk = startTk;
                deleteInstructionsAfter(startInstr);
                return 1;
            }
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 1;
        }
    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(startInstr);
        return 1;
    }

    success("exprOr1\n");
    return 1;
}



/// exprAnd: exprAnd AND exprEq | exprEq ;
/// exprAnd: exprEq exprAnd1; exprAnd1: (AND exprEq exprAnd1)?
int exprAnd(RetVal& rv){
    print("exprAnd\n");

    Token *startTk = crtTk;
    Instr* startInstr = lastInstruction;

    if(exprEq(rv)){
        if(exprAnd1(rv)){
            return 1;
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 0;
        }
    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(startInstr);
        return 0;
    }


    return 0;
}
int exprAnd1(RetVal& rv){
    print("exprAnd1\n");
    Instr *i1,*i2;Type t,t1,t2;
    Token *startTk = crtTk;
    RetVal rve;
    Instr* startInstr = lastInstruction;

    if(consume(AND)){

        i1=rv.type.nElements<0?getRVal(&rv):lastInstruction;
        t1=rv.type;

        if(exprEq(rve)){
            if(rv.type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be logically tested");

            if(rv.type.nElements>=0){      // vectors
                addInstr(O_AND_A);
             }
             else{  // non-vectors
                i2=getRVal(&rve);t2=rve.type;
                t=getArithType(&t1,&t2);
                addCastInstr(i1,&t1,&t);
                addCastInstr(i2,&t2,&t);
                switch(t.typeBase){
                        case TB_INT:addInstr(O_AND_I);break;
                        case TB_DOUBLE:addInstr(O_AND_D);break;
                        case TB_CHAR:addInstr(O_AND_C);break;
                        }
            }

            rv.type=createType(TB_INT,-1);
            rv.isCtVal=rv.isLVal=0;

            if(exprAnd1(rv)){
                return 1;
            }
            else{
                crtTk = startTk;
                deleteInstructionsAfter(startInstr);
                return 1;
            }
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 1;
        }
        success("exprAnd1\n");
        return 1;
    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(startInstr);
        return 1;
    }
    success("exprAnd1\n");
    return 1;
}



/// exprEq: exprEq ( EQUAL | NOTEQ ) exprRel | exprRel
/// exprEq: exprRel exprEq1; exprEq1: ( EQUAL | NOTEQ ) exprRel exprEq1
int exprEq(RetVal& rv){
    print("exprEq\n");

    Token *startTk = crtTk;
    Instr* startInstr = lastInstruction;

    if(exprRel(rv)){
        if(exprEq1(rv)){
            return 1;
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 0;
        }
    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(startInstr);
        return 0;
    }

    return 0;
}
int exprEq1(RetVal& rv){
    print("exprEq1\n");

    Instr *i1,*i2;Type t,t1,t2;
    Token *startTk = crtTk;
    RetVal rve;
    Token* tkop = crtTk;
    Instr* startInstr = lastInstruction;

    if(consume(EQUAL) || consume(NOTEQ)){

        i1=rv.type.nElements<0?getRVal(&rv):lastInstruction;
        t1=rv.type;

        if(exprRel(rve)){
            if(rv.type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                    tkerr(crtTk,"a structure cannot be compared");

            if(rv.type.nElements>=0){      // vectors
                addInstr(tkop->code==EQUAL?O_EQ_A:O_NOTEQ_A);
            }
            else{  // non-vectors
                i2=getRVal(&rve);t2=rve.type;
                t=getArithType(&t1,&t2);
                addCastInstr(i1,&t1,&t);
                addCastInstr(i2,&t2,&t);
                if(tkop->code==EQUAL){
                        switch(t.typeBase){
                                case TB_INT:addInstr(O_EQ_I);break;
                                case TB_DOUBLE:addInstr(O_EQ_D);break;
                                case TB_CHAR:addInstr(O_EQ_C);break;
                                }
                        }else{
                        switch(t.typeBase){
                                case TB_INT:addInstr(O_NOTEQ_I);break;
                                case TB_DOUBLE:addInstr(O_NOTEQ_D);break;
                                case TB_CHAR:addInstr(O_NOTEQ_C);break;
                                }
                        }
            }

            rv.type=createType(TB_INT,-1);
            rv.isCtVal=rv.isLVal=0;

            if(exprEq1(rv)){
                return 1;
            }
            else{
                crtTk = startTk;
                deleteInstructionsAfter(startInstr);
                return 1;
            }
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 1;
        }
    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(startInstr);
        return 1;
    }

    success("exprEq1\n");
    return 1;
}



///exprRel: exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd ;
///exprRel: exprAdd exprRel1; exprRel1:( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd exprRel1
int exprRel(RetVal& rv){
    print("exprRel\n");

    Token *startTk = crtTk;
    Instr* startInstr = lastInstruction;

    if(exprAdd(rv)){
        if(exprRel1(rv)){
            return 1;
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 0;
        }
    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(startInstr);
        return 0;
    }

    return 0;
}
int exprRel1(RetVal& rv){
    print("exprRel1\n");

    Instr *i1,*i2;Type t,t1,t2;
    Instr* startInstr = lastInstruction;
    Token *startTk = crtTk;
    RetVal rve;
    Token* tkop = crtTk;

    if(consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)){

        i1=getRVal(&rv);t1=rv.type;

        if(exprAdd(rve)){
            if(rv.type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be compared");
            if(rv.type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be compared");

            i2=getRVal(&rve);t2=rve.type;
            t=getArithType(&t1,&t2);
            addCastInstr(i1,&t1,&t);
            addCastInstr(i2,&t2,&t);


            switch(tkop->code){
                case LESS:
                        switch(t.typeBase){
                                case TB_INT:addInstr(O_LESS_I);break;
                                case TB_DOUBLE:addInstr(O_LESS_D);break;
                                case TB_CHAR:addInstr(O_LESS_C);break;
                                }
                        break;
                case LESSEQ:
                        switch(t.typeBase){
                                case TB_INT:addInstr(O_LESSEQ_I);break;
                                case TB_DOUBLE:addInstr(O_LESSEQ_D);break;
                                case TB_CHAR:addInstr(O_LESSEQ_C);break;
                                }
                        break;
                case GREATER:
                        switch(t.typeBase){
                                case TB_INT:addInstr(O_GREATER_I);break;
                                case TB_DOUBLE:addInstr(O_GREATER_D);break;
                                case TB_CHAR:addInstr(O_GREATER_C);break;
                                }
                        break;
                case GREATEREQ:
                        switch(t.typeBase){
                                case TB_INT:addInstr(O_GREATEREQ_I);break;
                                case TB_DOUBLE:addInstr(O_GREATEREQ_D);break;
                                case TB_CHAR:addInstr(O_GREATEREQ_C);break;
                                }
                        break;
            }

            rv.type=createType(TB_INT,-1);
            rv.isCtVal=rv.isLVal=0;

            if(exprRel1(rv)){
                return 1;
            }
            else{
                crtTk = startTk;
                deleteInstructionsAfter(startInstr);
                return 1;
            }

        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 1;
        }
    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(startInstr);
        return 1;
    }

    success("exprRel1\n");
    return 1;
}



/// exprAdd: exprAdd ( ADD | SUB ) exprMul | exprMul ;
/// exprAdd: exprMul exprAdd1; exprAdd1: ( ADD | SUB ) exprMul exprAdd1
int exprAdd(RetVal& rv){
    print("exprAdd\n");

    Token *startTk = crtTk;
    Instr* startInstr = lastInstruction;

    if(exprMul(rv)){
        if(exprAdd1(rv)){
            return 1;
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 0;
        }
    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(startInstr);
        return 0;
    }
}
int exprAdd1(RetVal& rv){
    print("exprAdd1\n");

    Token *startTk = crtTk;
    Instr *i1,*i2;Type t,t1,t2;
    Instr* startInstr = lastInstruction;
    Token* tkop = crtTk;
    RetVal rve;

    if(consume(ADD) || consume(SUB)){

        i1=getRVal(&rv);t1=rv.type;

        if(exprMul(rve)){
            if(rv.type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be added or subtracted");
            if(rv.type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be added or subtracted");

            rv.type=getArithType(&rv.type,&rve.type);
            i2=getRVal(&rve);t2=rve.type;
            addCastInstr(i1,&t1,&rv.type);
            addCastInstr(i2,&t2,&rv.type);
            if(tkop->code==ADD){
                switch(rv.type.typeBase){
                    case TB_INT:addInstr(O_ADD_I);break;
                    case TB_DOUBLE:addInstr(O_ADD_D);break;
                    case TB_CHAR:addInstr(O_ADD_C);break;
                }
            }
            else{
                switch(rv.type.typeBase){
                    case TB_INT:addInstr(O_SUB_I);break;
                    case TB_DOUBLE:addInstr(O_SUB_D);break;
                    case TB_CHAR:addInstr(O_SUB_C);break;
                }
            }

            rv.isCtVal=rv.isLVal=0;

            if(exprAdd1(rv)){
                return 1;
            }
            else{
                crtTk = startTk;
                deleteInstructionsAfter(startInstr);
                return 1;
            }
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 1;
        }

    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(startInstr);
        return 1;
    }

    success("exprAdd1\n");
    return 1;
}



/// exprMul: exprMul ( MUL | DIV ) exprCast | exprCast ;
/// exprMul: exprCast exprMul1; exprMul1:( MUL | DIV ) exprCast exprMul1
int exprMul(RetVal& rv){
    print("exprMul\n");

    Token *startTk = crtTk;
    Instr* startInstr = lastInstruction;

    if(exprCast(rv)){
        if(exprMul1(rv)){
            return 1;
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 0;
        }
    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(startInstr);
        return 0;
    }
}
int exprMul1(RetVal& rv){
    print("exprMul1\n");

    Token *startTk = crtTk;
    Instr* startInstr = lastInstruction;
    Instr *i1,*i2;Type t1,t2;
    RetVal rve;
    Token* tkop = crtTk;

    if(consume(MUL) || consume(DIV)){

        i1=getRVal(&rv);t1=rv.type;

        if(exprCast(rve)){
            if(rv.type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be multiplied or divided");
            if(rv.type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be multiplied or divided");
            rv.type=getArithType(&rv.type,&rve.type);

            i2=getRVal(&rve);t2=rve.type;
            addCastInstr(i1,&t1,&rv.type);
            addCastInstr(i2,&t2,&rv.type);

            if(tkop->code==MUL){
                switch(rv.type.typeBase){
                    case TB_INT:addInstr(O_MUL_I);break;
                    case TB_DOUBLE:addInstr(O_MUL_D);break;
                    case TB_CHAR:addInstr(O_MUL_C);break;
                }
            }
            else{
                switch(rv.type.typeBase){
                    case TB_INT:addInstr(O_DIV_I);break;
                    case TB_DOUBLE:addInstr(O_DIV_D);break;
                    case TB_CHAR:addInstr(O_DIV_C);break;
                }
            }

            rv.isCtVal=rv.isLVal=0;

            if(exprMul1(rv)){
                return 1;
            }
            else{
                crtTk = startTk;
                deleteInstructionsAfter(startInstr);
                return 1;
            }
        }
        else{
                crtTk = startTk;
                deleteInstructionsAfter(startInstr);
                return 1;
        }
    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(startInstr);
        return 1;
    }

    return 1;
}



/// exprCast: LPAR typeName RPAR exprCast | exprUnary ;
int exprCast(RetVal& rv){
    print("exprCast\n");

    Token *startTk = crtTk;
    Type t;
    RetVal rve;
    Instr* startInstr = lastInstruction;
    Instr *oldLastInstr=lastInstruction;

    if(consume(LPAR)){
        if(typeName(t)){
            if(consume(RPAR)){
                if(exprCast(rve)){
                    cast(&t,&rve.type);

                    if(rv.type.nElements<0&&rv.type.typeBase!=TB_STRUCT){
                        switch(rve.type.typeBase){
                            case TB_CHAR:
                                    switch(t.typeBase){
                                            case TB_INT:addInstr(O_CAST_C_I);break;
                                            case TB_DOUBLE:addInstr(O_CAST_C_D);break;
                                            }
                                    break;
                            case TB_DOUBLE:
                                    switch(t.typeBase){
                                            case TB_CHAR:addInstr(O_CAST_D_C);break;
                                            case TB_INT:addInstr(O_CAST_D_I);break;
                                            }
                                    break;
                            case TB_INT:
                                    switch(t.typeBase){
                                            case TB_CHAR:addInstr(O_CAST_I_C);break;
                                            case TB_DOUBLE:addInstr(O_CAST_I_D);break;
                                            }
                                    break;
                            }
                    }


                    rv.type=t;
                    rv.isCtVal=rv.isLVal=0;
                    return 1;
                }
                else{
                    crtTk = startTk;
                    deleteInstructionsAfter(startInstr);
                    return 0;
                }
            }
            else{
                crtTk = startTk;
                deleteInstructionsAfter(startInstr);
                return 0;
            }
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 0;
        }
    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(oldLastInstr);
        if(exprUnary(rv)){
            return 1;
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 0;
        }
    }

    return 0;
}



/// exprUnary: ( SUB | NOT ) exprUnary | exprPostfix ;
int exprUnary(RetVal& rv){
    print("exprUnary\n");

    Token *startTk = crtTk;
    Token* tkop = crtTk;
    Instr* startInstr = lastInstruction;

    if(consume(SUB) || consume(NOT)){
        if(exprUnary(rv)){
            if(tkop->code==SUB){
                if(rv.type.nElements>=0) tkerr(crtTk,"unary '-' cannot be applied to an array");
                if(rv.type.typeBase==TB_STRUCT) tkerr(crtTk,"unary '-' cannot be applied to a struct");

                getRVal(&rv);
                switch(rv.type.typeBase){
                    case TB_CHAR:addInstr(O_NEG_C);break;
                    case TB_INT:addInstr(O_NEG_I);break;
                    case TB_DOUBLE:addInstr(O_NEG_D);break;
                }

            }
            else{  // NOT
                if(rv.type.typeBase==TB_STRUCT) tkerr(crtTk,"'!' cannot be applied to a struct");

                if(rv.type.nElements<0){
                    getRVal(&rv);
                    switch(rv.type.typeBase){
                            case TB_CHAR:addInstr(O_NOT_C);break;
                            case TB_INT:addInstr(O_NOT_I);break;
                            case TB_DOUBLE:addInstr(O_NOT_D);break;
                    }
                }
                else
                    addInstr(O_NOT_A);

                rv.type=createType(TB_INT,-1);
            }
            rv.isCtVal=rv.isLVal=0;

            success("exprUnary\n");
            return 1;
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 0;
        }
    }
    else if(exprPostfix(rv)){
        return 1;
    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(startInstr);
        return 0;
    }

    return 0;
}



/// exprPostfix: exprPostfix LBRACKET expr RBRACKET | exprPostfix DOT ID | exprPrimary ;
/// exprPostfix: exprPrimary exprPostfix1;
/// exprPostfix1: LBRACKET expr RBRACKET exprPostfix1 | DOT ID exprPostfix1 | e
int exprPostfix(RetVal& rv){
    print("exprPostfix\n");

    Token *startTk = crtTk;
    Instr* startInstr = lastInstruction;

    if(exprPrimary(rv)){
        if(exprPostfix1(rv)){
            return 1;
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 0;
        }
    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(startInstr);
        return 0;
    }
}
int exprPostfix1(RetVal& rv){
    print("exprPostfix1\n");

    Token *startTk = crtTk;
    Instr* startInstr = lastInstruction;
    RetVal rve;

    if(consume(LBRACKET)){
        if(expr(rve)){
            if(rv.type.nElements<0)tkerr(crtTk,"only an array can be indexed");
            Type typeInt=createType(TB_INT,-1);
            cast(&typeInt,&rve.type);
            rv.type=rv.type;
            rv.type.nElements=-1;
            rv.isLVal=1;
            rv.isCtVal=0;

            if(consume(RBRACKET)){
                addCastInstr(lastInstruction,&rve.type,&typeInt);
                getRVal(&rve);
                if(typeBaseSize(&rv.type)!=1){
                        addInstrI(O_PUSHCT_I,typeBaseSize(&rv.type));
                        addInstr(O_MUL_I);
                        }
                addInstr(O_OFFSET);

                if(exprPostfix1(rv)){
                    return 1;
                }
                else{
                    crtTk = startTk;
                    deleteInstructionsAfter(startInstr);
                    return 1;
                }

            }
            else{
                crtTk = startTk;
                deleteInstructionsAfter(startInstr);
                return 1;
            }
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 1;
        }
        success("exprPostfix\n");
        return 1;
    }
    else if(consume(DOT)){
        Token* tkName = crtTk;
        if(consume(ID)){
            Symbol      *sStruct=rv.type.s;
            Symbol      *sMember=findSymbol(sStruct->members,tkName->text);

            if(!sMember)
                tkerr(crtTk,"struct %s does not have a member %s",sStruct->name,tkName->text);

            rv.type=sMember->type;
            rv.isLVal=1;
            rv.isCtVal=0;

            if(sMember->offset){
                addInstrI(O_PUSHCT_I,sMember->offset);
                addInstr(O_OFFSET);
            }

            if(exprPostfix1(rv)){
                return 1;
            }
            else{
                crtTk = startTk;
                deleteInstructionsAfter(startInstr);
                return 1;
            }
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 1;
        }
    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(startInstr);
        return 1;
    }

    success("exprPostfix1\n");
    return 1;
}



///exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
///           | CT_INT
///           | CT_REAL
///           | CT_CHAR
///           | CT_STRING
///           | LPAR expr RPAR ;

int exprPrimary(RetVal &rv){
    print("exprPrimary\n");

    Token *startTk = crtTk;
    Instr* startInstr = lastInstruction;
    Instr *i;
    Token* tkName = crtTk;
    RetVal arg;

    if(consume(ID)){
        Symbol *s=findSymbol(symbols,tkName->text);

        if(!s) tkerr(crtTk,"Expr Primary undefined symbol %s",tkName->text);
        rv.type=s->type;
        rv.isCtVal=0;
        rv.isLVal=1;

        if(consume(LPAR)){

            vector <Symbol*> :: iterator crtDefArg=s->args.begin();
            if(s->cls!=CLS_FUNC&&s->cls!=CLS_EXTFUNC)
                tkerr(crtTk,"call of the non-function %s",tkName->text);

            if(expr(arg)){

                if(crtDefArg==s->args.end())tkerr(crtTk,"too many arguments in call");
                cast(&((*crtDefArg)->type),&arg.type);

                if((*crtDefArg)->type.nElements<0){  //only arrays are passed by addr
                    i=getRVal(&arg);
                }
                else{
                    i=lastInstruction;
                }
                addCastInstr(i,&arg.type,&(*crtDefArg)->type);

                crtDefArg++;

                for(;;){
                    if(consume(COMMA)){
                        if(expr(arg)){

                            if(crtDefArg==s->args.end())tkerr(crtTk,"too many arguments in call");
                            cast(&((*crtDefArg)->type),&arg.type);

                            if((*crtDefArg)->type.nElements<0){
                                i=getRVal(&arg);
                            }
                            else{
                                i=lastInstruction;
                            }
                            addCastInstr(i,&arg.type,&(*crtDefArg)->type);

                            crtDefArg++;

                        }

                    }
                    else break;
                }
            }

            if(consume(RPAR)){

                i=addInstr(s->cls==CLS_FUNC?O_CALL:O_CALLEXT);
                i->args[0].addr=s->addr;

                if(crtDefArg!=s->args.end())tkerr(crtTk,"too few arguments in call");
                rv.type=s->type;
                rv.isCtVal=rv.isLVal=0;

                success("exprPrimary\n");
                return 1;
            }
            else{
                if(s->cls==CLS_FUNC||s->cls==CLS_EXTFUNC) tkerr(crtTk,"missing call for function %s",tkName->text);
            }
        }
        else{
            if(s->depth){
                addInstrI(O_PUSHFPADDR,s->offset);
            }
            else{
                addInstrA(O_PUSHCT_A,s->addr);
            }
        }
        return 1;
    }


    Token* tki = crtTk;
    if(consume(CT_INT)){
        //Token* tki = crtTk;

        addInstrI(O_PUSHCT_I,tki->i);

        rv.type=createType(TB_INT,-1);
        rv.ctVal.i=tki->i;

        printf("AAAAAAAAAAAAAAA %ld\n\n", crtTk->i);

        rv.isCtVal=1;
        rv.isLVal=0;

        return 1;
    }

    Token* tkr = crtTk;
    if(consume(CT_REAL)){
        i=addInstr(O_PUSHCT_D);
        i->args[0].d=tkr->r;

        rv.type=createType(TB_DOUBLE,-1);
        rv.ctVal.d=tkr->r;
        rv.isCtVal=1;
        rv.isLVal=0;

        return 1;
    }

    Token* tkc = crtTk;
    if(consume(CT_CHAR)){
        addInstrI(O_PUSHCT_C,tkc->i);

        rv.type=createType(TB_CHAR,-1);
        rv.ctVal.i=tkc->i;
        rv.isCtVal=1;
        rv.isLVal=0;

        return 1;
    }

    Token* tks = crtTk;
    if(consume(CT_STRING)){
        addInstrA(O_PUSHCT_A,tks->text);

        rv.type=createType(TB_CHAR,0);
        rv.ctVal.str=tks->text;
        rv.isCtVal=1;
        rv.isLVal=0;

        return 1;
    }

    if(consume(LPAR)){
        if(expr(rv)){
            if(consume(RPAR)){
                return 1;
            }
            else{
                tkerr(crtTk, "Missing right paranthesis");
            }
        }
        else{
            crtTk = startTk;
            deleteInstructionsAfter(startInstr);
            return 0;
        }
    }
    else{
        crtTk = startTk;
        deleteInstructionsAfter(startInstr);
        return 0;
    }

    return 0;
}
