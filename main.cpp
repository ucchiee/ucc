#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "ast.h"
#include "gen.h"
#include "lexer.h"
#include "parser.h"
using namespace std;

vector<unique_ptr<Ast::Node>> node_vec;

int main(int argc, char** argv) {
  if (argc != 2) {
    cerr << "invalid # of args" << endl;
    return 1;
  }

  Lexer::TokenStream ts{argv[1]};
  parser::Parser parser{ts};
  parser.program();

  cout << ".intel_syntax noprefix" << endl;
  cout << ".globl main" << endl;
  cout << "main:" << endl;

  // prologe
  cout << "  push rbp" << endl;
  cout << "  mov rbp, rsp" << endl;
  cout << "  sub rsp, 208" << endl;

  // code genertor for each node
  // wanna use auto for
  for (int i = 0; i <node_vec.size(); i++) {
    codegen::gen(move(node_vec.at(i)));

    cout << "  pop rax" << endl;
  }

  // epiloge
  cout << "  mov rsp, rbp" << endl;
  cout << "  pop rbp" << endl;
  cout << "  ret" << endl;
  return 0;
}
