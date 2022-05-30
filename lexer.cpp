#include "lexer.h"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
using namespace std;

Lexer::TokenStream::TokenStream(char* program)
    : m_program{program}, m_current_token_idx{0} {
  tokenize();
}

bool Lexer::TokenStream::consume(Lexer::Kind kind) {
  if (current().kind != kind) {
    return false;
  }
  m_current_token_idx++;
  return true;
}
bool Lexer::TokenStream::consume(char kind) {
  if (current().kind != Lexer::Kind(kind)) {
    return false;
  }
  m_current_token_idx++;
  return true;
}

void Lexer::TokenStream::expect(char op) {
  if (current().kind != Lexer::Kind(op)) {
    exit(-1);
  }
  m_current_token_idx++;
}

int Lexer::TokenStream::expect_number() {
  if (current().kind != Lexer::Kind::tk_int) {
    exit(-1);
  }
  int val = current().lexeme_number;
  m_current_token_idx++;
  return val;
}

bool Lexer::TokenStream::at_eof() {
  // do not forget Kind::end
  return !(m_current_token_idx < m_token_vec.size() - 1);
}

void Lexer::TokenStream::tokenize() {
  char* p;
  Token token;

  p = m_program;

  for (;;) {
    // skip space
    if (isspace(*p)) {
      p++;
      continue;
    }

    switch (*p) {
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
        token.lexeme_number = strtol(p, &p, 10);
        m_token_vec.push_back(token);
        continue;
      case '+':
      case '-':
      case '*':
      case '/':
      case '(':
      case ')':
        m_token_vec.push_back({Kind(*p++)});
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
