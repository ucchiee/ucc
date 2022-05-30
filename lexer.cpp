#include "lexer.h"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
using namespace std;

Lexer::TokenStream::TokenStream(char* program)
    : m_program{program}, m_current_token_idx{0} {
  tokeninze();
}

bool Lexer::TokenStream::consume(Lexer::Kind kind) {
  if (current().kind != kind) {
    return false;
  }
  m_current_token_idx++;
  return true;
}

int Lexer::TokenStream::expect_number() {
  if (current().kind != Lexer::Kind::tk_int) {
    exit(-1);
  }
  m_current_token_idx++;
  return current().lexeme_number;
}

void Lexer::TokenStream::tokeninze() {
  char ch;
  istringstream is{m_program};
  Token token;

  for (;;) {
    // skip space
    is >> ws;

    switch (is.peek()) {
      case 0:
        m_token_vec.push_back({Kind::end});
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
        m_token_vec.push_back(token);
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
        m_token_vec.push_back({Kind::end});
        return;
    }
  }
}

const Lexer::Token& Lexer::TokenStream::current() {
  return m_token_vec.at(m_current_token_idx);
}
