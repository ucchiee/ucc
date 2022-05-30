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
  int number_value;
};

class TokenStream {
 public:
  TokenStream(std::istream &s);
  ~TokenStream() = default;

  const Token &current();
  bool consume(Kind kind);
  int expect_number();

 private:
  std::istream &is;
  Token current_token{Kind::end};

  Token get();
};
}  // namespace Lexer
