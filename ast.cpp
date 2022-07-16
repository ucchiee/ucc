#include "ast.h"

#include <iostream>

#include "enum_magic.hpp"

using namespace std;

void ast::Node::add_child(unique_ptr<ast::Node> child) {
  this->child_vec.push_back(move(child));
}

std::unique_ptr<ast::Node> ast::create_node(ast::NodeKind kind,
                                            std::unique_ptr<ast::Node> first,
                                            std::unique_ptr<ast::Node> second) {
  unique_ptr<ast::Node> node = make_unique<ast::Node>();
  node->kind = kind;
  if (first) node->child_vec.push_back(move(first));
  if (second) node->child_vec.push_back(move(second));
  return node;
}
unique_ptr<ast::Node> ast::create_num(int val) {
  unique_ptr<ast::Node> node = ast::create_node(ast::NodeKind::nd_num);

  node->val = val;
  node->type = type::create_type(type::Kind::type_int, 4);
  return move(node);
}

void ast::dump_ast(std::unique_ptr<ast::Node> node, int depth) {
  string space;
  for (int i = 0; i < depth; i++) space += " ";
  cerr << space << magic_enum::enum_name(node->kind) << endl;

  for (auto &&child : node->child_vec) {
    ast::dump_ast(move(child), depth + 1);
  }
}
