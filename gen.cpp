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

static int idx_size;
static vector<string> regax = {"rax", "eax", "ax", "al"};
static vector<string> regdi = {"rdi", "edi", "di", "dil"};
static vector<string> regsi = {"rsi", "esi", "si", "sil"};
static vector<string> regdx = {"rdx", "edx", "dx", "dl"};
static vector<string> regcx = {"rcx", "ecx", "cx", "cl"};
static vector<string> reg10 = {"r10", "r10d", "r10w", "r10b"};
static vector<string> reg9 = {"r9", "r9d", "r9w", "r9b"};
static vector<string> reg8 = {"r8", "r8d", "r8w", "r8b"};

void gen(unique_ptr<ast::Node> node) {
  vector<vector<string>> arg_regs{regdi, regsi, regdx, regcx, reg8, reg9};
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
      print("  mov [rax], {}\n", arg_regs[arg_idx][idx_size]);
      return;
    }
    case ast::NodeKind::nd_lval: {
      auto type = node->type;
      gen_lval(move(node));
      // addr of lval is on the stack top
      print("  pop rax\n");
      print("  mov {}, [rax]\n", regax[idx_size]);

      // Make addr of m_ptr canonical
      if (type && type->is_m_ptr()) {
        // Upper 16bits are as same as 47bit
        // Use arithmetic shift to accomplish this.
        print("  sal rax, 16\n");
        print("  sar rax, 16\n");
        print("  add rax, 4\n");  // Skip counter region.
      }
      print("  push rax\n");  // must be 64bit reg
      return;
    }
    case ast::NodeKind::nd_gval_def: {
      string label = string(node->tok.lexeme_string, node->tok.len);
      print(".data\n");
      print(".globl {}\n", label);
      print("{}:\n", label);
      print("  .zero {}\n", node->type->get_size());
      return;
    }
    case ast::NodeKind::nd_gval: {
      gen_lval(move(node));
      // Addr of gval is on the stack top.
      print("  pop rax\n");
      print("  mov {}, [rax]\n", regax[idx_size]);
      print("  push rax\n");  // must be 64bit reg
      return;
    }
    case ast::NodeKind::nd_deref:
      gen(move(node->child_vec.at(0)));
      print("  pop rax\n");  // addr of lval
      print("  mov {}, [rax]\n", regax[idx_size]);
      print("  push rax\n");
      return;
    case ast::NodeKind::nd_addr:
      gen_lval(move(node->child_vec.at(0)));
      idx_size = 0;  // HACK: address should be 8 bytes.
      return;
    case ast::NodeKind::nd_assign:
      gen_lval(move(node->child_vec.at(0)));
      gen(move(node->child_vec.at(1)));

      print("  pop rdi\n");
      print("  pop rax\n");  // addr of lval
      print("  mov [rax], {}\n", regdi[idx_size]);
      print("  push rdi\n");
      return;
    case ast::NodeKind::nd_add_into:
      gen_lval(move(node->child_vec.at(0)));
      gen(move(node->child_vec.at(1)));

      print("  pop rdi\n");
      print("  pop rax\n");  // addr of lval
      print("  add [rax], {}\n", regdi[idx_size]);
      print("  push [rax]\n");  // push final result
      return;
    case ast::NodeKind::nd_if: {
      string if_end = create_label("ifEnd");
      // expr
      gen(move(node->child_vec.at(0)));

      print("  pop rax\n");
      print("  cmp {}, 0\n", regax[idx_size]);
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
      print("  cmp {}, 0\n", regax[idx_size]);
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
      print("  cmp {}, 0\n", regax[idx_size]);
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
        print("  cmp {}, 0\n", regax[idx_size]);
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
        print("  pop {}\n", arg_regs.at(i)[0]);
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
      print(".text\n");
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
      print("  add {}, {}\n", regax[idx_size], regdi[idx_size]);
      break;
    case ast::NodeKind::nd_sub:
      print("  sub {}, {}\n", regax[idx_size], regdi[idx_size]);
      break;
    case ast::NodeKind::nd_mul:
      print("  imul {}, {}\n", regax[idx_size], regdi[idx_size]);
      break;
    case ast::NodeKind::nd_div:
      print("  cqo\n");
      print("  idiv {}\n", regdi[idx_size]);
      break;
    case ast::NodeKind::nd_eq:
      print("  cmp {}, {}\n", regax[idx_size], regdi[idx_size]);
      print("  sete al\n");
      print("  movzb {}, al\n", regax[idx_size]);
      break;
    case ast::NodeKind::nd_ne:
      print("  cmp {}, {}\n", regax[idx_size], regdi[idx_size]);
      print("  setne al\n");
      print("  movzb {}, al\n", regax[idx_size]);
      break;
    case ast::NodeKind::nd_lt:
      print("  cmp {}, {}\n", regax[idx_size], regdi[idx_size]);
      print("  setl al\n");
      print("  movzb {}, al\n", regax[idx_size]);
      break;
    case ast::NodeKind::nd_le:
      print("  cmp {}, {}\n", regax[idx_size], regdi[idx_size]);
      print("  setle al\n");
      print("  movzb {}, al\n", regax[idx_size]);
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
    case ast::NodeKind::nd_deref: {
      size_t size = node->child_vec.at(0)->type->m_next->get_size();
      gen(move(node->child_vec.at(0)));
      set_idx_size(size);
      return;
    }
    case ast::NodeKind::nd_gval: {
      string label = string(node->tok.lexeme_string, node->tok.len);
      set_idx_size(node->type->get_size());
      print("  lea rax, {}[rip]\n", label);
      print("  push rax\n");
      return;
    }
    case ast::NodeKind::nd_lval:
    case ast::NodeKind::nd_arg_decl:
      set_idx_size(node->type->get_size());
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

void set_idx_size(size_t size) {
  if (size == 8) {
    idx_size = 0;
  } else if (size == 4) {
    idx_size = 1;
  } else if (size == 1) {
    idx_size = 3;
  }
}

}  // namespace codegen
