#include <iostream>
#include <vector>

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
  char* lexeme_string;
  int len;
  int lexeme_number;
};

class TokenStream {
 public:
  TokenStream(char *program);
  ~TokenStream() = default;

  bool consume(Kind kind);
  int expect_number();

 private:
  char *program;
  std::vector<Token> token_vec;
  int current_token_idx;

  void tokeninze();
  const Token &current();
};
}  // namespace Lexer
