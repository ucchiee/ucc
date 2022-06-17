#include "gen.h"

#include <iostream>
#include <memory>
#include <sstream>

#include "ast.h"
#include "enum_magic.hpp"

using namespace std;

void codegen::gen(unique_ptr<ast::Node> node) {
  string label1, label2, funcname;
  bool have_expr;
  switch (node->kind) {
    case ast::NodeKind::nd_return:
      gen(move(node->child_vec.at(0)));
      cout << "  pop rax" << endl;
      cout << "  mov rsp, rbp" << endl;
      cout << "  pop rbp" << endl;
      cout << "  ret" << endl;
      return;
    case ast::NodeKind::nd_num:
      cout << "  push " << node->val << endl;
      return;
    case ast::NodeKind::nd_lval:
      gen_lval(move(node));
      // addr of lval is on the stack top
      cout << "  pop rax" << endl;
      cout << "  mov rax, [rax]" << endl;
      cout << "  push rax" << endl;
      return;
    case ast::NodeKind::nd_deref:
      gen(move(node->child_vec.at(0)));
      cout << "  pop rax" << endl;  // addr of lval
      cout << "  mov rax, [rax]" << endl;
      cout << "  push rax" << endl;
      return;
    case ast::NodeKind::nd_addr:
      gen_lval(move(node->child_vec.at(0)));
      return;
    case ast::NodeKind::nd_assign:
      gen_lval(move(node->child_vec.at(0)));
      gen(move(node->child_vec.at(1)));

      cout << "  pop rdi" << endl;
      cout << "  pop rax" << endl;  // addr of lval
      cout << "  mov [rax], rdi" << endl;
      cout << "  push rdi" << endl;
      return;
    case ast::NodeKind::nd_add_into:
      gen_lval(move(node->child_vec.at(0)));
      gen(move(node->child_vec.at(1)));

      cout << "  pop rdi" << endl;
      cout << "  pop rax" << endl;  // addr of lval
      cout << "  add [rax], rdi" << endl;
      cout << "  push [rax]" << endl;  // push final result
      return;
    case ast::NodeKind::nd_if:
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
    case ast::NodeKind::nd_ifelse:
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
    case ast::NodeKind::nd_while:
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
    case ast::NodeKind::nd_for:
      // for (0; 1; 2) 3
      label1 = codegen::create_label("forStart");
      label2 = codegen::create_label("forEnd");
      // initialize
      gen(move(node->child_vec.at(0)));
      // start
      cout << label1 << ":" << endl;
      // expr
      have_expr = node->child_vec.at(1)->kind != ast::NodeKind::nd_blank;
      if (have_expr) {
        gen(move(node->child_vec.at(1)));
        cout << "  pop rax" << endl;
        cout << "  cmp rax, 0" << endl;
        cout << "  je " << label2 << endl;
      }
      // body
      gen(move(node->child_vec.at(2)));
      // update
      gen(move(node->child_vec.at(3)));
      cout << "  jmp " << label1 << endl;
      // end
      if (have_expr) {
        cout << label2 << ":" << endl;
      }
      return;
    case ast::NodeKind::nd_blank:
      return;
    case ast::NodeKind::nd_compound:
      for (int i = 0; i < node->child_vec.size(); i++) {
        gen(move(node->child_vec.at(i)));
      }
      return;
    case ast::NodeKind::nd_funcall:
      funcname = {node->tok.lexeme_string, (unsigned long)node->tok.len};
      cout << "  lea rax, [rip + " << funcname << "]" << endl;
      cout << "  call rax" << endl;
      cout << "  push rax" << endl;
      return;
    default:
      break;
  }

  gen(move(node->child_vec.at(0)));
  gen(move(node->child_vec.at(1)));

  cout << "  pop rdi" << endl;
  cout << "  pop rax" << endl;

  switch (node->kind) {
    case ast::NodeKind::nd_add:
      cout << "  add rax, rdi" << endl;
      break;
    case ast::NodeKind::nd_sub:
      cout << "  sub rax, rdi" << endl;
      break;
    case ast::NodeKind::nd_mul:
      cout << "  imul rax, rdi" << endl;
      break;
    case ast::NodeKind::nd_div:
      cout << "  cqo" << endl;
      cout << "  idiv rdi" << endl;
      break;
    case ast::NodeKind::nd_eq:
      cout << "  cmp rax, rdi" << endl;
      cout << "  sete al" << endl;
      cout << "  movzb rax, al" << endl;
      break;
    case ast::NodeKind::nd_ne:
      cout << "  cmp rax, rdi" << endl;
      cout << "  setne al" << endl;
      cout << "  movzb rax, al" << endl;
      break;
    case ast::NodeKind::nd_lt:
      cout << "  cmp rax, rdi" << endl;
      cout << "  setl al" << endl;
      cout << "  movzb rax, al" << endl;
      break;
    case ast::NodeKind::nd_le:
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

void codegen::gen_lval(unique_ptr<ast::Node> node) {
  // push addr of lval (i.e. rbp - 8)
  switch (node->kind) {
    case ast::NodeKind::nd_deref:
      gen(move(node->child_vec.at(0)));
      return;
    case ast::NodeKind::nd_lval:
      cout << "  lea rax, [rbp - " << node->offset << "]" << endl;
      cout << "  push rax" << endl;
      return;
    default:
      cerr << "nd_lval node or nd_deref are expected, but not" << endl;
      cerr << "current node kind is:" << magic_enum::enum_name(node->kind)
           << endl;
      exit(-1);
  }
}

string codegen::create_label(string name) {
  static int num = 0;
  stringstream ss;
  ss << codegen::label_prefix << "." << name << "." << num++;
  return ss.str();
}
