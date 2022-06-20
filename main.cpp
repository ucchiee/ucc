#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

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

  lexer::TokenStream ts{argv[1]};
  parser::Parser parser{ts};
  auto root = parser.program();

  cout << ".intel_syntax noprefix" << endl;

  codegen::gen(move(root));

  return 0;
}
