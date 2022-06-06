#include "ast.h"

#include <memory>

using namespace std;

unique_ptr<Ast::Node> Ast::create_node(NodeKind kind,
                                       std::unique_ptr<Node> left,
                                       std::unique_ptr<Node> right) {
  unique_ptr<Ast::Node> node = make_unique<Ast::Node>();
  node->kind = kind;
  node->left = move(left);
  node->right = move(right);
  return move(node);
}

unique_ptr<Ast::Node> Ast::create_num(int val) {
  unique_ptr<Ast::Node> node = make_unique<Ast::Node>();
  node->kind = Ast::NodeKind::nd_num;
  node->val = val;
  return move(node);
}
