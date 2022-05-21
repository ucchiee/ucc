#include <iostream>
using namespace std;

namespace Lexer {

enum class Kind : char {
    // token
    end,
    tk_id,
    tk_int,
    tk_char,
    tk_string,
    // keyword
    kw_int,
    kw_char,
    kw_void,
    kw_if,
    kw_else,
    // kw_for,  // use while for a while
    kw_while,
    kw_return,
    // op
    op_eq,
    op_and,
    op_or,
    // others
    plus = '+',
    minus = '-',
    mul = '*',
    div = '/',
    mod = '%',
    assign = '=',
    ampr = '&',
    excl = '!',
    lp = '(',
    rp = ')',
    lsb = '[',
    rsb = ']',
    rcb = '{',
    lcb = '}',
    lab = '<',
    rab = '>',
    colon = ':',
    semicolon = ';',

};

struct Token {
    Kind kind;
    string string_value;
    double number_value;
};

class Token_stream {
   public:
    Token_stream(istream& s) : ip{&s}, owns{false}, ct{Kind::end} {}
    Token_stream(istream* p) : ip{p}, owns{true}, ct{Kind::end} {}
    ~Token_stream() { close(); }

    Token get();
    const Token& current() { return ct; }

    void set_input(istream& s) {
        close();
        ip = &s;
        owns = false;
    }

    void set_input(istream* p) {
        close();
        ip = p;
        owns = true;
    }

   private:
    void close() {
        if (owns) delete ip;
    }
    istream* ip;
    bool owns;
    Token ct{Kind::end};
};

}  // namespace Lexer
