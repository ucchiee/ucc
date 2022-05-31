#pragma once

#include <memory>

#include "ast.h"
#include "lexer.h"

namespace parser {

class Parser {
 public:
  Parser(Lexer::TokenStream &ts);
  Parser(Parser &&) = default;
  Parser(const Parser &) = default;
  Parser &operator=(Parser &&) = delete;
  Parser &operator=(const Parser &) = delete;
  ~Parser() = default;

  std::unique_ptr<Ast::Node> expr();
  std::unique_ptr<Ast::Node> mul();
  std::unique_ptr<Ast::Node> unary();
  std::unique_ptr<Ast::Node> primary();

 private:
  Lexer::TokenStream &m_ts;
};

}  // namespace parser
