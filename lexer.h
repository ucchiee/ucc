#include <iostream>

namespace Lexer {

enum class Kind : int {
  // アスキー文字はそのまま扱う
  // token
  end = 128,
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
  op_eq,   // ==
  op_and,  // &&
  op_or,   // ||
};

struct Token {
  Kind kind;
  std::string string_value;
  double number_value;
};

class TokenStream {
 public:
  TokenStream(std::istream &s) : is{s}, current_token{Kind::end} {}
  ~TokenStream() = default;

  Token get();
  const Token &current() { return current_token; }

 private:
  std::istream &is;
  Token current_token{Kind::end};
};

}  // namespace Lexer
