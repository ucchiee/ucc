#pragma once
#include <memory>
#include <vector>

#include "lexer.h"
#include "lval.h"

namespace ast {
enum class NodeKind {
  nd_blank,
  nd_add,
  nd_sub,
  nd_mul,
  nd_div,
  nd_deref,
  nd_addr,
  nd_num,
  nd_return,
  nd_assign,
  nd_add_into,
  nd_if,
  nd_ifelse,
  nd_while,
  nd_for,
  nd_compound,
  nd_funcall,
  nd_funcdef,
  nd_param_decl,
  nd_eq,
  nd_ne,
  nd_lt,
  nd_le,
  nd_lval,
};

struct Node {
  NodeKind kind;
  std::vector<std::unique_ptr<Node>> child_vec;
  int val;
  int offset;        // variable
  lexer::Token tok;  // funcall, funcdef
  int total_size;
  std::vector<std::shared_ptr<symbol::LVal>> local;

  void add_child(std::unique_ptr<Node> node);
};

std::unique_ptr<ast::Node> create_node(
    ast::NodeKind kind, std::unique_ptr<ast::Node> first = nullptr,
    std::unique_ptr<ast::Node> second = nullptr);
std::unique_ptr<Node> create_num(int num);

void dump_ast(std::unique_ptr<ast::Node> node, int depth);

}  // namespace ast
