#include "lexical.h"
#include "syntax.h"

extern Table_ident TID;

template <class T, int max_size >
Stack <T, max_size>::Stack() {
    s = new T[size = max_size];
    top = 0;
}

template <class T, int max_size >
Stack <T, max_size>::~Stack() {
    delete[] s;
}

template <class T, int max_size >
void Stack <T, max_size >::reset() { top = 0; }

template <class T, int max_size >
bool Stack <T, max_size >::is_empty() { return top == 0; }

template <class T, int max_size >
bool Stack <T, max_size >::is_full() { return top == max_size; }

template <class T, int max_size >
void Stack <T, max_size >::print() {
    for (int i = 0; i < top; i++)
        cout << i << " " << s[i] << endl;
}

template <class T, int max_size >
void Stack <T, max_size >::push(T i)
{
    if (!is_full()) {
        s[top] = i;
        ++top;
    }
    else
        throw "Stack_is_full";
}

template <class T, int max_size >
T Stack <T, max_size >::pop()
{
    if (!is_empty())
    {
        --top;
        return s[top];
    }
    else
        throw "Stack_is_empty";
}

template <class T, int max_size >
T Stack <T, max_size >::get_top()
{
    return s[top - 1];
}

Poliz::Poliz(int max_size) {
    p = new Lex[size = max_size];
    pos = 0;
}

Poliz::~Poliz() {
    delete[] p;
}
void Poliz::put_lex(Lex l) { p[pos] = l; ++pos; }
void Poliz::put_lex(Lex l, int place) { p[place] = l; }
void Poliz::blank() { ++pos; }
int Poliz::get_pos() { return pos; };

Lex & Poliz::operator[] (int index)
{
    if (index > size)
        throw "POLIZ:out of array";
    else
        if (index > pos)
            throw "POLIZ:indefinite element of array";
        else
            return p[index];
};

void Poliz::print() {
    for (int i = 0; i < pos; i++)
        cout << p[i];
}

Parser::Parser(const char * program) : scan(program), prog(1000) {}

void Parser::gl() {
    curr_l = scan.gl();
    curr_t = curr_l.get_type();
    curr_v = curr_l.get_val();
    cout << curr_l << endl;
}
void Parser::analyze() {
    cout << "starting analyze: " << endl;
    P();
    prog.print();
    cout << endl << "Parser success" << endl;
}

void Parser::P() {
        gl();
        D();
        B();
        if (curr_t != LEX_END)
            throw curr_l;
}

void Parser::D() {
        st_int.reset();
        D1();
        while (curr_t == LEX_SEMICOLON){
            gl();
            D1();
        }
}

void Parser::D1() {
        if (curr_t == LEX_FLOAT || curr_t == LEX_INT){
            type = curr_t;
            gl();
            if (curr_t == LEX_ID){
                st_int.push(curr_v);
                gl();
            }else
                throw curr_l;
            while (curr_t == LEX_COMMA){
                gl();
                if (curr_t == LEX_ID){
                    st_int.push(curr_v);
                    gl();
                }else
                    throw curr_l;
            }
            dec();
       }
}

void Parser::B() {
    if (curr_t == LEX_BEGIN ){
        gl();
        S();
        while (curr_t == LEX_SEMICOLON )
        {
            gl();
            S();
        }
        if (curr_t == LEX_END )
            return;
        else
            throw curr_l;
    }
}

void Parser::S() {
    if (curr_t == LEX_ID ){
        check_id();
        prog.put_lex(Lex(POLIZ_ADDRESS,curr_v));
        gl();
        if (curr_t == LEX_ASSIGN ){
            gl();
            E();
            eq_type();
            prog.put_lex (Lex (LEX_ASSIGN) );
        }
        else
            throw curr_l;
    } //assign-end
    else if (curr_t == LEX_PRINT){
        gl();
        if (curr_t == LEX_ID || curr_t == LEX_INUM || curr_t == LEX_FNUM)
            prog.put_lex(curr_l);
        else
            throw "syntax: LEX_PRINT";
        prog.put_lex(LEX_PRINT);
        gl();
    }
    else
        B();
}

void Parser::E() {
    E1();
    if (curr_t == LEX_SG || curr_t == LEX_SL || curr_t == LEX_SGE ||
        curr_t == LEX_SLE || curr_t == LEX_SEQ || curr_t == LEX_SNQ)
    {
        st_lex.push(curr_l);
        gl();
        E1();
        check_op();
    }
}

void Parser::E1() {
    T();
    while (curr_t == LEX_PLUS || curr_t == LEX_MINUS || curr_t == LEX_OR) {
        st_lex.push(curr_l);
        gl();
        T();
        check_op();
    }
}

void Parser::T() {
    F();
    while (curr_t == LEX_MUL || curr_t == LEX_DIV || curr_t == LEX_AND) {
        st_lex.push(curr_l);
        gl();
        F();
        check_op();
    }
}

void Parser::F() {
    if (curr_t == LEX_ID) {
        check_id();
        prog.put_lex(Lex(LEX_ID, curr_v));
        gl();
    }
    else if (curr_t == LEX_INUM || curr_t == LEX_FNUM) {
        st_lex.push(curr_l);
        prog.put_lex(curr_l);
        gl();
    }
    else if (curr_t == LEX_TRUE) {
        st_lex.push(LEX_BOOL);
        prog.put_lex(Lex(LEX_TRUE, 1));
        gl();
    }
    else if (curr_t == LEX_FALSE) {
        st_lex.push(LEX_BOOL);
        prog.put_lex(Lex(LEX_FALSE, 0));
        gl();
    }
    else if (curr_t == LEX_NOT) {
        gl();
        F();
        //check_not();
    }
    else if (curr_t == LEX_LPAREN) {
        gl();
        E();
        if (curr_t == LEX_RPAREN)
            gl();
        else
            throw curr_l;
    }
    else
        throw curr_l;
}



void Parser::dec()
{
    int i;
    while (!st_int.is_empty())
    {
        i = st_int.pop();
        if (TID.var[i].declare)
            throw "Redeclaration";
        else
        {
            TID.var[i].declare = true;
            TID.var[i].set_type(type); //type_var
            cout << "declared : " << TID.var[i].name << endl;
        }
    }
}

void Parser::check_id()
{
    int c_val = (int)curr_v;
    if (TID.var[c_val].declare)
        st_lex.push(Lex(TID.var[c_val].get_type(), curr_v));
    else
        throw "Not declared";
}

void Parser::eq_type()
{
    st_lex.print();
    Lex t1 = st_lex.pop();
    Lex t2 = st_lex.pop();
    if (t1.get_type() == LEX_FLOAT && t2.get_type() == LEX_INT)
        throw "error: float to int";
}

void Parser::check_op()
{
    type_lex oper;
    Lex t1, t2, op;
    t2 = st_lex.pop();
    op = st_lex.pop();
    t1 = st_lex.pop();
    oper = op.get_type();
    if (oper ==LEX_PLUS || oper == LEX_MINUS
            || oper ==LEX_MUL || oper ==LEX_DIV){
        if (t1.get_type() == LEX_FLOAT || t2.get_type() == LEX_FLOAT)
            st_lex.push(LEX_FLOAT);
        else
            st_lex.push(LEX_INT);
    }else
        throw "wrong types are in operation";
     prog.put_lex(op);
}

void Parser::check_not ()
{
    if (st_lex.pop().get_type() != LEX_BOOL)
        throw "wrong type is in not";
    else
        st_lex.push (LEX_BOOL);
}

void Executer::execute(Poliz & prog) {
    Stack< Lex, 100 > args;
    int i, index = 0;
    Lex lex1, lex2, lex3;
    int size = prog.get_pos();

    while (index < size) {
        args.print();
        cout << endl;
        curr_l = prog[index];
        switch (curr_l.get_type())
        {
        case LEX_INUM:
        case LEX_FNUM:
        case POLIZ_ADDRESS:
            args.push(curr_l);
            break;
        case LEX_ID:
            i = (int)curr_l.get_val();
            if (TID.var[i].assign) {
                args.push(curr_l);
                break;
            }
            else
                throw "POLIZ: indefinite identifier";
        case LEX_PLUS:
            args.push(args.pop() + args.pop());
            break;
        case LEX_MINUS:
            lex1 = args.pop();
            args.push(args.pop() - lex1);
            break;
        case LEX_MUL:
            args.push(args.pop() * args.pop());
            break;
        case LEX_DIV:
            lex1 = args.pop();
            args.push(args.pop() / lex1);
            break;
        case LEX_ASSIGN:
            lex1 = args.pop();
            //cout << "lvalue : " << lex1.v_lex.d << endl;
            lex2 = args.pop();
            lex3 = to_const(lex1);
            i = (int)lex2.get_val();
            if (lex3.get_type() == LEX_FNUM && TID.var[i].get_type() == LEX_INT)
                throw "impilcit float to int";
            //cout << "rvalue : " << lex3.v_lex.d << endl;
            TID.var[i].set_val(lex3.get_val());
            TID.var[i].assign = true;
            break;
        case LEX_PRINT:
            lex1 = args.pop();
            lex1 = to_const(lex1);
            if (lex1.get_type() == LEX_INUM)
                cout << (int)lex1.get_val() << endl;
            else
                cout << lex1.get_val() << endl;
            break;
        default:
            throw "POLIZ: unexpected elem";
        }
        ++index;
    }
    cout << "Finish of executing!!!" << endl;
}


void Interpretator::interpretation()
{
    pars.analyze();
    cout << "Start execute: " << endl;
    E.execute(pars.prog);
}