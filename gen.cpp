#include "gen.h"

#include <iostream>
#include <memory>

#include "ast.h"

using namespace std;

void codegen::gen(unique_ptr<Ast::Node> node) {
  switch (node->kind) {
    case Ast::NodeKind::nd_num:
      cout << "  push " << node->val << endl;
      return;
    case Ast::NodeKind::nd_lval:
      gen_lval(move(node));
      // addr of lval is on the stack top
      cout << "  pop rax" << endl;
      cout << "  mov rax, [rax]" << endl;
      cout << "  push rax" << endl;
      return;
    case Ast::NodeKind::nd_assign:
      gen_lval(move(node->left));
      gen(move(node->right));

      cout << "  pop rdi" << endl;
      cout << "  pop rax" << endl;  // addr of lval
      cout << "  mov [rax], rdi" << endl;
      cout << "  push rdi" << endl;
      return;
    default:
      break;
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
    case Ast::NodeKind::nd_eq:
      cout << "  cmp rax, rdi" << endl;
      cout << "  sete al" << endl;
      cout << "  movzb rax, al" << endl;
      break;
    case Ast::NodeKind::nd_neq:
      cout << "  cmp rax, rdi" << endl;
      cout << "  setne al" << endl;
      cout << "  movzb rax, al" << endl;
      break;
    case Ast::NodeKind::nd_le:
      cout << "  cmp rax, rdi" << endl;
      cout << "  setl al" << endl;
      cout << "  movzb rax, al" << endl;
      break;
    case Ast::NodeKind::nd_leq:
      cout << "  cmp rax, rdi" << endl;
      cout << "  setle al" << endl;
      cout << "  movzb rax, al" << endl;
      break;
    default:
      exit(-1);
  }

  cout << "  push rax" << endl;
}

void codegen::gen_lval(unique_ptr<Ast::Node> node) {
  // push addr of lval (i.e. rbp - 8)
  if (node->kind != Ast::NodeKind::nd_lval) {
    exit(-1);
  }
  cout << "  mov rax, rbp" << endl;
  cout << "  sub rax, " << node->offset << endl;
  cout << "  push rax" << endl;
}
