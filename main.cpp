#include <iostream>
#include <sstream>

#include "lexer.h"
using namespace std;

int main(int argc, char** argv) {
  if (argc != 2) {
    cerr << "invalid # of args" << endl;
    return 1;
  }

  Lexer::TokenStream ts{argv[1]};

  cout << ".intel_syntax noprefix" << endl;
  cout << ".globl main" << endl;
  cout << "main:" << endl;
  cout << "  mov rax, " << ts.expect_number() << endl;
  while (!ts.at_eof()) {
    if (ts.consume('+')) {
      cout << "  add rax, " << ts.expect_number() << endl;
    } else if (ts.consume('-')) {
      cout << "  sub rax, " << ts.expect_number() << endl;
    } else {
      break;
    }
  }
  cout << "  ret" << endl;
  return 0;
}
