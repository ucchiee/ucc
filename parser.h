#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "ast.h"
#include "lexer.h"
#include "type.h"

namespace parser {

class Parser {
 public:
  Parser(lexer::TokenStream &ts);
  Parser(Parser &&) = default;
  Parser(const Parser &) = default;
  Parser &operator=(Parser &&) = delete;
  Parser &operator=(const Parser &) = delete;
  ~Parser() = default;

  std::unique_ptr<ast::Node> program();
  std::shared_ptr<type::Type> type_specifier();
  std::pair<lexer::Token, std::shared_ptr<type::Type>> declarator(
      std::shared_ptr<type::Type> type, bool global = false);
  std::pair<lexer::Token, std::shared_ptr<type::Type>> param_decl();
  std::unique_ptr<ast::Node> register_args_as_local(
      std::pair<lexer::Token, std::shared_ptr<type::Type>>);
  std::unique_ptr<ast::Node> stmt();
  std::unique_ptr<ast::Node> compound_stmt();
  std::unique_ptr<ast::Node> expr();
  std::unique_ptr<ast::Node> assign();
  std::unique_ptr<ast::Node> equality();
  std::unique_ptr<ast::Node> relational();
  std::unique_ptr<ast::Node> add(bool convert_arr = true);
  std::unique_ptr<ast::Node> mul(bool convert_arr = true);
  std::unique_ptr<ast::Node> unary(bool convert_arr = true);
  std::unique_ptr<ast::Node> primary(bool convert_arr = true);

  std::pair<std::shared_ptr<type::Type>, std::shared_ptr<type::Type>>
      convert_type(std::shared_ptr<type::Type>, std::shared_ptr<type::Type>);
  std::shared_ptr<type::Type> check_and_merge_type(
      std::shared_ptr<type::Type>, std::shared_ptr<type::Type>,
      lexer::Kind op_kind = lexer::Kind::end);

 private:
  lexer::TokenStream &m_ts;
};

std::vector<std::unique_ptr<ast::Node>> create_rc_delete_calls();

}  // namespace parser
