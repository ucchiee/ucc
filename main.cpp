#include <iostream>
#include <sstream>

#include "lexer.h"
using namespace std;

int main(int argc, char** argv) {
  if (argc != 2) {
    cerr << "invalid # of args" << endl;
    return 1;
  }

  std::istringstream iss{argv[1]};
  Lexer::TokenStream ts{iss};

  cout << ".intel_syntax noprefix" << endl;
  cout << ".globl main" << endl;
  cout << "main:" << endl;
  cout << "  mov rax, " << ts.expect_number() << endl;
  cout << "  ret" << endl;
  return 0;
}
