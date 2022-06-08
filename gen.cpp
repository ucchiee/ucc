#include "gen.h"

#include <iostream>
#include <memory>
#include <sstream>

#include "ast.h"

using namespace std;

void codegen::gen(unique_ptr<Ast::Node> node) {
  string label1, label2;
  switch (node->kind) {
    case Ast::NodeKind::nd_return:
      gen(move(node->child_vec.at(0)));
      cout << "  pop rax" << endl;
      cout << "  mov rsp, rbp" << endl;
      cout << "  pop rbp" << endl;
      cout << "  ret" << endl;
      return;
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
      gen_lval(move(node->child_vec.at(0)));
      gen(move(node->child_vec.at(1)));

      cout << "  pop rdi" << endl;
      cout << "  pop rax" << endl;  // addr of lval
      cout << "  mov [rax], rdi" << endl;
      cout << "  push rdi" << endl;
      return;
    case Ast::NodeKind::nd_if:
      label1 = codegen::create_label("ifEnd");
      // expr
      gen(move(node->child_vec.at(0)));

      cout << "  pop rax" << endl;
      cout << "  cmp rax, 0" << endl;
      cout << "  je " << label1 << endl;
      // then
      gen(move(node->child_vec.at(1)));
      cout << label1 << ":" << endl;
      return;
    case Ast::NodeKind::nd_ifelse:
      label1 = codegen::create_label("ifEnd");
      label2 = codegen::create_label("elseEnd");
      // expr
      gen(move(node->child_vec.at(0)));

      cout << "  pop rax" << endl;
      cout << "  cmp rax, 0" << endl;
      cout << "  je " << label1 << endl;
      // then
      gen(move(node->child_vec.at(1)));
      cout << "  jmp " << label2 << endl;
      cout << label1 << ":" << endl;
      // else
      gen(move(node->child_vec.at(2)));
      cout << label2 << ":" << endl;
      return;
    case Ast::NodeKind::nd_while:
      label1 = codegen::create_label("whileStart");
      label2 = codegen::create_label("whileEnd");
      // start
      cout << label1 << ":" << endl;
      // expr
      gen(move(node->child_vec.at(0)));

      cout << "  pop rax" << endl;
      cout << "  cmp rax, 0" << endl;
      cout << "  je " << label2 << endl;
      // body
      gen(move(node->child_vec.at(1)));
      cout << "  jmp " << label1 << endl;
      // end
      cout << label2 << ":" << endl;
      return;
    default:
      break;
  }

  gen(move(node->child_vec.at(0)));
  gen(move(node->child_vec.at(1)));

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
      cerr << "unexpected NnodeKind" << endl;
      exit(-1);
  }

  cout << "  push rax" << endl;
}

void codegen::gen_lval(unique_ptr<Ast::Node> node) {
  // push addr of lval (i.e. rbp - 8)
  if (node->kind != Ast::NodeKind::nd_lval) {
    cerr << "nd_lval node is expected, but not" << endl;
    exit(-1);
  }
  cout << "  lea rax, [rbp - " << node->offset << "]" << endl;
  cout << "  push rax" << endl;
}

string codegen::create_label(string name) {
  static int num = 0;
  stringstream ss;
  ss << codegen::label_prefix << "." << name << "." << num++;
  return ss.str();
}
