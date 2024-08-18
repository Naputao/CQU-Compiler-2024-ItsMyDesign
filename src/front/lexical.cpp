#include "front/lexical.h"

#include <cassert>
#include <map>
#include <string>

#define TODO assert(0 && "todo")

// #define DEBUG_DFA
// #define DEBUG_SCANNER

std::string frontend::toString(State s)
{
    switch (s) {
        case State::Empty: return "Empty";
        case State::Ident: return "Ident";
        case State::IntLiteral: return "IntLiteral";
        case State::FloatLiteral: return "FloatLiteral";
        case State::op: return "op";
        default: assert(0 && "invalid State");
    }
    return "";
}

std::set<std::string> frontend::keywords = {
    "const", "int",      "float", "if",     "else",
    "while", "continue", "break", "return", "void"};

frontend::DFA::DFA() : cur_state(frontend::State::Empty), cur_str() {}

frontend::DFA::~DFA() {}

bool frontend::DFA::next(char input, Token& buf)
{
#ifdef DEBUG_DFA
#    include <iostream>
    std::cout << "in state [" << toString(cur_state) << "], input = \'" << input
              << "\', str = " << cur_str << "\t";
#endif
    switch (this->cur_state) {
        case State::Empty:
            switch (input) {
                case 'A' ... 'Z':
                case 'a' ... 'z':
                case '_':
                    this->cur_state = State::Ident;
                    this->cur_str   = input;
                    return false;
                case '0' ... '9':
                    this->cur_state = State::IntLiteral;
                    this->cur_str   = input;
                    return false;
                case '.':
                    this->cur_state = State::FloatLiteral;
                    this->cur_str += input;
                    return false;
                case '+':
                case '-':
                case '*':
                case '/':
                case '%':
                case '<':
                case '>':
                case ':':
                case '=':
                case ';':
                case ',':
                case '(':
                case ')':
                case '[':
                case ']':
                case '{':
                case '}':
                case '!':
                case '&':
                case '|':
                    this->cur_state = State::op;
                    this->cur_str   = input;
                    return false;
                default: this->cur_state = State::Empty; return false;
            }

        case State::Ident:
            switch (input) {
                case 'A' ... 'Z':
                case 'a' ... 'z':
                case '_':
                case '0' ... '9':
                    this->cur_state = frontend::State::Ident;
                    this->cur_str += input;
                    return false;
                case '+':
                case '-':
                case '*':
                case '/':
                case '%':
                case '<':
                case '>':
                case ':':
                case '=':
                case ';':
                case ',':
                case '(':
                case ')':
                case '[':
                case ']':
                case '{':
                case '}':
                case '!':
                case '&':
                case '|':
                    this->cur_state = State::op;
                    buf.type        = get_Ident_type(this->cur_str);
                    buf.value       = this->cur_str;
                    this->cur_str   = input;
                    return true;
                default:
                    this->cur_state = frontend::State::Empty;
                    buf.type        = get_Ident_type(this->cur_str);
                    buf.value       = this->cur_str;
                    this->cur_str   = input;
                    return true;
            }
        case State::IntLiteral:
            switch (input) {
                case '0' ... '9':
                case 'x':
                case 'a' ... 'f':
                case 'A' ... 'F':
                    this->cur_state = State::IntLiteral;
                    this->cur_str += input;
                    return false;
                case '.':
                    this->cur_state = State::FloatLiteral;
                    this->cur_str += input;
                    return false;
                case '+':
                case '-':
                case '*':
                case '/':
                case '%':
                case '<':
                case '>':
                case ':':
                case '=':
                case ';':
                case ',':
                case '(':
                case ')':
                case '[':
                case ']':
                case '{':
                case '}':
                case '!':
                case '&':
                case '|':
                    this->cur_state = State::op;
                    buf.type        = frontend::TokenType::INTLTR;
                    buf.value       = this->cur_str;
                    this->cur_str   = input;
                    return true;
                default:
                    this->cur_state = State::Empty;
                    buf.type        = frontend::TokenType::INTLTR;
                    buf.value       = this->cur_str;
                    this->cur_str   = input;
                    return true;
            }
        case State::FloatLiteral:
            switch (input) {
                case '0' ... '9': this->cur_str += input; return false;
                case '+':
                case '-':
                case '*':
                case '/':
                case '%':
                case '<':
                case '>':
                case ':':
                case '=':
                case ';':
                case ',':
                case '(':
                case ')':
                case '[':
                case ']':
                case '{':
                case '}':
                case '!':
                case '&':
                case '|':
                    this->cur_state = State::op;
                    buf.type        = frontend::TokenType::FLOATLTR;
                    buf.value       = this->cur_str;
                    this->cur_str   = input;
                    return true;
                default:
                    this->cur_state = State::Empty;
                    buf.type        = frontend::TokenType::FLOATLTR;
                    buf.value       = this->cur_str;
                    this->cur_str   = input;
                    return true;
            }
        case State::op:
            switch (input) {
                case 'A' ... 'Z':
                case 'a' ... 'z':
                case '_':
                    this->cur_state = State::Ident;
                    buf.type        = get_op_type(this->cur_str);
                    buf.value       = this->cur_str;
                    this->cur_str   = input;
                    return true;
                case '0' ... '9':
                    this->cur_state = State::IntLiteral;
                    buf.type        = get_op_type(this->cur_str);
                    buf.value       = this->cur_str;
                    this->cur_str   = input;
                    return true;
                case '.':
                    this->cur_state = State::FloatLiteral;
                    buf.type        = get_op_type(this->cur_str);
                    buf.value       = this->cur_str;
                    this->cur_str   = input;
                    return true;
                case '+':
                case '-':
                case '*':
                case '/':
                case '%':
                case '<':
                case '>':
                case ':':
                case ';':
                case ',':
                case '(':
                case ')':
                case '[':
                case ']':
                case '{':
                case '}':
                case '!':
                    this->cur_state = State::op;
                    buf.type        = get_op_type(this->cur_str);
                    buf.value       = this->cur_str;
                    this->cur_str   = input;
                    return true;
                case '&':
                    if (this->cur_str == "&") {
                        this->cur_str += input;
                        return false;
                    }
                    this->cur_state = State::op;
                    buf.type        = get_op_type(this->cur_str);
                    buf.value       = this->cur_str;
                    this->cur_str   = input;
                    return true;
                case '|':
                    if (this->cur_str == "|") {
                        this->cur_str += input;
                        return false;
                    }
                    this->cur_state = State::op;
                    buf.type        = get_op_type(this->cur_str);
                    buf.value       = this->cur_str;
                    this->cur_str   = input;
                    return true;
                case '=':
                    if (this->cur_str == "<" || this->cur_str == ">" ||
                        this->cur_str == "=" || this->cur_str == "!") {
                        this->cur_str += input;
                        return false;
                    }
                    this->cur_state = State::op;
                    buf.type        = get_op_type(this->cur_str);
                    buf.value       = this->cur_str;
                    this->cur_str   = input;
                    return true;
                default:
                    this->cur_state = State::Empty;
                    buf.type        = get_op_type(this->cur_str);
                    buf.value       = this->cur_str;
                    this->cur_str   = input;
                    return true;
            }
        default: assert(0 && "DFA Wrong!!!");
    }
    std::string ret = buf.value;
#ifdef DEBUG_DFA
    std::cout << "next state is [" << toString(cur_state)
              << "], next str = " << cur_str << "\t, ret = " << ret
              << std::endl;
#endif
    return false;
}

void frontend::DFA::reset()
{
    cur_state = State::Empty;
    cur_str   = "";
}

frontend::Scanner::Scanner(std::string filename) : fin(filename)
{
    if (!fin.is_open()) {
        assert(0 && "in Scanner constructor, input file cannot open");
    }
}

frontend::Scanner::~Scanner()
{
    fin.close();
}

std::vector<frontend::Token> frontend::Scanner::run()
{
    frontend::DFA                dfa;
    std::vector<frontend::Token> tk_stream;
    frontend::Token              tk;
    int                          state = 0;
    for (char c = this->fin.get(); c != -1; c = this->fin.get()) {
        switch (state)  // deal with "//"
        {
            case 0:
                state = (c == '/') ? 1 : 0;
                if (c != '/' && dfa.next(c, tk))
                    tk_stream.push_back(tk);
                break;
            case 1:
                state = (c == '*') ? 3 : 0;
                state = (c == '/') ? 2 : state;
                if (c != '/' && c != '*') {
                    if (dfa.next('/', tk))
                        tk_stream.push_back(tk);
                    if (dfa.next(c, tk))
                        tk_stream.push_back(tk);
                }
                break;
            case 2:
                // if c is '\n' reset and input '\n'
                state = (c == 10) ? 0 : 2;
                break;  // if c is not '\n' continue ignoring
            case 3: state = (c == '*') ? 4 : 3; break;
            case 4: state = (c == '/') ? 0 : 3; break;
            default: break;
        }
    }
    if (dfa.next((char)0, tk))
        tk_stream.push_back(tk);
#ifdef DEBUG_SCANNER
#    include <iostream>
    std::cout << "token: " << toString(tk.type) << "\t" << tk.value
              << std::endl;
#endif
    return tk_stream;
}

frontend::TokenType frontend::get_op_type(std::string S)
{
    if (S == "+")
        return frontend::TokenType::PLUS;
    if (S == "-")
        return frontend::TokenType::MINU;
    if (S == "*")
        return frontend::TokenType::MULT;
    if (S == "/")
        return frontend::TokenType::DIV;
    if (S == "%")
        return frontend::TokenType::MOD;
    if (S == "<")
        return frontend::TokenType::LSS;
    if (S == ">")
        return frontend::TokenType::GTR;
    if (S == ":")
        return frontend::TokenType::COLON;
    if (S == "=")
        return frontend::TokenType::ASSIGN;
    if (S == ";")
        return frontend::TokenType::SEMICN;
    if (S == ",")
        return frontend::TokenType::COMMA;
    if (S == "(")
        return frontend::TokenType::LPARENT;
    if (S == ")")
        return frontend::TokenType::RPARENT;
    if (S == "[")
        return frontend::TokenType::LBRACK;
    if (S == "]")
        return frontend::TokenType::RBRACK;
    if (S == "{")
        return frontend::TokenType::LBRACE;
    if (S == "}")
        return frontend::TokenType::RBRACE;
    if (S == "!")
        return frontend::TokenType::NOT;
    if (S == "<=")
        return frontend::TokenType::LEQ;
    if (S == ">=")
        return frontend::TokenType::GEQ;
    if (S == "==")
        return frontend::TokenType::EQL;
    if (S == "!=")
        return frontend::TokenType::NEQ;
    if (S == "&&")
        return frontend::TokenType::AND;
    if (S == "||")
        return frontend::TokenType::OR;
    assert(0 && "Wrong OP!!!");
    return frontend::TokenType::OR;
}

frontend::TokenType frontend::get_Ident_type(std::string S)
{
    if (S == "const")
        return frontend::TokenType::CONSTTK;
    if (S == "void")
        return frontend::TokenType::VOIDTK;
    if (S == "int")
        return frontend::TokenType::INTTK;
    if (S == "float")
        return frontend::TokenType::FLOATTK;
    if (S == "if")
        return frontend::TokenType::IFTK;
    if (S == "else")
        return frontend::TokenType::ELSETK;
    if (S == "while")
        return frontend::TokenType::WHILETK;
    if (S == "continue")
        return frontend::TokenType::CONTINUETK;
    if (S == "break")
        return frontend::TokenType::BREAKTK;
    if (S == "return")
        return frontend::TokenType::RETURNTK;
    return frontend::TokenType::IDENFR;
}