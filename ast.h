#pragma once
#include <memory>
#include <vector>

#include "lexer.h"
#include "symtable.h"
#include "type.h"

namespace ast {
enum class NodeKind {
  nd_program,
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
  nd_funcdecl,
  nd_arg_decl,
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
  std::shared_ptr<type::Type> type;  // for decl and expr
  int offset;                        // variable
  lexer::Token tok;                  // funcall, funcdef, ident
  int total_size;                    // funcdef
  int arg_idx;
  std::shared_ptr<symbol::Symbol> local;

  void add_child(std::unique_ptr<Node> node);
};

std::unique_ptr<ast::Node> create_node(
    ast::NodeKind kind, std::shared_ptr<type::Type> type,
    std::unique_ptr<ast::Node> first = nullptr,
    std::unique_ptr<ast::Node> second = nullptr);
std::unique_ptr<ast::Node> create_node(
    ast::NodeKind kind, std::unique_ptr<ast::Node> first = nullptr,
    std::unique_ptr<ast::Node> second = nullptr);
std::unique_ptr<Node> create_num(int num,
                                 std::shared_ptr<type::Type> type = nullptr);

void dump_ast(std::unique_ptr<ast::Node> node, int depth);

}  // namespace ast
