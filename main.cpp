#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "ast.h"
#include "gen.h"
#include "lexer.h"
#include "parser.h"
using namespace std;

extern vector<unique_ptr<ast::Node>> node_vec;

int main(int argc, char** argv) {
  if (argc != 2) {
    cerr << "invalid # of args" << endl;
    return 1;
  }

  lexer::TokenStream ts{argv[1]};
  parser::Parser parser{ts};
  parser.program();

  cout << ".intel_syntax noprefix" << endl;

  // code genertor for each node
  // wanna use auto for
  for (int i = 0; i < node_vec.size(); i++) {
    // ast::dump_ast(move(node_vec.at(i)), 0);
    codegen::gen(move(node_vec.at(i)));
  }

  return 0;
}
