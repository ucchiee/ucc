#include <iostream>
#include <memory>
#include <sstream>

#include "ast.h"
#include "gen.h"
#include "lexer.h"
#include "parser.h"
using namespace std;

int main(int argc, char** argv) {
  if (argc != 2) {
    cerr << "invalid # of args" << endl;
    return 1;
  }

  Lexer::TokenStream ts{argv[1]};
  parser::Parser parser{ts};
  unique_ptr<Ast::Node> node = parser.expr();

  cout << ".intel_syntax noprefix" << endl;
  cout << ".globl main" << endl;
  cout << "main:" << endl;

  codegen::gen(move(node));
  cout << "  pop rax" << endl;
  cout << "  ret" << endl;
  return 0;
}
