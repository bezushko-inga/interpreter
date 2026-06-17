#include <stack>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <string>
#include <ctype.h>
#include <algorithm>

using namespace std;

// lexical analyzer

enum type_of_lex {
    LEX_NULL,                                               //   0

    LEX_AND, LEX_OR, LEX_NOT, LEX_IF, LEX_ELSE, LEX_DO,     //   6
    LEX_WHILE, LEX_FOR, LEX_BREAK, LEX_GOTO, LEX_PROGRAM,   //  11
    LEX_FALSE, LEX_TRUE, LEX_READ, LEX_WRITE,               //  15
    LEX_INT, LEX_BOOL, LEX_STR, LEX_STRUCT, LEX_REAL,       //  20

    LEX_SEMICOLON, LEX_COMMA, LEX_COLON, LEX_DOT, LEX_NEQ,  //  25
    LEX_ASSIGN, LEX_LPAREN, LEX_RPAREN, LEX_EQ, LEX_LSS,    //  30
    LEX_GTR, LEX_PLUS, LEX_MINUS, LEX_TIMES, LEX_SLASH,     //  35
    LEX_LEQ, LEX_GEQ, LEX_PERCENT, LEX_LBRACE, LEX_RBRACE,  //  40

    LEX_NUM, LEX_ID, LEX_FIN,                               //  43
    POLIZ_LABEL, POLIZ_ADDRESS, POLIZ_GO, POLIZ_FGO         //  47
};

class Lex {
private:
    type_of_lex type;
    int value;
    string str_value;
public:
    Lex(type_of_lex t = LEX_NULL, int v = 0, string s = "-") : type(t), value(v), str_value(s) {}
    void put_type(type_of_lex t) { type = t; }
    void put_value(int v) { value = v; }
    void put_string(string s) { str_value = s; }
    type_of_lex get_type() const { return type; }
    int get_value() const { return value; }
    string get_string() const { return str_value; }
    friend ostream& operator << (ostream& s, Lex l);
};

class Ident {
private:
    string name;
    bool declare;
    type_of_lex type;
    bool assign;
    int value;
    string str;
public:
    Ident() {
        declare = false;
        assign = false;
    }
    Ident(const string newname) {
        name = newname;
        declare = false;
        assign = false;
    }
    bool operator== (const string& s) const { return name == s; }
    string get_name() const { return name; }
    string get_string() const { return str; }
    bool get_declare() const { return declare; }
    void put_name(string newname) { name = newname; }
    void put_string(string s) { str = s; }
    void put_declare() { declare = true; }
    type_of_lex get_type() const { return type; }
    void put_type(type_of_lex t) { type = t; }
    bool get_assign() const { return assign; }
    void put_assign() { assign = true; }
    int get_value() const { return value; }
    void put_value(int val) { value = val; }
};

vector<Ident> TID;

int put(const string& buf) { // add variable to the identifier table
    for (int i = 0; i < TID.size(); i++) {
        if (TID[i] == buf) return i;
    }
    TID.push_back(Ident(buf));
    return TID.size() - 1;
}

int find_ident(const string& buf) {
    for (int i = 0; i < TID.size(); i++) {
        if (TID[i] == buf) return i;
    }
    return -1;
}

class Scanner {
private:
    FILE* fp;
    int c;
    int look(const string buf, const char** list) {
        int i = 1;
        while (list[i]) {
            if (buf == list[i]) return i;
            i++;
        }
        return 0;
    }
    void gc() { c = fgetc(fp); }
public:
    static const char* TW[], * TD[];
    Scanner(const char* program) {
        fp = fopen(program, "r");
        if (!fp) throw "file openning error";
    }
    Lex get_lex();
    ~Scanner() { fclose(fp); }
};

const char* Scanner::TW[] = { // keywords
    "", "and", "or", "not", "if", "else", "do", "while", "for",
    "break", "goto", "program", "false", "true", "read", "write",
    "int", "bool", "string", "struct", "real",
    NULL
};

const char* Scanner::TD[] = { // delimiters
    "", ";", ",", ":", ".", "!=", "=", "(", ")", "==", "<",
    ">", "+", "-", "*", "/", "<=", ">=", "%", "{", "}",
    NULL
};

Lex Scanner::get_lex() {
    enum state { H, IDENT, NUMB, STRLIT, SLASH, COM, COMS, ALE, NEQ };
    state CS = H;
    string buf;
    int digit, j;
    do {
        gc();
        switch (CS) {
        case H: // initial state
            if (isspace(c)) {} // whitespace: space, tab, newline, etc.
            else if (isalpha(c)) {
                buf.push_back(c);
                CS = IDENT;
            } // letter
            else if (isdigit(c)) {
                digit = c - '0';
                CS = NUMB;
            } // number
            else if (c == '=' || c == '<' || c == '>') {
                buf.push_back(c);
                CS = ALE;
            } // possible two-character operator
            else if (c == '!') {
                buf.push_back(c);
                CS = NEQ;
            } // !=
            else if (c == '/') {
                CS = SLASH;
            } // comment
            else if (c == '"') {
                CS = STRLIT;
            } // string
            else if (c == EOF)
                return Lex(LEX_FIN, 0); // EOF
            else { // single-character delimiter from TD
                buf.push_back(c);
                if ((j = look(buf, TD)))
                    return Lex((type_of_lex)(j + (int)LEX_REAL), j);
                else throw c;
            }
            break;

        case IDENT: // identifier
            if (isalpha(c) || isdigit(c) || c == '.') buf.push_back(c);
            else {
                ungetc(c, fp); 
                if ((j = look(buf, TW))) {
                    if ((type_of_lex)j == LEX_TRUE)
                        return Lex(LEX_TRUE, 1);
                    if ((type_of_lex)j == LEX_FALSE)
                        return Lex(LEX_FALSE, 0);
                    return Lex((type_of_lex)j, j);
                }
                else return Lex(LEX_ID, put(buf)); // identifier
            }
            break;

        case NUMB:
            if (isdigit(c)) digit = digit * 10 + (c - '0');
            else {
                ungetc(c, fp);
                return Lex(LEX_NUM, digit);
            }
            break;

        case STRLIT:
            if (c == '"') return Lex(LEX_STR, 0, buf);
            else if (c == EOF) throw c;
            else buf.push_back(c);
            break;

        case SLASH:
            if (c == '/') CS = COM;
            else if (c == '*') CS = COMS;
            else {
                ungetc(c, fp);
                return Lex(LEX_SLASH, look("/", TD));
            }
            break;

        case COMS:
            if (c == '*') {
                while (c == '*') gc();
                if (c == '/') CS = H;
            }
            else if (c == EOF) throw c;
            break;

        case COM:
            if (c == EOF || c == '\n') CS = H;
            break;

        case ALE:
            if (c == '=') {
                buf.push_back(c);
                if ((j = look(buf, TD)))
                    return Lex((type_of_lex)(j + (int)LEX_REAL), j);
            }
            else {
                ungetc(c, fp);
                if ((j = look(buf, TD)))
                    return Lex((type_of_lex)(j + (int)LEX_REAL), j);
            }
            break;

        case NEQ:
            if (c == '=') {
                buf.push_back(c);
                j = look(buf, TD);
                return Lex(LEX_NEQ, j);
            }
            else throw c;
            break;
        }
    } while (true);
}

ostream& operator<< (ostream& s, Lex l) {
    string name;
    if (l.type <= LEX_REAL) name = Scanner::TW[l.type];
    else if (l.type >= LEX_SEMICOLON && l.type <= LEX_RBRACE)
        name = Scanner::TD[l.type - LEX_SEMICOLON + 1];
    else if (l.type == LEX_ID) name = TID[l.value].get_name();
    else if (l.type == LEX_FIN) name = "@";
    else if (l.type == LEX_NUM) name = "num";
    else if (l.type == POLIZ_LABEL) name = "label";
    else if (l.type == POLIZ_ADDRESS) name = "addr";
    else if (l.type == POLIZ_GO) name = "!";
    else if (l.type == POLIZ_FGO) name = "!F";
    else throw l;
    s << '(' << name << ',' << l.value << ");" << endl;
    return s;
}

// syntax analyzer

class structure {
public:
    string name;
    vector<Ident> TSID;
};

class structinf {
public:
    string name;
    string type;
};

vector<int> labels;
vector<int> adress;
vector<int> gotos;
vector<structure> structs;
vector<structinf> infs;

class Parser {
    Scanner scanner;
    Lex CurrentLex;
    type_of_lex LexType;
    int LexValue;
    int BracketCount;
    int InCycle = 0;
    stack<int> st_int;
    stack<type_of_lex> st_lex;
    type_of_lex type;
    type_of_lex ltype;
    int lb = 0;

    void P();   // P  -> program {<S;> <C;> B}
    void S();   // S  -> struct I {<C;>}
    void C();   // C  -> [int | bool | string] H <,H>
    void H();   // H  -> I | I = E7
    void B();   // B  -> <A>
    void A();   // A  -> if (E) A [else A] | while(E) A | for(C; E; E) A |
    //       read(I); | write(E <,E>); | goto I; | I:A; | {B} | E;
    void E();   // E  -> E1 = E | E1
    void E1();  // E1 -> E2 < or E2>
    void E2();  // E2  -> E3 < and E3>
    void E3();  // E3  -> E4 < [ == | < | > | <= | >= | != ] E4>
    void E4();  // E4  -> E5 < [ + | - ] E5>
    void E5();  // E5  -> E6 < [ * | / | % ] E6>
    void E6();  // E6  -> E7 | E7 | not E6 | (E) | D
    void E7();  // E7 -> [-|+] N | "S" | true | false

    void gl() {
        CurrentLex = scanner.get_lex();
        LexType = CurrentLex.get_type();
        LexValue = CurrentLex.get_value();
        //cout << "Current Lex " << CurrentLex;
    }
    void IdentInit();

    // semantic analyser
    void dec(type_of_lex type);
    void check_id();
    void check_init(type_of_lex t1, type_of_lex t2);
    void check_op();
    void check_not();
    void eq_type();
    void eq_bool();
    void check_id_in_read();

public:
    Parser(const char* program) : scanner(program), BracketCount(0) {}
    vector<int> BreakCounter;
    void analyze();

    vector<Lex> poliz;
};

void add_label(int num) {
    for (int i = 0; i < labels.size(); i++) {
        if (labels[i] == num)
            throw "label redefinding";
    }
    labels.push_back(num);
}

int add_goto(int num, int addr) {
    for (int i = 0; i < gotos.size(); i++) {
        if (gotos[i] == num)
            return adress[i];
    }
    if (addr) {
        adress.push_back(addr);
        gotos.push_back(num);
    }
    return 0;
}

int is_struct(const string& buf) {
    for (int i = 0; i < structs.size(); i++) {
        if (structs[i].name == buf)
            return i;
    }
    return -1;
}

int is_struct_var(const string& name) {
    for (int i = 0; i < infs.size(); i++) {
        if (infs[i].name == name)
            return i;
    }
    return -1;
}

void Parser::analyze() {
    gl();
    P();
    if (LexType != LEX_FIN)
        throw "the end of file was expected";
    if (BracketCount)
        throw "unclosed bracket";
    int flag = 1;
    int flag2;
    for (int i = 0; i < gotos.size() && flag; i++) {
        flag2 = 1;
        for (int j = 0; j < labels.size() && flag2; j++)
            if (gotos[i] == labels[j]) flag2 = 0;
        if (flag2) flag = 0;
    }
    if (!flag) throw "label error";
    //for (Lex lex : poliz) cout << lex;

}

void Parser::IdentInit() {
    if (LexType == LEX_INT || LexType == LEX_BOOL || LexType == LEX_STR) {
        while (true) {
            type = LexType;
            gl();
            C();
            if (LexType == LEX_SEMICOLON) gl();
            else throw "';' was expected 1";
            if (LexType != LEX_INT && LexType != LEX_BOOL && LexType != LEX_STR)
                break;
        }
    }
    else if (LexType == LEX_ID) {
        string StructName;
        string StructType = TID[LexValue].get_name();
        structinf StructInf;
        string CurName;
        do {
            gl();
            StructName = TID[LexValue].get_name();
            int idx = is_struct(StructType);
            if (idx == -1)
                throw "unknown structure type";
            StructInf.type = StructType;
            StructInf.name = StructName;
            infs.push_back(StructInf);
            st_int.push(LexValue);
            CurName = StructName;
            dec(LEX_STRUCT);
            for (int i = 0; i < structs[idx].TSID.size(); i++) {
                string CurVar = structs[idx].TSID[i].get_name();
                string id = CurName + '.' + CurVar;
                int Value = put(id);
                st_int.push(Value);
                dec(structs[idx].TSID[i].get_type());
            }
            gl();
        } while (LexType == LEX_COMMA);
        if (LexType == LEX_SEMICOLON) gl();
        else throw "';' was expected";
        if (LexType != LEX_LBRACE) IdentInit();
    }
}

void Parser::P() { // P  -> program {<S;> <С;> B}
    if (LexType == LEX_PROGRAM) gl();
    else throw "'program' was expected";
    if (LexType == LEX_LBRACE) {
        gl();
        BracketCount++;
    }
    else throw "'{' was expected";
    while (LexType == LEX_STRUCT) {
        S();
        gl();
    }
    IdentInit();
    B();
    if (LexType == LEX_RBRACE) {
        gl();
        BracketCount--;
    }
    else throw "'}' was expected";
}

void Parser::S() { // S  -> struct I {<C;>}
    Ident Elem;
    structure s;
    gl();
    if (LexType == LEX_ID) {
        s.name = TID[LexValue].get_name();
        gl();
    }
    else throw "structure without name";
    if (LexType == LEX_LBRACE) {
        gl();
    }
    else throw "'{' was expected";
    while (LexType != LEX_RBRACE) {
        if (LexType != LEX_INT && LexType != LEX_BOOL && LexType != LEX_STR)
            throw "unexpected id in structure";
        type_of_lex str_type = LexType;
        gl();
        while (LexType != LEX_SEMICOLON) {
            if (LexType == LEX_ID) {
                Elem.put_name(TID[LexValue].get_name());
                Elem.put_type(str_type);
                s.TSID.push_back(Elem);
                gl();
            }
            else throw "id was expected";
            if (LexType == LEX_COMMA) gl();
            else if (LexType != LEX_SEMICOLON) throw "unexpected char";
        }
        gl();
    }
    structs.push_back(s);
}

void Parser::C() { // С  -> [int | bool | string] H <,H>
    H();
    while (LexType == LEX_COMMA) {
        gl();
        H();
    }
}

void Parser::H() { // H  -> I | I = E7
    if (LexType == LEX_ID) {
        int TempValue = LexValue;
        st_int.push(TempValue);
        dec(type);
        gl();
        if (LexType == LEX_ASSIGN) {
            poliz.push_back(Lex(POLIZ_ADDRESS, TempValue));
            gl();
            E7();
            check_init(type, ltype);
        }
    }
    else throw "id was expected";

}

void Parser::B() { // B  -> <A>
    while (LexType != LEX_FIN && LexType != LEX_RBRACE) A();
}

void Parser::A() { // A  -> if (E) A [else A] | while(E) A | for(C; E; E) A |
    //       read(I); | write(E <,E>); | goto I; | I:A; | {B} | E;
    if (LexType == LEX_IF) {
        gl();
        if (LexType == LEX_LPAREN) gl();
        else throw "'(' was expected";
        E();
        eq_bool();
        int pl1 = poliz.size();
        poliz.push_back(Lex());
        poliz.push_back(Lex(POLIZ_FGO));
        if (LexType == LEX_RPAREN) gl();
        else throw "')' was expected";
        A();
        if (LexType == LEX_ELSE) {
            int pl2 = poliz.size();
            poliz.push_back(Lex());
            poliz.push_back(Lex(POLIZ_GO));
            poliz[pl1] = Lex(POLIZ_LABEL, poliz.size());
            gl();
            A();
            poliz[pl2] = Lex(POLIZ_LABEL, poliz.size());
        }
        else poliz[pl1] = Lex(POLIZ_LABEL, poliz.size());
    }
    else if (LexType == LEX_WHILE) {
        InCycle++;
        int oldBreakSize = BreakCounter.size();

        int pl1 = poliz.size();

        gl();
        if (LexType == LEX_LPAREN) gl();
        else throw "'(' was expected";

        E();
        eq_bool();

        int pl2 = poliz.size();
        poliz.push_back(Lex());
        poliz.push_back(Lex(POLIZ_FGO));

        if (LexType == LEX_RPAREN) gl();
        else throw "')' was expected";

        A();

        poliz.push_back(Lex(POLIZ_LABEL, pl1));
        poliz.push_back(Lex(POLIZ_GO));

        int label_end = poliz.size();
        poliz[pl2] = Lex(POLIZ_LABEL, label_end);

        for (int i = oldBreakSize; i < BreakCounter.size(); i++)
            poliz[BreakCounter[i]] = Lex(POLIZ_LABEL, label_end);

        BreakCounter.resize(oldBreakSize);
        InCycle--;
    }
    else if (LexType == LEX_BREAK) {
        if (!InCycle)
            throw "unexpected break";

        int pl = poliz.size();
        poliz.push_back(Lex());
        poliz.push_back(Lex(POLIZ_GO));
        BreakCounter.push_back(pl);

        gl();
        if (LexType == LEX_SEMICOLON) gl();
        else throw "';' was expected";
    }
    else if (LexType == LEX_FOR) {
        InCycle++;
        int oldBreakSize = BreakCounter.size();

        gl();
        if (LexType == LEX_LPAREN) gl();
        else throw "'(' was expected";

        if (LexType != LEX_SEMICOLON) {
            E();
            poliz.push_back(Lex(LEX_SEMICOLON));
        }

        if (LexType == LEX_SEMICOLON) gl();
        else throw "';' was expected";

        int label_cond = poliz.size();

        if (LexType != LEX_SEMICOLON) {
            E();
            eq_bool();
        }
        else {
            poliz.push_back(Lex(LEX_TRUE, 1));
        }

        int pl_fgo = poliz.size();
        poliz.push_back(Lex());
        poliz.push_back(Lex(POLIZ_FGO));

        int pl_go_body = poliz.size();
        poliz.push_back(Lex());
        poliz.push_back(Lex(POLIZ_GO));

        if (LexType == LEX_SEMICOLON) gl();
        else throw "';' was expected";

        int label_step = poliz.size();

        if (LexType != LEX_RPAREN) {
            E();
            poliz.push_back(Lex(LEX_SEMICOLON));
        }

        poliz.push_back(Lex(POLIZ_LABEL, label_cond));
        poliz.push_back(Lex(POLIZ_GO));

        if (LexType == LEX_RPAREN) gl();
        else throw "')' was expected";

        int label_body = poliz.size();
        poliz[pl_go_body] = Lex(POLIZ_LABEL, label_body);

        A();

        poliz.push_back(Lex(POLIZ_LABEL, label_step));
        poliz.push_back(Lex(POLIZ_GO));

        int label_end = poliz.size();
        poliz[pl_fgo] = Lex(POLIZ_LABEL, label_end);

        for (int i = oldBreakSize; i < BreakCounter.size(); i++)
            poliz[BreakCounter[i]] = Lex(POLIZ_LABEL, label_end);

        BreakCounter.resize(oldBreakSize);
        InCycle--;
    }
    else if (LexType == LEX_GOTO) {
        gl();
        if (LexType == LEX_ID) {
            int label = add_goto(LexValue, 0);
            lb++;
            if (label) {
                poliz.push_back(Lex(POLIZ_LABEL, label));
                poliz.push_back(Lex(POLIZ_GO));
            }
            else {
                add_goto(LexValue, poliz.size());
                poliz.push_back(Lex());
                poliz.push_back(Lex(POLIZ_GO));
            }
            gl();
            if (LexType == LEX_SEMICOLON) gl();
            else throw "';' was expected 4";
        }
        else throw "label was expected";
    }
    else if (LexType == LEX_READ) {
        gl();
        if (LexType == LEX_LPAREN) gl();
        else throw "'(' was expected";
        if (LexType == LEX_ID) {
            check_id_in_read();
            poliz.push_back(Lex(POLIZ_ADDRESS, LexValue));
            gl();
        }
        else throw "unexpected id";
        poliz.push_back(Lex(LEX_READ));
        if (LexType == LEX_RPAREN) gl();
        else throw "')' was expected";
        if (LexType == LEX_SEMICOLON) gl();
        else throw "';' was expected 5";
    }
    else if (LexType == LEX_WRITE) {
        gl();
        if (LexType != LEX_LPAREN) throw "'(' was expected";
        do {
            gl();
            if (LexType == LEX_ID) {
                if (is_struct_var(TID[LexValue].get_name()) >= 0)
                    throw "can not write structure";
            }
            E();
            poliz.push_back(Lex(LEX_WRITE));
        } while (LexType == LEX_COMMA);
        if (LexType == LEX_RPAREN) gl();
        else throw "')' was expected";
        if (LexType == LEX_SEMICOLON) gl();
        else throw "';' was expected 6";
    }
    else if (LexType == LEX_LBRACE) {
        gl();
        BracketCount++;
        B();
        if (LexType == LEX_RBRACE) {
            BracketCount--;
            if (!BracketCount)
                for (Lex lex : poliz) cout << lex;
            gl();
        }
        else throw "'}' was expected";
    }
    else {
        E();
        if (LexType == LEX_SEMICOLON) {
            poliz.push_back(Lex(LEX_SEMICOLON));
            gl();
        }
        else if (LexType != LEX_RBRACE && LexType != LEX_FIN)
            throw "';' was expected 7";
    }
}

void Parser::E() {
    if (LexType == LEX_ID) {
        const string name = TID[LexValue].get_name();
        int flg = is_struct_var(name);
        if (flg == -1) {
            int pl = poliz.size();
            E1();
            if (LexType == LEX_ASSIGN) {
                poliz[pl].put_type(POLIZ_ADDRESS);
                //st_lex.push(LexType);
                gl();
                E();
                eq_type();
            }
        }
        else {
            string StructType = infs[flg].type;
            string StructName = infs[flg].name;
            vector<Ident> Table;
            for (int i = 0; i < structs.size(); i++) {
                if (structs[i].name == StructType) {
                    Table = structs[i].TSID;
                    break;
                }
            }
            gl();
            if (LexType == LEX_SEMICOLON) return;
            else if (LexType != LEX_ASSIGN) throw "wrong operation";
            gl();
            string RightName = TID[LexValue].get_name();
            int rflg = is_struct_var(RightName);

            if (rflg == -1)
                throw "right operand is not structure";

            if (StructType != infs[rflg].type)
                throw "different structure types";

            for (int i = 0; i < Table.size(); i++) {
                string leftField = StructName + "." + Table[i].get_name();
                string rightField = RightName + "." + Table[i].get_name();

                int leftId = find_ident(leftField);
                int rightId = find_ident(rightField);

                if (leftId == -1 || rightId == -1)
                    throw "structure field not found";

                if (!TID[rightId].get_assign())
                    throw "uninitialized structure field";

                poliz.push_back(Lex(POLIZ_ADDRESS, leftId));
                poliz.push_back(Lex(LEX_ID, rightId));
                poliz.push_back(Lex(LEX_ASSIGN));
                poliz.push_back(Lex(LEX_SEMICOLON));
            }

            gl();
        }
    }
    else E1();
}

void Parser::E1() {
    E2();
    while (LexType == LEX_OR) {
        st_lex.push(LexType);
        gl();
        E2();
        check_op();
    }
}

void Parser::E2() {
    E3();
    while (LexType == LEX_AND) {
        st_lex.push(LexType);
        gl();
        E3();
        check_op();
    }
}

void Parser::E3() {
    E4();
    while (LexType == LEX_EQ || LexType == LEX_LSS || LexType == LEX_GTR ||
        LexType == LEX_LEQ || LexType == LEX_GEQ || LexType == LEX_NEQ) {
        st_lex.push(LexType);
        gl();
        E4();
        check_op();
    }
}

void Parser::E4() {
    E5();
    while (LexType == LEX_PLUS || LexType == LEX_MINUS) {
        st_lex.push(LexType);
        gl();
        E5();
        check_op();
    }
}

void Parser::E5() {
    E6();
    while (LexType == LEX_TIMES || LexType == LEX_SLASH || LexType == LEX_PERCENT) {
        st_lex.push(LexType);
        gl();
        E6();
        check_op();
    }
}

void Parser::E6() {
    if (LexType == LEX_ID) {
        if (!TID[LexValue].get_declare()) {
            int label = LexValue;
            gl();
            if (LexType == LEX_COLON) {
                lb--;
                add_label(label);
                int pl1 = poliz.size();
                int goto_label = add_goto(label, pl1);
                if (goto_label) poliz[goto_label] = Lex(POLIZ_LABEL, pl1);
                gl();
                A();
            }
            else throw "undeclared id";
        }
        else {
            poliz.push_back(CurrentLex);
            st_lex.push(TID[LexValue].get_type());
            gl();
        }
    }
    else if (LexType == LEX_NOT) {
        gl();
        E6();
        check_not();
    }
    else if (LexType == LEX_LPAREN) {
        gl();
        E();
        if (LexType == LEX_RPAREN) gl();
        else throw "')' was expected";
    }
    else E7();
}

void Parser::E7() {
    if (LexType == LEX_MINUS) {
        gl();
        if (LexType == LEX_NUM) {
            st_lex.push(LexType);
            poliz.push_back(Lex(LexType, -LexValue));
            ltype = LexType;
            gl();
        }
        else throw "number was expected";
    }
    else if (LexType == LEX_PLUS) {
        gl();
        if (LexType == LEX_NUM) {
            st_lex.push(LEX_INT);
            poliz.push_back(Lex(LexType, LexValue));
            ltype = LexType;
            gl();
        }
        else throw "number was expected";
    }
    else if (LexType == LEX_NUM) {
        st_lex.push(LexType);
        poliz.push_back(Lex(LexType, LexValue));
        ltype = LexType;
        gl();
    }
    else if (LexType == LEX_STR) {
        st_lex.push(LexType);
        poliz.push_back(CurrentLex);
        ltype = LexType;
        gl();
    }
    else if (LexType == LEX_TRUE) {
        st_lex.push(LEX_BOOL);
        poliz.push_back(Lex(LexType, LexValue));
        ltype = LexType;
        gl();
    }
    else if (LexType == LEX_FALSE) {
        st_lex.push(LEX_BOOL);
        poliz.push_back(Lex(LexType, LexValue));
        ltype = LexType;
        gl();
    }
    else throw "unknown expression";
}

// семантические функции

void Parser::dec(type_of_lex type) {
    int i;
    while (!st_int.empty())
    {
        i = st_int.top();
        st_int.pop();
        if (TID[i].get_declare())
            throw "declared twice";
        else
        {
            TID[i].put_declare();
            TID[i].put_type(type);
        }
    }
}

void Parser::check_id()
{
    if (TID[LexValue].get_declare())
        poliz.push_back(Lex(LexType, LexValue));
    else
        throw "not declared";
}

void Parser::check_init(type_of_lex t1, type_of_lex t2)
{
    if (t1 == LEX_BOOL) {
        if (t2 != LEX_TRUE && t2 != LEX_FALSE && t2 != LEX_BOOL)
            throw "initialization error";
    }
    else if (t1 == LEX_INT && (t2 != LEX_NUM && t2 != LEX_INT))
        throw "initialization error";
    else if (t1 == LEX_STR && t2 != LEX_STR)
        throw "initialization error";
    poliz.push_back(Lex(LEX_ASSIGN));
    poliz.push_back(Lex(LEX_SEMICOLON));
}

void Parser::check_op()
{
    type_of_lex t1, t2, op, t = LEX_INT, r = LEX_BOOL;

    t2 = st_lex.top();
    st_lex.pop();
    op = st_lex.top();
    st_lex.pop();
    t1 = st_lex.top();
    st_lex.pop();
    if (t1 == LEX_NUM) t1 = LEX_INT;
    if (t2 == LEX_NUM) t2 = LEX_INT;

    if (op == LEX_PLUS || op == LEX_MINUS || op == LEX_TIMES || op == LEX_SLASH || op == LEX_PERCENT)
        r = LEX_INT;
    if (op == LEX_PLUS && t1 == LEX_STR)
        r = t = LEX_STR;
    if (op == LEX_OR || op == LEX_AND)
        t = LEX_BOOL;
    if (op == LEX_LSS || op == LEX_GTR || op == LEX_EQ || op == LEX_NEQ || op == LEX_LEQ || op == LEX_GEQ)
        if (t1 == LEX_INT)
            t = LEX_INT;
        else
            t = LEX_STR;
    if (t1 != t2 || t1 != t) {
        throw "wrong types are in operation";
    }
    st_lex.push(r);
    poliz.push_back(Lex(op));
}

void Parser::check_not()
{
    type_of_lex t = st_lex.top();
    st_lex.pop();
    if (t != LEX_BOOL)
        throw "wrong type is in not";
    else
    {
        st_lex.push(LEX_BOOL);
        poliz.push_back(Lex(LEX_NOT));
    }
}

void Parser::eq_type()
{
    type_of_lex t1 = st_lex.top();
    st_lex.pop();
    type_of_lex t2 = st_lex.top();
    st_lex.pop();

    if (t1 == LEX_NUM) t1 = LEX_INT;
    if (t2 == LEX_NUM) t2 = LEX_INT;
    if (t1 != t2) throw "wrong types are in =";
    if (t1 != LEX_BOOL && t1 != LEX_INT && t1 != LEX_STR)
        throw "wrong types are in =";
    poliz.push_back(Lex(LEX_ASSIGN));
}

void Parser::eq_bool()
{
    type_of_lex t = st_lex.top();
    st_lex.pop();
    if (t != LEX_BOOL)
        throw "expression is not boolean";
}

void Parser::check_id_in_read()
{
    if (is_struct_var(TID[LexValue].get_name()) >= 0)
        throw "can not read structure";

    if (!TID[LexValue].get_declare())
        throw "not declared";
}

class Executer {
public:
    void execute(vector<Lex>& poliz);
};

class Interpretator {
    Parser parser;
    Executer executer;
public:
    Interpretator(const char* program) : parser(program) {}
    void interpretation();
};

void Executer::execute(vector<Lex>& poliz) {
    Lex lex, lex1, lex2;
    stack<Lex> LexStack;
    int pl = 0;
    while (pl < poliz.size()) {
        lex = poliz[pl];
        cout << lex;
        switch (lex.get_type()) {

        case LEX_TRUE: case LEX_FALSE: case LEX_NUM: 
        case LEX_STR: case POLIZ_ADDRESS: case POLIZ_LABEL:
            LexStack.push(lex);
            break;

        case LEX_ID:
            if (TID[lex.get_value()].get_assign()) 
                if (TID[lex.get_value()].get_type() == LEX_STR)
                    LexStack.push(Lex(TID[lex.get_value()].get_type(), 0, TID[lex.get_value()].get_string()));
                else
                    LexStack.push(Lex(TID[lex.get_value()].get_type(), TID[lex.get_value()].get_value()));
            else throw "wrong ident";
            break;

        case LEX_NOT:
            lex1 = LexStack.top();
            LexStack.pop();
            LexStack.push(Lex(lex1.get_type(), !lex1.get_value()));
            break;

        case LEX_OR:
            lex1 = LexStack.top();
            LexStack.pop();
            lex2 = LexStack.top();
            LexStack.pop();
            LexStack.push(Lex(lex1.get_type(), lex1.get_value() || lex2.get_value()));
            break;

        case LEX_AND:
            lex1 = LexStack.top();
            LexStack.pop();
            lex2 = LexStack.top();
            LexStack.pop();
            LexStack.push(Lex(lex1.get_type(), lex1.get_value() && lex2.get_value()));
            break;

        case POLIZ_GO:
            lex1 = LexStack.top();
            LexStack.pop();
            pl = lex1.get_value() - 1;
            break;

        case POLIZ_FGO:
            lex1 = LexStack.top();
            LexStack.pop();
            if (lex1.get_value() != poliz.size()) {
                lex2 = LexStack.top();
                LexStack.pop();
                if (!lex2.get_value()) pl = lex1.get_value() - 1;
            } else pl = lex1.get_value() - 1;
            break;

        case LEX_WRITE:
            lex1 = LexStack.top();
            LexStack.pop();

            if (lex1.get_type() == LEX_INT || lex1.get_type() == LEX_NUM)
                cout << "********" << lex1.get_value() << "********" << endl;
            else if (lex1.get_type() == LEX_STR) cout << "********" << lex1.get_string() << "********" << endl;
            else {
                if (lex1.get_value()) cout << "********" << "true" << "********" << endl;
                else cout << "********" << "false" << "********" << endl;
            }
            break;

        case LEX_READ:
            lex1 = LexStack.top();
            LexStack.pop();
            if (TID[lex1.get_value()].get_type() == LEX_INT) {
                cout << TID[lex1.get_value()].get_name() << " = ";
                int num; 
                cin >> num;
                TID[lex1.get_value()].put_value(num);
            }
            else if (TID[lex1.get_value()].get_type() == LEX_BOOL) {
                cout << TID[lex1.get_value()].get_name() << " = ";
                string s;
                cin >> s;
                if (s == "true") TID[lex1.get_value()].put_value(1);
                else TID[lex1.get_value()].put_value(0);
            }
            else {
                cout << TID[lex1.get_value()].get_name() << " = ";
                string s;
                cin >> s;
                TID[lex1.get_value()].put_string(s);
            }
            TID[lex1.get_value()].put_assign();
            break;

        case LEX_PLUS:
            lex1 = LexStack.top();
            LexStack.pop();
            lex2 = LexStack.top();
            LexStack.pop();
            if (lex1.get_type() == LEX_STR) {
                LexStack.push(Lex(lex1.get_type(), 0, lex2.get_string() + lex1.get_string()));
            } 
            else
                LexStack.push(Lex(lex1.get_type(), lex1.get_value() + lex2.get_value()));
            break;

        case LEX_TIMES:
            lex1 = LexStack.top();
            LexStack.pop();
            lex2 = LexStack.top();
            LexStack.pop();
            LexStack.push(Lex(lex1.get_type(), lex1.get_value() * lex2.get_value()));
            break;

        case LEX_MINUS:
            lex1 = LexStack.top();
            LexStack.pop();
            lex2 = LexStack.top();
            LexStack.pop();
            LexStack.push(Lex(lex1.get_type(), lex2.get_value() - lex1.get_value()));
            break;

        case LEX_SLASH:
            lex1 = LexStack.top();
            LexStack.pop();
            lex2 = LexStack.top();
            LexStack.pop();
            if (lex1.get_value() != 0) 
                LexStack.push(Lex(lex1.get_type(), lex2.get_value() / lex1.get_value()));
            else throw "division on zero";
            break;

        case LEX_PERCENT:
            lex1 = LexStack.top();
            LexStack.pop();
            lex2 = LexStack.top();
            LexStack.pop();
            if (lex1.get_value() != 0) 
                LexStack.push(Lex(lex1.get_type(), lex2.get_value() % lex1.get_value()));
            else throw "division on zero";
            break;

        case LEX_EQ:
            lex1 = LexStack.top();
            LexStack.pop();
            lex2 = LexStack.top();
            LexStack.pop();
            if (lex1.get_type() == LEX_STR)
                LexStack.push(Lex(LEX_BOOL, lex1.get_string() == lex2.get_string()));
            else
                LexStack.push(Lex(LEX_BOOL, lex1.get_value() == lex2.get_value()));
            break;

        case LEX_LSS:
            lex1 = LexStack.top();
            LexStack.pop();
            lex2 = LexStack.top();
            LexStack.pop();
            if (lex1.get_type() == LEX_STR)
                LexStack.push(Lex(LEX_BOOL, lex2.get_string() < lex1.get_string()));
            else
                LexStack.push(Lex(LEX_BOOL, lex2.get_value() < lex1.get_value()));
            break;

        case LEX_GTR:
            lex1 = LexStack.top();
            LexStack.pop();
            lex2 = LexStack.top();
            LexStack.pop();
            if (lex1.get_type() == LEX_STR)
                LexStack.push(Lex(LEX_BOOL, lex2.get_string() > lex1.get_string()));
            else
                LexStack.push(Lex(LEX_BOOL, lex2.get_value() > lex1.get_value()));
            break;

        case LEX_LEQ:
            lex1 = LexStack.top();
            LexStack.pop();
            lex2 = LexStack.top();
            LexStack.pop();
            LexStack.push(Lex(LEX_BOOL, lex2.get_value() <= lex1.get_value()));
            break;

        case LEX_GEQ:
            lex1 = LexStack.top();
            LexStack.pop();
            lex2 = LexStack.top();
            LexStack.pop();
            LexStack.push(Lex(LEX_BOOL, lex2.get_value() >= lex1.get_value()));
            break;

        case LEX_NEQ:
            lex1 = LexStack.top();
            LexStack.pop();
            lex2 = LexStack.top();
            LexStack.pop();
            if (lex1.get_type() == LEX_STR)
                LexStack.push(Lex(LEX_BOOL, lex1.get_string() != lex2.get_string()));
            else
                LexStack.push(Lex(LEX_BOOL, lex1.get_value() != lex2.get_value()));
            break;

        case LEX_ASSIGN:
            lex1 = LexStack.top();
            LexStack.pop();
            lex2 = LexStack.top();
            LexStack.pop();
            if (lex1.get_type() == LEX_STR) {
                TID[lex2.get_value()].put_string(lex1.get_string());
                LexStack.push(lex1);
            }
            else if (lex1.get_type() == POLIZ_ADDRESS) {
                TID[lex2.get_value()].put_value(TID[lex1.get_value()].get_value());
                LexStack.push(lex2);
            }
            else {
                TID[lex2.get_value()].put_value(lex1.get_value());
                LexStack.push(lex1);
            }
            TID[lex2.get_value()].put_assign();
            break;

        case LEX_SEMICOLON:
            if (!LexStack.empty())
                LexStack.pop();
            break;

        default:
            throw ":(";
            break;
        }
        pl += 1;
    };
}

void Interpretator::interpretation() {
    parser.analyze();
    executer.execute(parser.poliz);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "usage: ./interpreter program.txt" << endl;
        return 1;
    }
    try {
        Scanner scanner(argv[1]);
        cout << "start analyzing\n" << endl;
        Interpretator interpretator(argv[1]);
        interpretator.interpretation();
        cout << "ok" << endl;
    }
    catch (char c) {
        if (c == EOF) cout << "unexpected end of file" << endl;
        else cout << "unexpected char " << c << endl;
        return 1;
    }
    catch (const char* str) {
        cout << str << endl;
        return 1;
    }
    catch (Lex l) {
        cout << "unexpected lex" << endl;
        return 1;
    }
}