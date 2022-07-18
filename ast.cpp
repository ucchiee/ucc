#include "ast.h"

#include <iostream>

#include "enum_magic.hpp"
#include "type.h"

using namespace std;

namespace ast {

void Node::add_child(unique_ptr<Node> child) {
  this->child_vec.push_back(move(child));
}

std::unique_ptr<ast::Node> create_node(ast::NodeKind kind,
                                       std::shared_ptr<type::Type> type,
                                       std::unique_ptr<ast::Node> first,
                                       std::unique_ptr<ast::Node> second) {
  auto node = create_node(kind, move(first), move(second));
  node->type = type;
  return node;
}

unique_ptr<Node> create_node(NodeKind kind, unique_ptr<Node> first,
                             unique_ptr<Node> second) {
  unique_ptr<Node> node = make_unique<Node>();
  node->kind = kind;
  if (first) node->child_vec.push_back(move(first));
  if (second) node->child_vec.push_back(move(second));
  return node;
}

unique_ptr<Node> create_num(int val, shared_ptr<type::Type> type) {
  if (type == nullptr) type = type::create_int();
  unique_ptr<Node> node = create_node(NodeKind::nd_num, type);

  node->val = val;
  return move(node);
}

void dump_ast(unique_ptr<Node> node, int depth) {
  string space;
  for (int i = 0; i < depth; i++) space += " ";
  cerr << space << magic_enum::enum_name(node->kind) << endl;

  for (auto &&child : node->child_vec) {
    dump_ast(move(child), depth + 1);
  }
}
}  // namespace ast
