#include "gen.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "ast.h"
#include "enum_magic.hpp"

using namespace std;

namespace codegen {

void gen(unique_ptr<ast::Node> node) {
  vector<string> arg_regs{"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
  switch (node->kind) {
    case ast::NodeKind::nd_program:
      for (auto&& child : node->child_vec) {
        gen(move(child));
      }
      return;
    case ast::NodeKind::nd_return:
      gen(move(node->child_vec.at(0)));
      cout << "  pop rax\n";
      cout << "  mov rsp, rbp\n";
      cout << "  pop rbp\n";
      cout << "  ret\n";
      return;
    case ast::NodeKind::nd_num:
      cout << "  push " << node->val << '\n';
      return;
    case ast::NodeKind::nd_arg_decl: {
      int arg_idx = node->arg_idx;
      gen_lval(move(node));
      // addr of lval is on the stack top
      cout << "  pop rax\n";
      cout << "  mov [rax], " << arg_regs.at(arg_idx) << '\n';
      return;
    }
    case ast::NodeKind::nd_lval:
      gen_lval(move(node));
      // addr of lval is on the stack top
      cout << "  pop rax\n";
      cout << "  mov rax, [rax]\n";
      cout << "  push rax\n";
      return;
    case ast::NodeKind::nd_deref:
      gen(move(node->child_vec.at(0)));
      cout << "  pop rax\n";  // addr of lval
      cout << "  mov rax, [rax]\n";
      cout << "  push rax\n";
      return;
    case ast::NodeKind::nd_addr:
      gen_lval(move(node->child_vec.at(0)));
      return;
    case ast::NodeKind::nd_assign:
      gen_lval(move(node->child_vec.at(0)));
      gen(move(node->child_vec.at(1)));

      cout << "  pop rdi\n";
      cout << "  pop rax\n";  // addr of lval
      cout << "  mov [rax], rdi\n";
      cout << "  push rdi\n";
      return;
    case ast::NodeKind::nd_add_into:
      gen_lval(move(node->child_vec.at(0)));
      gen(move(node->child_vec.at(1)));

      cout << "  pop rdi\n";
      cout << "  pop rax\n";  // addr of lval
      cout << "  add [rax], rdi\n";
      cout << "  push [rax]\n";  // push final result
      return;
    case ast::NodeKind::nd_if: {
      string if_end = create_label("ifEnd");
      // expr
      gen(move(node->child_vec.at(0)));

      cout << "  pop rax\n";
      cout << "  cmp rax, 0\n";
      cout << "  je " << if_end << '\n';
      // then
      gen(move(node->child_vec.at(1)));
      cout << if_end << ":\n";
      return;
    }
    case ast::NodeKind::nd_ifelse: {
      string if_end = create_label("ifEnd");
      string else_end = create_label("elseEnd");
      // expr
      gen(move(node->child_vec.at(0)));

      cout << "  pop rax\n";
      cout << "  cmp rax, 0\n";
      cout << "  je " << if_end << '\n';
      // then
      gen(move(node->child_vec.at(1)));
      cout << "  jmp " << else_end << '\n';
      cout << if_end << ":\n";
      // else
      gen(move(node->child_vec.at(2)));
      cout << else_end << ":\n";
      return;
    }
    case ast::NodeKind::nd_while: {
      string while_start = create_label("whileStart");
      string while_end = create_label("whileEnd");
      // start
      cout << while_start << ":\n";
      // expr
      gen(move(node->child_vec.at(0)));

      cout << "  pop rax\n";
      cout << "  cmp rax, 0\n";
      cout << "  je " << while_end << '\n';
      // body
      gen(move(node->child_vec.at(1)));
      cout << "  jmp " << while_start << '\n';
      // end
      cout << while_end << ":\n";
      return;
    }
    case ast::NodeKind::nd_for: {
      // for (0; 1; 2) 3
      string for_start = create_label("forStart");
      string for_end = create_label("forEnd");
      // initialize
      gen(move(node->child_vec.at(0)));
      // start
      cout << for_start << ":\n";
      // expr
      bool have_expr = node->child_vec.at(1)->kind != ast::NodeKind::nd_blank;
      if (have_expr) {
        gen(move(node->child_vec.at(1)));
        cout << "  pop rax\n";
        cout << "  cmp rax, 0\n";
        cout << "  je " << for_end << '\n';
      }
      // body
      gen(move(node->child_vec.at(2)));
      // update
      gen(move(node->child_vec.at(3)));
      cout << "  jmp " << for_start << '\n';
      // end
      if (have_expr) {
        cout << for_end << ":\n";
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
        cout << "  pop " << arg_regs.at(i) << '\n';
      }
      // 16-alignment of rsp
      if (num_args % 2 != 0) {
        cout << "  sub rsp, 8\n";
      }
      // function call
      string funcname = {node->tok.lexeme_string, (unsigned long)node->tok.len};
      cout << "  call _" << funcname << '\n';
      cout << "  push rax\n";
      return;
    }
    case ast::NodeKind::nd_funcdef: {
      string funcname = {node->tok.lexeme_string, (unsigned long)node->tok.len};
      cout << ".globl _" << funcname << '\n';
      cout << ".p2align 4, 0x90\n";
      cout << "_" << funcname << ":\n";

      // prologe
      cout << "  push rbp\n";
      cout << "  mov rbp, rsp\n";
      if (node->total_size > 0) {
        cout << "  sub rsp, " << node->total_size << '\n';
      }

      // code generation of function body
      // last element is compound, others are arguments
      for (auto&& child : node->child_vec) {
        gen(move(child));
      }
      cout << "  pop rax\n";

      // epiloge
      cout << "  mov rsp, rbp\n";
      cout << "  pop rbp\n";
      cout << "  ret\n";
      return;
    }
    case ast::NodeKind::nd_funcdecl:
      return;
    default:
      break;
  }

  gen(move(node->child_vec.at(0)));
  gen(move(node->child_vec.at(1)));

  cout << "  pop rdi\n";
  cout << "  pop rax\n";

  switch (node->kind) {
    case ast::NodeKind::nd_add:
      cout << "  add rax, rdi\n";
      break;
    case ast::NodeKind::nd_sub:
      cout << "  sub rax, rdi\n";
      break;
    case ast::NodeKind::nd_mul:
      cout << "  imul rax, rdi\n";
      break;
    case ast::NodeKind::nd_div:
      cout << "  cqo\n";
      cout << "  idiv rdi\n";
      break;
    case ast::NodeKind::nd_eq:
      cout << "  cmp rax, rdi\n";
      cout << "  sete al\n";
      cout << "  movzx rax, al\n";
      break;
    case ast::NodeKind::nd_ne:
      cout << "  cmp rax, rdi\n";
      cout << "  setne al\n";
      cout << "  movzx rax, al\n";
      break;
    case ast::NodeKind::nd_lt:
      cout << "  cmp rax, rdi\n";
      cout << "  setl al\n";
      cout << "  movzx rax, al\n";
      break;
    case ast::NodeKind::nd_le:
      cout << "  cmp rax, rdi\n";
      cout << "  setle al\n";
      cout << "  movzx rax, al\n";
      break;
    default:
      cerr << "unexpected NnodeKind\n";
      exit(-1);
  }

  cout << "  push rax\n";
}

void gen_lval(unique_ptr<ast::Node> node) {
  // push addr of lval (i.e. rbp - 8)
  switch (node->kind) {
    case ast::NodeKind::nd_deref:
      gen(move(node->child_vec.at(0)));
      return;
    case ast::NodeKind::nd_lval:
    case ast::NodeKind::nd_arg_decl:
      cout << "  lea rax, [rbp - " << node->offset << "]\n";
      cout << "  push rax\n";
      return;
    default:
      cerr << "nd_lval node or nd_deref are expected, but not\n";
      cerr << "current node kind is:" << magic_enum::enum_name(node->kind)
           << '\n';
      exit(-1);
  }
}

string create_label(string name) {
  static int num = 0;
  stringstream ss;
  ss << label_prefix << "." << name << "." << num++;
  return ss.str();
}
}  // namespace codegen
