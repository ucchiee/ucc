#pragma once

#include <memory>
#include <vector>

#include "ast.h"
#include "lexer.h"

namespace parser {

struct LVal {
  char *name;
  int len;
  int offset;
};

class Parser {
 public:
  Parser(Lexer::TokenStream &ts);
  Parser(Parser &&) = default;
  Parser(const Parser &) = default;
  Parser &operator=(Parser &&) = delete;
  Parser &operator=(const Parser &) = delete;
  ~Parser() = default;

  void program();
  std::unique_ptr<Ast::Node> stmt();
  std::unique_ptr<Ast::Node> expr();
  std::unique_ptr<Ast::Node> assign();
  std::unique_ptr<Ast::Node> equality();
  std::unique_ptr<Ast::Node> relational();
  std::unique_ptr<Ast::Node> add();
  std::unique_ptr<Ast::Node> mul();
  std::unique_ptr<Ast::Node> unary();
  std::unique_ptr<Ast::Node> primary();

 private:
  Lexer::TokenStream &m_ts;
};

std::shared_ptr<parser::LVal> find_lval(Lexer::Token token);

}  // namespace parser
