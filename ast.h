#pragma once
#include <memory>

namespace Ast {
enum class NodeKind {
  nd_add,
  nd_sub,
  nd_mul,
  nd_div,
  nd_num,
  nd_eq,
  nd_neq,
  nd_le,
  nd_leq,
};

struct Node {
  NodeKind kind;
  std::unique_ptr<Node> left;
  std::unique_ptr<Node> right;
  int val;
};

std::unique_ptr<Node> create_node(NodeKind kind, std::unique_ptr<Node> left,
                                  std::unique_ptr<Node> right);
std::unique_ptr<Node> create_num(int num);

}  // namespace Ast
