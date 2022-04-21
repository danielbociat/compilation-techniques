#include <bits/stdc++.h>
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
                    //addTk(LINECOMMENT);
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
                //addTk(COMMENT);
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
                    pCrtCh++;
                }

                break;
            }

            case 32:{
                switch(ch){
                    case 'a':{
                        pCrtCh++;
                        state = 31;
                        break;
                    }
                    case 'b':{
                        pCrtCh++;
                        state = 31;
                        break;
                    }
                    case 'f':{
                        pCrtCh++;
                        state = 31;
                        break;
                    }
                    case 'n':{
                        pCrtCh++;
                        state = 31;
                        break;
                    }
                    case 'r':{
                        pCrtCh++;
                        state = 31;
                        break;
                    }
                    case 't':{
                        pCrtCh++;
                        state = 31;
                        break;
                    }
                    case 'v':{
                        pCrtCh++;
                        state = 31;
                        break;
                    }
                    case '\'':{
                        pCrtCh++;
                        state = 31;
                        break;
                    }
                    case '?':{
                        pCrtCh++;
                        state = 31;
                        break;
                    }
                    case '\"':{
                        pCrtCh++;
                        state = 31;
                        break;
                    }
                    case '\\':{
                        pCrtCh++;
                        state = 31;
                        break;
                    }
                    case '0':{
                        pCrtCh++;
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
                addTk(CT_STRING);
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
    if(crtTk->code == code){
        consumedTk = crtTk;
        crtTk = crtTk->next;

        printf("consumed %s\n", codes[code]);

        return 1;
    }
    return 0;
}

int unit();

int ruleIf();
int ruleWhile();
int ruleFor();
int ruleBreak();
int ruleReturn();

int declStruct();
int declVar();
int typeBase();
int arrayDecl();
int typeName();

int declFunc();
int funcArg();

int stm();
int stmCompound();

int expr();
int exprAssign();
int exprOr();
int exprOr1();
int exprAnd();
int exprAnd1();
int exprEq();
int exprEq1();
int exprRel();
int exprRel1();
int exprAdd();
int exprAdd1();
int exprMul();
int exprMul1();
int exprCast();
int exprUnary();
int exprPostfix();
int exprPostfix1();
int exprPrimary();


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
            default:{
                printf("Code: %s\n", codes[current->code]);
                break;
            }
        }

        current = current->next;
    }
}

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

int main()
{
    FILE *f = fopen("2.c", "rb");

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
        if(!consume(ID)) tkerr(crtTk, "invalid expression declStruct 1");
        if(!consume(LACC)) {crtTk=startTk; return 0;};

        for(;;){
            if(declVar()) {}
            else break;
        }

        if(!consume(RACC)) tkerr(crtTk, "invalid expression declStruct 3");
        if(!consume(SEMICOLON)) tkerr(crtTk, "invalid expression declStruct 4");

        success("declStruct\n");
        return 1;
    }

    return 0;
}

///declVar:  typeBase ID arrayDecl? ( COMMA ID arrayDecl? )* SEMICOLON ;
int declVar(){
    print("declVar\n");

    Token *startTk = crtTk;
    if(typeBase()){
        if(!consume(ID)) tkerr(crtTk, "invalid expression declVar 1");
        arrayDecl();

        for(;;){
            if(consume(COMMA)){
                if(!consume(ID)) tkerr(crtTk, "invalid expression declVar 2");
                arrayDecl();
            }
            else
                break;

        }

        if(!consume(SEMICOLON)) tkerr(crtTk, "invalid expression declVar 3");

        success("declVar\n");
        return 1;
    }

    crtTk = startTk;
    return 0;
}


int typeBase(){
    print("typeBase\n");
    if(consume(INT)) return 1;
    if(consume(DOUBLE)) return 1;
    if(consume(CHAR)) return 1;
    if(consume(STRUCT)){
        if(!consume(ID)) tkerr(crtTk, "invalid expression typeBase");

        success("typeBase\n");
        return 1;
    }
    return 0;
}

int arrayDecl(){
    print("arrayDecl\n");
    if(consume(LBRACKET)){
        expr();
        if(!consume(RBRACKET)) tkerr(crtTk, "invalid expression arrayDecl");

        success("arrayDecl\n");
        return 1;
    }
    return 0;
}

int typeName(){
    print("typeName\n");
    if(typeBase()){
        arrayDecl();

        success("typeName\n");
        return 1;
    }

    return 0;
}


int declFunc(){
    print("declFunc\n");
    Token *startTk = crtTk;

    if(typeBase()){
        if(consume(MUL)){}
    }
    else if(consume(VOID)){}
    else{
      crtTk = startTk;
      return 0;
    }

    if(!consume(ID)) tkerr(crtTk, "invalid expression declFunc");
    if(!consume(LPAR)) {crtTk = startTk; return 0;};
    if(funcArg()){
        for(;;){
            if(consume(COMMA)){
                if(!funcArg()) tkerr(crtTk, "invalid expression declFunc");
            }
            else break;
        }
    }
    if(!consume(RPAR)) tkerr(crtTk, "invalid expression declFunc");
    if(!stmCompound()) tkerr(crtTk, "invalid expression declFunc");

    success("declFunc\n");
    return 1;
}

///funcArg: typeBase ID arrayDecl? ;
int funcArg(){
    print("funcArg\n");
    Token *startTk = crtTk;

    if(typeBase()){
        if(!consume(ID)) tkerr(crtTk, "invalid expression funcArg");
        if(arrayDecl()) {}

        success("funcArg\n");
        return 1;
    }

    return 0;
}

int stm(){
    print("stm\n");
    Token *startTk = crtTk;

    if(stmCompound()){
        return 1;
    } else {crtTk = startTk;}

    if(ruleIf()){
        print("RULE IF\n");
        return 1;
    } else {crtTk = startTk;}

    if(ruleWhile()){
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


    if(expr()){}
    if(consume(SEMICOLON)) return 1;

    return 0;
}

int stmCompound()
{
    print("stmCompound\n");

    Token *startTk = crtTk;

    if(consume(LACC)){
        for(;;){
            if(declVar()){
            }
            else if(stm()){
            }
            else break;
        }

        if(!consume(RACC))tkerr(crtTk,"missing } or syntax error stmCompound");

        success("stmCompound\n");
        return 1;
    }


    return 0;
}

int ruleIf(){
    Token *startTk = crtTk;

    print("ruleIf\n");

    if(consume(IF)){
        if(!consume(LPAR))  tkerr(crtTk,"missing ( after if");
        if(!expr()) tkerr(crtTk,"invalid expression after (");
        if(!consume(RPAR))  tkerr(crtTk,"missing )");
        if(!stm())  tkerr(crtTk,"missing if statement");

        if(consume(ELSE)){
            if(!stm())  tkerr(crtTk,"missing else statement");
        }

        success("ruleIf\n");
        return 1;
    }

    return 0;
}

int ruleWhile(){
    Token *startTk = crtTk;

    print("ruleWhile\n");

    if(consume(WHILE)){
        if(!consume(LPAR))  tkerr(crtTk,"missing ( after while");
        if(!expr())tkerr(crtTk,"invalid expression after (");
        if(!consume(RPAR))  tkerr(crtTk,"missing )");

        if(!stm())  tkerr(crtTk,"missing while statement");
        success("ruleWhile\n");
        return 1;
    }


    return 0;
}

int ruleFor(){
    Token *startTk = crtTk;

    print("ruleFor\n");

    if(consume(FOR)){
        if(!consume(LPAR))  tkerr(crtTk,"missing ( after for");
        if(expr()){}
        if(!consume(SEMICOLON)) tkerr(crtTk,"missing ; after first statement in for");

        if(expr()){}
        if(!consume(SEMICOLON)) tkerr(crtTk,"missing ; after second statement in for");

        if(expr()){}
        if(!consume(RPAR))  tkerr(crtTk,"missing ) after third statement in for");

        if(!stm())  tkerr(crtTk,"missing for statement");
        success("ruleFor\n");
        return 1;
    }

    return 0;
}

int ruleBreak(){
    Token *startTk = crtTk;

    print("ruleBreak\n");

    if(consume(BREAK)){
        if(!consume(SEMICOLON)) tkerr(crtTk,"missing ; after break");
        success("ruleBreak\n");
        return 1;
    }

    return 0;
}

int ruleReturn(){
    Token *startTk = crtTk;

    print("ruleReturn\n");

    if(consume(RETURN)){
        if(expr()) {}
        if(!consume(SEMICOLON)) tkerr(crtTk,"missing ; after return");

        success("ruleReturn\n");
        return 1;
    }

    return 0;
}

/// expr: exprAssign
int expr(){
    print("expr\n");
    return exprAssign();
}

/// exprAssign: exprUnary ASSIGN exprAssign | exprOr
int exprAssign(){
    print("exprAssign\n");
    Token *startTk = crtTk;

    if(exprUnary()){
        if(consume(ASSIGN)){
            if(exprAssign()){
                success("exprAssign\n");
                return 1;
            }

            tkerr(crtTk, "invalid expression after =, exprAssign");
        }
    }

    crtTk = startTk;
    if(exprOr()){
        success("exprAssign\n");
        return 1;
    }

    return 0;
}



/// exprOr: exprOr OR exprAnd | exprAnd =>
/// => exprOr: exprAnd exprOr1; exprOr1: OR exprAnd exprOr1 | e
int exprOr(){
    print("exprOr\n");
    Token *startTk = crtTk;

    if(exprAnd()){
        if(!exprOr1()) tkerr(crtTk, "invalid expression exprOr");
        success("exprOr\n");
        return 1;
    }


    return 0;
}
int exprOr1(){
    print("exprOr1\n");
    Token *startTk = crtTk;

    if(consume(OR)){
        if(!exprAnd()) tkerr(crtTk, "invalid expression exprOr1");
        if(!exprOr1()) tkerr(crtTk, "invalid expression exprOr1");

        success("exprOr1\n");
        return 1;
    }

    success("exprOr1\n");
    return 1;
}



/// exprAnd: exprAnd AND exprEq | exprEq ;
/// exprAnd: exprEq exprAnd1; exprAnd1: (AND exprEq exprAnd1)?
int exprAnd(){
    print("exprAnd\n");
    Token *startTk = crtTk;

    if(exprEq()){
        if(!exprAnd1()) tkerr(crtTk, "invalid expression exprAnd");

        success("exprAnd\n");
        return 1;
    }


    return 0;
}
int exprAnd1(){
    print("exprAnd1\n");
    Token *startTk = crtTk;

    if(consume(AND)){
        if(!exprEq()) tkerr(crtTk, "invalid expression exprAnd1");
        if(!exprAnd1()) tkerr(crtTk, "invalid expression exprAnd1");

        success("exprAnd1\n");
        return 1;
    }

    success("exprAnd1\n");
    return 1;
}



/// exprEq: exprEq ( EQUAL | NOTEQ ) exprRel | exprRel
/// exprEq: exprRel exprEq1; exprEq1: ( EQUAL | NOTEQ ) exprRel exprEq1
int exprEq(){
    print("exprEq\n");
    Token *startTk = crtTk;

    if(exprRel()){
        if(!exprEq1()) tkerr(crtTk, "invalid expression exprEq");

        success("exprEq\n");
        return 1;
    }


    return 0;
}
int exprEq1(){
    print("exprEq1\n");
    Token *startTk = crtTk;

    if(consume(EQUAL) || consume(NOTEQ)){
        if(!exprRel()) tkerr(crtTk, "invalid expression exprEq1");
        if(!exprEq1()) tkerr(crtTk, "invalid expression exprEq1");

        success("exprEq1\n");
        return 1;
    }

    success("exprEq1\n");
    return 1;
}



///exprRel: exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd ;
///exprRel: exprAdd exprRel1; exprRel1:( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd exprRel1
int exprRel(){
    print("exprRel\n");
    Token *startTk = crtTk;

    if(exprAdd()){
        if(!exprRel1()) tkerr(crtTk, "invalid expression exprRel");

        success("exprRel\n");
        return 1;
    }

    return 0;
}
int exprRel1(){
    print("exprRel1\n");
    Token *startTk = crtTk;

    if(consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)){
        if(!exprAdd()) tkerr(crtTk, "invalid expression exprRel1");
        if(!exprRel1()) tkerr(crtTk, "invalid expression exprRel1");

        success("exprRel1\n");
        return 1;
    }

    success("exprRel1\n");
    return 1;
}



/// exprAdd: exprAdd ( ADD | SUB ) exprMul | exprMul ;
/// exprAdd: exprMul exprAdd1; exprAdd1: ( ADD | SUB ) exprMul exprAdd1
int exprAdd(){
    print("exprAdd\n");
    Token *startTk = crtTk;

    if(exprMul()){
        if(!exprAdd1()) tkerr(crtTk, "invalid expression exprAdd");

        success("exprAdd\n");
        return 1;
    }


    return 0;
}
int exprAdd1(){
    print("exprAdd1\n");
    Token *startTk = crtTk;

    if(consume(ADD) || consume(SUB)){
        if(!exprMul()) tkerr(crtTk, "invalid expression exprAdd1");
        if(!exprAdd1()) tkerr(crtTk, "invalid expression exprAdd1");

        success("exprAdd1\n");
        return 1;
    }

    success("exprAdd1\n");
    return 1;
}



/// exprMul: exprMul ( MUL | DIV ) exprCast | exprCast ;
/// exprMul: exprCast exprMul1; exprMul1:( MUL | DIV ) exprCast exprMul1
int exprMul(){
    print("exprMul\n");
    Token *startTk = crtTk;

    if(exprCast()){
        if(!exprMul1()) tkerr(crtTk, "invalid expression exprMul");

        success("exprMul\n");
        return 1;
    }


    return 0;
}
int exprMul1(){
    print("exprMul1\n");
    Token *startTk = crtTk;

    if(consume(MUL) || consume(DIV)){
        if(!exprCast()) tkerr(crtTk, "invalid expression exprMul1");
        if(!exprMul1()) tkerr(crtTk, "invalid expression exprMul1");

        success("exprMul1\n");
        return 1;
    }

    success("exprMul1\n");
    return 1;
}



/// exprCast: LPAR typeName RPAR exprCast | exprUnary ;
int exprCast(){
    print("exprCast\n");
    Token *startTk = crtTk;
    if(consume(LPAR)){
        if(!typeName()) tkerr(crtTk, "invalid expression exprCast");
        if(!consume(RPAR)) tkerr(crtTk, "invalid expression exprCast");
        if(!exprUnary()) tkerr(crtTk, "invalid expression exprCast");

        success("exprCast\n");
        return 1;
    }


    if(exprUnary()){
        success("exprCast\n");
        return 1;
    }

    return 0;
}



/// exprUnary: ( SUB | NOT ) exprUnary | exprPostfix ;
int exprUnary(){
    print("exprUnary\n");
    Token *startTk = crtTk;

    if(consume(SUB) || consume(NOT)){
        if(!exprUnary) tkerr(crtTk, "invalid expression exprUnary");

        success("exprUnary\n");
        return 1;
    }

    if(exprPostfix()){
        success("exprUnary\n");
        return 1;
    }



    return 0;
}



/// exprPostfix: exprPostfix LBRACKET expr RBRACKET | exprPostfix DOT ID | exprPrimary ;
/// exprPostfix: exprPrimary exprPostfix1;
/// exprPostfix1: LBRACKET expr RBRACKET exprPostfix1 | DOT ID exprPostfix1 | e
int exprPostfix(){
    print("exprPostfix\n");
    Token *startTk = crtTk;

    if(exprPrimary()){
        if(!exprPostfix1()) tkerr(crtTk, "invalid expression exprPostfix");

        success("exprPostfix\n");
        return 1;
    }

    return 0;
}
int exprPostfix1(){
    print("exprPostfix1\n");
    Token *startTk = crtTk;

    if(consume(LBRACKET)){
        if(!expr()) tkerr(crtTk, "invalid expression exprPostfix1");
        if(!consume(RBRACKET)) tkerr(crtTk, "invalid expression exprPostfix1");
        if(!exprPostfix1()) tkerr(crtTk, "invalid expression exprPostfix1");

        success("exprPostfix1\n");
        return 1;
    }

    if(consume(DOT)){
        if(!consume(ID)) tkerr(crtTk, "invalid expression exprPostfix1");
        if(!exprPostfix1()) tkerr(crtTk, "invalid expression exprPostfix1");

        success("exprPostfix1\n");
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
int exprPrimary(){
    print("exprPrimary\n");

    Token *startTk = crtTk;
    if(consume(ID)){
        if(consume(LPAR)){
            if(!expr()){
                for(;;){
                    if(consume(COMMA)){
                        if(!expr()) tkerr(crtTk, "invalid expression exprPrimary");
                    }
                    else break;
                }
            }

            if(!consume(RPAR)) tkerr(crtTk, "invalid expression exprPrimary");
        }

        success("exprPrimary\n");
        return 1;
    }

    if(consume(CT_INT)) return 1;
    if(consume(CT_REAL)) return 1;
    if(consume(CT_CHAR)) return 1;
    if(consume(CT_STRING)) return 1;

    if(consume(LPAR)){
        if(!expr()) tkerr(crtTk, "invalid expression exprPrimary");
        if(!consume(RPAR)) tkerr(crtTk, "invalid expression exprPrimary");

        success("exprPrimary\n");
        return 1;
    }

    //crtTk = startTk;
    return 0;
}
