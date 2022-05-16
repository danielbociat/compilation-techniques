#include <bits/stdc++.h>
#include "symbol.h"

using namespace std;

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
string ct_str;
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
                    pCrtCh++;
                    state = 24;
                }
                break;
            }

            case 23:{
                switch(ch){
                    case 'a':{
                        pCrtCh++;
                        state = 24;
                        break;
                    }
                    case 'b':{
                        pCrtCh++;
                        state = 24;
                        break;
                    }
                    case 'f':{
                        pCrtCh++;
                        state = 24;
                        break;
                    }
                    case 'n':{
                        pCrtCh++;
                        state = 24;
                        break;
                    }
                    case 'r':{
                        pCrtCh++;
                        state = 24;
                        break;
                    }
                    case 't':{
                        pCrtCh++;
                        state = 24;
                        break;
                    }
                    case 'v':{
                        pCrtCh++;
                        state = 24;
                        break;
                    }
                    case '\'':{
                        pCrtCh++;
                        state = 24;
                        break;
                    }
                    case '?':{
                        pCrtCh++;
                        state = 24;
                        break;
                    }
                    case '\"':{
                        pCrtCh++;
                        state = 24;
                        break;
                    }
                    case '\\':{
                        pCrtCh++;
                        state = 24;
                        break;
                    }
                    case '0':{
                        pCrtCh++;
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


/// Syntactic Analyzer Functions ///
int unit();

int ruleIf(RetVal& rv);
int ruleWhile(RetVal& rv);
int ruleFor();
int ruleBreak();
int ruleReturn();

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
    Symbol *s = new Symbol();

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




Type createType(int typeBase, int nElements){
    Type t;
    t.typeBase = typeBase;
    t.nElements = nElements;

    return t;
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
    Symbol *s=addSymbol(symbols,name,CLS_EXTFUNC);
    s->type=type;
    initSymbols(s->args);
    return s;
}
Symbol *addFuncArg(Symbol *func,const char *name,Type type){
    Symbol *a=addSymbol(func->args,name,CLS_VAR);
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


void put_s(char s[]);
void get_s(char s[]);
void put_i(int i);
int get_i();
void put_d(double d);
double get_d();
void put_c(char c);
char get_c();
double seconds();
/// DEBUG FUNCTIONS ///

int DEBUG = 1, SUCCESS = 0;
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
}
/**
void addExtFunctions(){
    Symbol *s;
    s = addExtFunc("put_s", createType(TB_VOID, -1), (void*)put_s);
    addFuncArg(s,"s",createType(TB_CHAR,0));

    s = addExtFunc("put_i", createType(TB_VOID, -1), (void*)put_i);
    addFuncArg(s, "i", createType(TB_INT, -1));

    s = addExtFunc("get_i", createType(TB_INT, -1), (void*)get_i);

    s=addExtFunc("put_d",createType(TB_VOID,-1), (void*)put_d);
    addFuncArg(s,"d",createType(TB_DOUBLE, -1));

    s = addExtFunc("get_d", createType(TB_DOUBLE, -1), (void*)get_d);

    s=addExtFunc("put_c",createType(TB_VOID,-1), (void*)put_c);
    addFuncArg(s,"c",createType(TB_CHAR, -1));

    s = addExtFunc("get_c", createType(TB_CHAR, -1), (void*)get_c);

    s = addExtFunc("seconds", createType(TB_DOUBLE, -1), (void*)seconds);


    s = addExtFunc("get_s", createType(TB_VOID, -1), (void*)get_s);
    addFuncArg(s, "s", createType(TB_CHAR, 0));
}
**/

int main()
{
    //addExtFunctions();

    FILE *f = fopen("0.c", "rb");

    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *text = (char*)malloc(size);

    fread(text,size, 1, f);
    text[size] = 0;
    pCrtCh = text;

    for(;;){
        getNextToken();
        //printf("%s ", codes[lastToken->code]);
        if(lastToken->code == END)
            break;
    }

    printTokens(tokens);

    crtTk = tokens;
    unit();

    fclose(f);
    return 0;
}

int unit(){
    print("unit\n");

    for(;;){
        if(declStruct() || declFunc() || declVar()) {}
        else break;
    }

    if(!consume(END)) tkerr(crtTk, "missing END token in unit");

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
    RetVal rv;

    if(consume(LBRACKET)){
        if(expr(rv)){
            if(!rv.isCtVal)tkerr(crtTk,"the array size is not a constant");
            if(rv.type.typeBase!=TB_INT)tkerr(crtTk,"the array size is not an integer");
            ret.nElements=rv.ctVal.i;
            ret.nElements = 0;
        }
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
    Type t;

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
        return 0;
    }

    Token* tkName = crtTk;

    if(consume(ID)){
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

                cout << "AAAAAA";
                if(stmCompound()){
                    deleteSymbolsAfter(symbols, crtFunc);
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

            s=addSymbol(crtFunc->args,tkName->text,CLS_VAR);
            s->mem=MEM_ARG;
            s->type=t;

            return 1;
        }
        else{
            crtTk = startTk;
            return 0;
        }

    }

    crtTk = startTk;
    return 0;

}


int stm(){
    print("stm\n");
    Token *startTk = crtTk;
    RetVal rv, rv1, rv2, rv3;

    if(stmCompound()){
        return 1;
    } else {crtTk = startTk;}

    if(ruleIf(rv)){
        print("RULE IF\n");
        return 1;
    } else {crtTk = startTk;}

    if(ruleWhile(rv)){
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

    if(ruleReturn()){
        print("RULE RETURN\n");
        return 1;
    } else {crtTk = startTk;}


    if(expr(rv1)){}
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

int ruleIf(RetVal& rv){
    print("ruleIf\n");

    Token *startTk = crtTk;

    if(consume(IF)){
        if(consume(LPAR)){
            if(expr(rv)){
                if(rv.type.typeBase==TB_STRUCT)
                    tkerr(crtTk,"a structure cannot be logically tested");

                if(consume(RPAR)){
                    if(stm()){
                        if(consume(ELSE)){
                            if(consume(stm())){
                                return 1;
                            }
                            else{
                                tkerr(crtTk, "Invalid ELSE statement");
                            }
                        }

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

    crtTk = startTk;
    return 0;
}

int ruleWhile(RetVal& rv){
    Token *startTk = crtTk;

    print("ruleWhile\n");

    if(consume(WHILE)){
        if(consume(LPAR)){
            if(expr(rv)){
                if(rv.type.typeBase==TB_STRUCT)
                    tkerr(crtTk,"a structure cannot be logically tested");

                if(consume(RPAR)){
                    if(stm()){
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

int ruleReturn(){
    print("ruleReturn\n");

    Token *startTk = crtTk;
    RetVal rv;


    if(consume(RETURN)){
        if(expr(rv)){
            if(crtFunc->type.typeBase==TB_VOID)
                tkerr(crtTk,"a void function cannot return a value");
            cast(&crtFunc->type,&rv.type);
        }

        if(consume(SEMICOLON)){
            return 1;
        }
        else{
            tkerr(crtTk, "Expected semicolon");
        }
    }

    crtTk = startTk;
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

    if(exprUnary(rv)){
        if(consume(ASSIGN)){
            if(exprAssign(rve)){
                if(!rv.isLVal) tkerr(crtTk,"cannot assign to a non-lval");
                if(rv.type.nElements>-1||rve.type.nElements>-1)
                    tkerr(crtTk,"the arrays cannot be assigned");
                cast(&rv.type,&rve.type);
                rv.isCtVal=rv.isLVal=0;
                return 1;
            }
            else{
                crtTk = startTk;
            }
        }
        else{
            crtTk = startTk;
        }
    }

    crtTk = startTk;
    if(exprOr(rv)){
        success("exprAssign\n");
        return 1;
    }
    else{
        crtTk = startTk;
        return 0;
    }
    return 0;
}



/// exprOr: exprOr OR exprAnd | exprAnd =>
/// => exprOr: exprAnd exprOr1; exprOr1: OR exprAnd exprOr1 | e
int exprOr(RetVal& rv){
    print("exprOr\n");
    Token *startTk = crtTk;

    if(exprAnd(rv)){
        if(exprOr1(rv)){
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
int exprOr1(RetVal& rv){
    print("exprOr1\n");
    Token *startTk = crtTk;
    RetVal rve;

    if(consume(OR)){
        if(exprAnd(rve)){
            if(rv.type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                    tkerr(crtTk,"a structure cannot be logically tested");
            rv.type=createType(TB_INT,-1);
            rv.isCtVal=rv.isLVal=0;

            if(exprOr1(rv)){
                return 1;
            }
            else{
                crtTk = startTk;
                return 1;
            }
        }
        else{
            crtTk = startTk;
            return 1;
        }
    }
    else{
        crtTk = startTk;
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

    if(exprEq(rv)){
        if(exprAnd1(rv)){
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
int exprAnd1(RetVal& rv){
    print("exprAnd1\n");

    Token *startTk = crtTk;
    RetVal rve;

    if(consume(AND)){
        if(exprEq(rve)){
            if(rv.type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be logically tested");
            rv.type=createType(TB_INT,-1);
            rv.isCtVal=rv.isLVal=0;

            if(exprAnd1(rv)){
                return 1;
            }
            else{
                crtTk = startTk;
                return 1;
            }
        }
        else{
            crtTk = startTk;
            return 1;
        }
        success("exprAnd1\n");
        return 1;
    }
    else{
        crtTk = startTk;
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

    if(exprRel(rv)){
        if(exprEq1(rv)){
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
int exprEq1(RetVal& rv){
    print("exprEq1\n");

    Token *startTk = crtTk;
    RetVal rve;
    Token* tkop = crtTk;

    if(consume(EQUAL) || consume(NOTEQ)){
        if(exprRel(rve)){
            if(rv.type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                    tkerr(crtTk,"a structure cannot be compared");

            rv.type=createType(TB_INT,-1);
            rv.isCtVal=rv.isLVal=0;
            if(exprEq1(rv)){
                return 1;
            }
            else{
                crtTk = startTk;
                return 1;
            }
        }
        else{
            crtTk = startTk;
            return 1;
        }
    }
    else{
        crtTk = startTk;
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

    if(exprAdd(rv)){
        if(exprRel1(rv)){
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
int exprRel1(RetVal& rv){
    print("exprRel1\n");

    Token *startTk = crtTk;
    RetVal rve;
    Token* tkop = crtTk;

    if(consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)){
        if(exprAdd(rve)){
            if(rv.type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be compared");
            if(rv.type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be compared");

            rv.type=createType(TB_INT,-1);
            rv.isCtVal=rv.isLVal=0;

            if(exprRel1(rv)){
                return 1;
            }
            else{
                crtTk = startTk;
                return 1;
            }

        }
        else{
            crtTk = startTk;
            return 1;
        }
    }
    else{
        crtTk = startTk;
    }

    success("exprRel1\n");
    return 1;
}



/// exprAdd: exprAdd ( ADD | SUB ) exprMul | exprMul ;
/// exprAdd: exprMul exprAdd1; exprAdd1: ( ADD | SUB ) exprMul exprAdd1
int exprAdd(RetVal& rv){
    print("exprAdd\n");

    Token *startTk = crtTk;

    if(exprMul(rv)){
        if(exprAdd1(rv)){
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
}
int exprAdd1(RetVal& rv){
    print("exprAdd1\n");

    Token *startTk = crtTk;
    Token* tkop = crtTk;
    RetVal rve;

    if(consume(ADD) || consume(SUB)){
        if(exprMul(rve)){
            if(rv.type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be added or subtracted");
            if(rv.type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be added or subtracted");
            rv.type=getArithType(&rv.type,&rve.type);
            rv.isCtVal=rv.isLVal=0;

            if(exprAdd1(rv)){
                return 1;
            }
            else{
                crtTk = startTk;
                return 1;
            }
        }
        else{
            crtTk = startTk;
            return 1;
        }

    }
    else{
        crtTk = startTk;
    }

    success("exprAdd1\n");
    return 1;
}



/// exprMul: exprMul ( MUL | DIV ) exprCast | exprCast ;
/// exprMul: exprCast exprMul1; exprMul1:( MUL | DIV ) exprCast exprMul1
int exprMul(RetVal& rv){
    print("exprMul\n");

    Token *startTk = crtTk;

    if(exprCast(rv)){
        if(exprMul1(rv)){
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
}
int exprMul1(RetVal& rv){
    print("exprMul1\n");

    Token *startTk = crtTk;
    RetVal rve;
    Token* tkop = crtTk;

    if(consume(MUL) || consume(DIV)){
        if(exprCast(rve)){
            if(rv.type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be multiplied or divided");
            if(rv.type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be multiplied or divided");
            rv.type=getArithType(&rv.type,&rve.type);
            rv.isCtVal=rv.isLVal=0;

            if(exprMul1(rv)){
                return 1;
            }
            else{
                crtTk = startTk;
                return 1;
            }
        }

        success("exprMul1\n");
        return 1;
    }
    else{
        crtTk = startTk;
    }

    return 1;
}



/// exprCast: LPAR typeName RPAR exprCast | exprUnary ;
int exprCast(RetVal& rv){
    print("exprCast\n");

    Token *startTk = crtTk;
    Type t;
    RetVal rve;

    if(consume(LPAR)){
        if(typeName(t)){
            if(consume(RPAR)){
                if(exprCast(rve)){
                    cast(&t,&rve.type);
                    rv.type=t;
                    rv.isCtVal=rv.isLVal=0;
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
        }
        else{
            crtTk = startTk;
            return 0;
        }
    }
    else{
        crtTk = startTk;
        if(exprUnary(rv)){
            return 1;
        }
        else{
            crtTk = startTk;
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

    if(consume(SUB) || consume(NOT)){
        if(exprUnary(rv)){
            if(tkop->code==SUB){
                if(rv.type.nElements>=0) tkerr(crtTk,"unary '-' cannot be applied to an array");
                if(rv.type.typeBase==TB_STRUCT) tkerr(crtTk,"unary '-' cannot be applied to a struct");
            }
            else{  // NOT
                if(rv.type.typeBase==TB_STRUCT) tkerr(crtTk,"'!' cannot be applied to a struct");
                rv.type=createType(TB_INT,-1);
            }
            rv.isCtVal=rv.isLVal=0;

            success("exprUnary\n");
            return 1;
        }
        else{
            crtTk = startTk;
            return 0;
        }
    }
    else if(exprPostfix(rv)){
        return 1;
    }
    else{
        crtTk = startTk;
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

    if(exprPrimary(rv)){
        if(exprPostfix1(rv)){
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
}
int exprPostfix1(RetVal& rv){
    print("exprPostfix1\n");

    Token *startTk = crtTk;
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
                if(exprPostfix1(rv)){
                    return 1;
                }
                else{
                    crtTk = startTk;
                    return 1;
                }

            }
            else{
                crtTk = startTk;
                return 1;
            }
        }
        else{
            crtTk = startTk;
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

            if(exprPostfix1(rv)){
                return 1;
            }
            else{
                crtTk = startTk;
                return 1;
            }
        }
        else{
            crtTk = startTk;
            return 1;
        }
    }
    else{
        crtTk = startTk;

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
                crtDefArg++;

                for(;;){
                    if(consume(COMMA)){
                        if(expr(arg)){

                            if(crtDefArg==s->args.end())tkerr(crtTk,"too many arguments in call");
                            cast(&((*crtDefArg)->type),&arg.type);
                            crtDefArg++;

                        }

                    }
                    else break;
                }
            }

            if(consume(RPAR)){
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
        return 1;
    }

    if(consume(CT_INT)){
        Token* tki = crtTk;
        rv.type=createType(TB_INT,-1);
        rv.ctVal.i=tki->i;
        rv.isCtVal=1;
        rv.isLVal=0;

        return 1;
    }
    if(consume(CT_REAL)){
        Token* tkr = crtTk;
        rv.type=createType(TB_DOUBLE,-1);
        rv.ctVal.d=tkr->r;
        rv.isCtVal=1;
        rv.isLVal=0;

        return 1;
    }
    if(consume(CT_CHAR)){
        Token* tkc = crtTk;
        rv.type=createType(TB_CHAR,-1);
        rv.ctVal.i=tkc->i;
        rv.isCtVal=1;
        rv.isLVal=0;

        return 1;
    }
    if(consume(CT_STRING)){
        Token* tks = crtTk;
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
            return 0;
        }
    }
    else{
        crtTk = startTk;
        return 0;
    }

    return 0;
}
