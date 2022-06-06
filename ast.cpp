#include "ast.h"

#include <initializer_list>
#include <memory>
#include <utility>

using namespace std;

void Ast::Node::add_child(unique_ptr<Ast::Node> child) {
  this->child_vec.push_back(move(child));
}

std::unique_ptr<Ast::Node> Ast::create_node(Ast::NodeKind kind,
                                            std::unique_ptr<Ast::Node> first,
                                            std::unique_ptr<Ast::Node> second) {
  unique_ptr<Ast::Node> node = make_unique<Ast::Node>();

  node->kind = kind;
  if (first) node->child_vec.push_back(move(first));
  if (second) node->child_vec.push_back(move(second));
  return node;
}
unique_ptr<Ast::Node> Ast::create_num(int val) {
  unique_ptr<Ast::Node> node = Ast::create_node(Ast::NodeKind::nd_num);

  node->val = val;
  return move(node);
}
