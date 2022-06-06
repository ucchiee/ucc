#pragma once
#include <initializer_list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Ast {
enum class NodeKind {
  nd_add,
  nd_sub,
  nd_mul,
  nd_div,
  nd_num,
  nd_return,
  nd_assign,
  nd_if,
  nd_ifelse,
  nd_eq,
  nd_neq,
  nd_le,
  nd_leq,
  nd_lval,
};

struct Node {
  NodeKind kind;
  std::vector<std::unique_ptr<Node>> child_vec;
  int val;  // used by nd_num, or label number
  int offset;

  void add_child(std::unique_ptr<Node> node);
};

std::unique_ptr<Ast::Node> create_node(
    Ast::NodeKind kind, std::unique_ptr<Ast::Node> first = nullptr,
    std::unique_ptr<Ast::Node> second = nullptr);
std::unique_ptr<Node> create_num(int num);

void dump_ast(std::unique_ptr<Ast::Node> node, int depth);

}  // namespace Ast
