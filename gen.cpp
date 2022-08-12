#include "gen.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "ast.h"
#include "enum_magic.hpp"
#include "fmt/core.h"

using namespace std;
using fmt::print;

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
      print("  pop rax\n");
      print("  mov rsp, rbp\n");
      print("  pop rbp\n");
      print("  ret\n");
      return;
    case ast::NodeKind::nd_num:
      print("  push {}\n", node->val);
      return;
    case ast::NodeKind::nd_arg_decl: {
      int arg_idx = node->arg_idx;
      gen_lval(move(node));
      // addr of lval is on the stack top
      print("  pop rax\n");
      print("  mov [rax], {}\n", arg_regs.at(arg_idx));
      return;
    }
    case ast::NodeKind::nd_lval:
      gen_lval(move(node));
      // addr of lval is on the stack top
      print("  pop rax\n");
      print("  mov rax, [rax]\n");
      print("  push rax\n");
      return;
    case ast::NodeKind::nd_deref:
      gen(move(node->child_vec.at(0)));
      print("  pop rax\n");  // addr of lval
      print("  mov rax, [rax]\n");
      print("  push rax\n");
      return;
    case ast::NodeKind::nd_addr:
      gen_lval(move(node->child_vec.at(0)));
      return;
    case ast::NodeKind::nd_assign:
      gen_lval(move(node->child_vec.at(0)));
      gen(move(node->child_vec.at(1)));

      print("  pop rdi\n");
      print("  pop rax\n");  // addr of lval
      print("  mov [rax], rdi\n");
      print("  push rdi\n");
      return;
    case ast::NodeKind::nd_add_into:
      gen_lval(move(node->child_vec.at(0)));
      gen(move(node->child_vec.at(1)));

      print("  pop rdi\n");
      print("  pop rax\n");  // addr of lval
      print("  add [rax], rdi\n");
      print("  push [rax]\n");  // push final result
      return;
    case ast::NodeKind::nd_if: {
      string if_end = create_label("ifEnd");
      // expr
      gen(move(node->child_vec.at(0)));

      print("  pop rax\n");
      print("  cmp rax, 0\n");
      print("  je {}\n", if_end);
      // then
      gen(move(node->child_vec.at(1)));
      print("{}:\n", if_end);
      return;
    }
    case ast::NodeKind::nd_ifelse: {
      string if_end = create_label("ifEnd");
      string else_end = create_label("elseEnd");
      // expr
      gen(move(node->child_vec.at(0)));

      print("  pop rax\n");
      print("  cmp rax, 0\n");
      print("  je {}\n", if_end);
      // then
      gen(move(node->child_vec.at(1)));
      print("  jmp {}\n", else_end);
      print("{}:\n", if_end);
      // else
      gen(move(node->child_vec.at(2)));
      print("{}:\n", else_end);
      return;
    }
    case ast::NodeKind::nd_while: {
      string while_start = create_label("whileStart");
      string while_end = create_label("whileEnd");
      // start
      print("{}:\n", while_start);
      // expr
      gen(move(node->child_vec.at(0)));

      print("  pop rax\n");
      print("  cmp rax, 0\n");
      print("  je {}\n", while_end);
      // body
      gen(move(node->child_vec.at(1)));
      print("  jmp {}\n", while_start);
      // end
      print("{}:\n", while_end);
      return;
    }
    case ast::NodeKind::nd_for: {
      // for (0; 1; 2) 3
      string for_start = create_label("forStart");
      string for_end = create_label("forEnd");
      // initialize
      gen(move(node->child_vec.at(0)));
      // start
      print("{}:\n", for_start);
      // expr
      bool have_expr = node->child_vec.at(1)->kind != ast::NodeKind::nd_blank;
      if (have_expr) {
        gen(move(node->child_vec.at(1)));
        print("  pop rax\n");
        print("  cmp rax, 0\n");
        print("  je {}\n", for_end);
      }
      // body
      gen(move(node->child_vec.at(2)));
      // update
      gen(move(node->child_vec.at(3)));
      print("  jmp {}\n", for_start);
      // end
      if (have_expr) {
        print("{}:\n", for_end);
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
        print("  pop {}\n", arg_regs.at(i));
      }
      // 16-alignment of rsp
      if (num_args % 2 != 0) {
        print("  sub rsp, 8\n");
      }
      // function call
      string funcname = {node->tok.lexeme_string, (unsigned long)node->tok.len};
      print("  call {}\n", funcname);
      print("  push rax\n");
      return;
    }
    case ast::NodeKind::nd_funcdef: {
      string funcname = {node->tok.lexeme_string, (unsigned long)node->tok.len};
      print(".globl {}\n", funcname);
      print("{}:\n", funcname);

      // prologe
      print("  push rbp\n");
      print("  mov rbp, rsp\n");
      if (node->total_size > 0) {
        print("  sub rsp, {}\n", node->total_size);
      }

      // code generation of function body
      // last element is compound, others are arguments
      for (auto&& child : node->child_vec) {
        gen(move(child));
      }
      print("  pop rax\n");

      // epiloge
      print("  mov rsp, rbp\n");
      print("  pop rbp\n");
      print("  ret\n");
      return;
    }
    case ast::NodeKind::nd_funcdecl:
      return;
    default:
      break;
  }

  gen(move(node->child_vec.at(0)));
  gen(move(node->child_vec.at(1)));

  print("  pop rdi\n");
  print("  pop rax\n");

  switch (node->kind) {
    case ast::NodeKind::nd_add:
      print("  add rax, rdi\n");
      break;
    case ast::NodeKind::nd_sub:
      print("  sub rax, rdi\n");
      break;
    case ast::NodeKind::nd_mul:
      print("  imul rax, rdi\n");
      break;
    case ast::NodeKind::nd_div:
      print("  cqo\n");
      print("  idiv rdi\n");
      break;
    case ast::NodeKind::nd_eq:
      print("  cmp rax, rdi\n");
      print("  sete al\n");
      print("  movzb rax, al\n");
      break;
    case ast::NodeKind::nd_ne:
      print("  cmp rax, rdi\n");
      print("  setne al\n");
      print("  movzb rax, al\n");
      break;
    case ast::NodeKind::nd_lt:
      print("  cmp rax, rdi\n");
      print("  setl al\n");
      print("  movzb rax, al\n");
      break;
    case ast::NodeKind::nd_le:
      print("  cmp rax, rdi\n");
      print("  setle al\n");
      print("  movzb rax, al\n");
      break;
    default:
      cerr << "unexpected NnodeKind\n";
      exit(-1);
  }

  print("  push rax\n");
}

void gen_lval(unique_ptr<ast::Node> node) {
  // push addr of lval (i.e. rbp - 8)
  switch (node->kind) {
    case ast::NodeKind::nd_deref:
      gen(move(node->child_vec.at(0)));
      return;
    case ast::NodeKind::nd_lval:
    case ast::NodeKind::nd_arg_decl:
      print("  lea rax, [rbp - {}]\n", node->offset);
      print("  push rax\n");
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
