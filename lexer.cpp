#include "lexer.h"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
using namespace std;

Lexer::TokenStream::TokenStream(char* program)
    : program{program}, current_token_idx{0} {
  tokeninze();
}

bool Lexer::TokenStream::consume(Lexer::Kind kind) {
  if (current().kind != kind) {
    return false;
  }
  current_token_idx++;
  return true;
}

int Lexer::TokenStream::expect_number() {
  if (current().kind != Lexer::Kind::tk_int) {
    exit(-1);
  }
  current_token_idx++;
  return current().lexeme_number;
}

void Lexer::TokenStream::tokeninze() {
  char ch;
  istringstream is{program};
  Token token;

  // skip space
  is >> ws;

  for (;;) {
    switch (is.peek()) {
      case 0:
        token_vec.push_back({Kind::end});
        return;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        // number
        token.kind = Kind::tk_int;  // TODO: ひとまずintだけ
        is >> token.lexeme_number;
        token_vec.push_back(token);
        continue;
      default:
        // if (isalpha(ch) || ch == '_') {
        //   // id, reserved(TODO)
        //   current_token.string_value = ch;
        //   while (is.get(ch)) {
        //     if (isalnum(ch) || ch == '_') {
        //       current_token.string_value += ch;
        //     } else {
        //       is.putback(ch);
        //       break;
        //     }
        //   }
        //   current_token.kind = {Kind::tk_id};
        //   return current_token;
        // } else {
        //   // TODO: literal, ==, &&, ||
        //   // 1 char ascii symbol
        //   return current_token = {(Kind)ch};
        // }
        // unexpected
        token_vec.push_back({Kind::end});
        return;
    }
  }
}

const Lexer::Token& Lexer::TokenStream::current() {
  return token_vec.at(current_token_idx);
}
