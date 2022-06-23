#include "gen.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "ast.h"
#include "enum_magic.hpp"

using namespace std;

void codegen::gen(unique_ptr<ast::Node> node) {
  vector<string> arg_regs{"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
  switch (node->kind) {
    case ast::NodeKind::nd_program:
      for (auto&& child : node->child_vec) {
        gen(move(child));
      }
      return;
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
    case ast::NodeKind::nd_param_decl: {
      int arg_idx = node->arg_idx;
      gen_lval(move(node));
      // addr of lval is on the stack top
      cout << "  pop rax" << endl;
      cout << "  mov [rax], " << arg_regs.at(arg_idx) << endl;
      return;
    }
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
    case ast::NodeKind::nd_if: {
      string if_end = codegen::create_label("ifEnd");
      // expr
      gen(move(node->child_vec.at(0)));

      cout << "  pop rax" << endl;
      cout << "  cmp rax, 0" << endl;
      cout << "  je " << if_end << endl;
      // then
      gen(move(node->child_vec.at(1)));
      cout << if_end << ":" << endl;
      return;
    }
    case ast::NodeKind::nd_ifelse: {
      string if_end = codegen::create_label("ifEnd");
      string else_end = codegen::create_label("elseEnd");
      // expr
      gen(move(node->child_vec.at(0)));

      cout << "  pop rax" << endl;
      cout << "  cmp rax, 0" << endl;
      cout << "  je " << if_end << endl;
      // then
      gen(move(node->child_vec.at(1)));
      cout << "  jmp " << else_end << endl;
      cout << if_end << ":" << endl;
      // else
      gen(move(node->child_vec.at(2)));
      cout << else_end << ":" << endl;
      return;
    }
    case ast::NodeKind::nd_while: {
      string while_start = codegen::create_label("whileStart");
      string while_end = codegen::create_label("whileEnd");
      // start
      cout << while_start << ":" << endl;
      // expr
      gen(move(node->child_vec.at(0)));

      cout << "  pop rax" << endl;
      cout << "  cmp rax, 0" << endl;
      cout << "  je " << while_end << endl;
      // body
      gen(move(node->child_vec.at(1)));
      cout << "  jmp " << while_start << endl;
      // end
      cout << while_end << ":" << endl;
      return;
    }
    case ast::NodeKind::nd_for: {
      // for (0; 1; 2) 3
      string for_start = codegen::create_label("forStart");
      string for_end = codegen::create_label("forEnd");
      // initialize
      gen(move(node->child_vec.at(0)));
      // start
      cout << for_start << ":" << endl;
      // expr
      bool have_expr = node->child_vec.at(1)->kind != ast::NodeKind::nd_blank;
      if (have_expr) {
        gen(move(node->child_vec.at(1)));
        cout << "  pop rax" << endl;
        cout << "  cmp rax, 0" << endl;
        cout << "  je " << for_end << endl;
      }
      // body
      gen(move(node->child_vec.at(2)));
      // update
      gen(move(node->child_vec.at(3)));
      cout << "  jmp " << for_start << endl;
      // end
      if (have_expr) {
        cout << for_end << ":" << endl;
      }
      return;
    }
    case ast::NodeKind::nd_blank:
      return;
    case ast::NodeKind::nd_compound:
      for (int i = 0; i < node->child_vec.size(); i++) {
        gen(move(node->child_vec.at(i)));
      }
      return;
    case ast::NodeKind::nd_funcall: {
      // evaluate args and push them into stack
      int num_args = node->child_vec.size();
      while (!node->child_vec.empty()) {
        gen(move(node->child_vec.back()));
        node->child_vec.pop_back();
      }
      // set args in the registers
      for (int i = 0; i < num_args; i++) {
        cout << "  pop " << arg_regs.at(i) << endl;
      }
      // 16-alignment of rsp
      if (num_args % 2 != 0) {
        cout << "  sub rsp, 8" << endl;
      }
      // function call
      string funcname = {node->tok.lexeme_string, (unsigned long)node->tok.len};
      cout << "  call " << funcname << endl;
      cout << "  push rax" << endl;
      return;
    }
    case ast::NodeKind::nd_funcdef: {
      string funcname = {node->tok.lexeme_string, (unsigned long)node->tok.len};
      cout << ".globl " << funcname << endl;
      cout << funcname << ":" << endl;

      // prologe
      cout << "  push rbp" << endl;
      cout << "  mov rbp, rsp" << endl;
      if (node->total_size > 0) {
        cout << "  sub rsp, " << node->total_size << endl;
      }

      // code generation of function body
      // last element is compound, others are arguments
      for (auto&& child : node->child_vec) {
        gen(move(child));
      }
      cout << "  pop rax" << endl;

      // epiloge
      cout << "  mov rsp, rbp" << endl;
      cout << "  pop rbp" << endl;
      cout << "  ret" << endl;
      return;
    }
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
    case ast::NodeKind::nd_param_decl:
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
