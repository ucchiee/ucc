#include "gen.h"

#include <iostream>
#include <memory>

#include "ast.h"

using namespace std;

void codegen::gen(unique_ptr<Ast::Node> node) {
  if (node->kind == Ast::NodeKind::nd_num) {
    cout << "  push " << node->val << endl;
    return;
  }

  gen(move(node->left));
  gen(move(node->right));

  cout << "  pop rdi" << endl;
  cout << "  pop rax" << endl;

  switch (node->kind) {
    case Ast::NodeKind::nd_add:
      cout << "  add rax, rdi" << endl;
      break;
    case Ast::NodeKind::nd_sub:
      cout << "  sub rax, rdi" << endl;
      break;
    case Ast::NodeKind::nd_mul:
      cout << "  imul rax, rdi" << endl;
      break;
    case Ast::NodeKind::nd_div:
      cout << "  cqo" << endl;
      cout << "  idiv rdi" << endl;
      break;
    default:
      exit(-1);
  }

  cout << "  push rax" << endl;
}
