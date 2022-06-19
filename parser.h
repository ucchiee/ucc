#pragma once

#include <memory>
#include <vector>

#include "ast.h"
#include "lexer.h"

namespace parser {

class Parser {
 public:
  Parser(lexer::TokenStream &ts);
  Parser(Parser &&) = default;
  Parser(const Parser &) = default;
  Parser &operator=(Parser &&) = delete;
  Parser &operator=(const Parser &) = delete;
  ~Parser() = default;

  void program();
  std::unique_ptr<ast::Node> funcdef();
  std::unique_ptr<ast::Node> param_decl();
  std::unique_ptr<ast::Node> stmt();
  std::unique_ptr<ast::Node> compound_stmt();
  std::unique_ptr<ast::Node> expr();
  std::unique_ptr<ast::Node> assign();
  std::unique_ptr<ast::Node> equality();
  std::unique_ptr<ast::Node> relational();
  std::unique_ptr<ast::Node> add();
  std::unique_ptr<ast::Node> mul();
  std::unique_ptr<ast::Node> unary();
  std::unique_ptr<ast::Node> primary();

 private:
  lexer::TokenStream &m_ts;
};

}  // namespace parser
