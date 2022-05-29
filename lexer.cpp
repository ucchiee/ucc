#include "lexer.h"

#include <cctype>
#include <iostream>
using namespace std;

Lexer::Token Lexer::TokenStream::get() {
  // needs impl
  char ch;

  do {  // skip space
    if (!is.get(ch)) return current_token = {Kind::end};
  } while (ch != '\n' && isspace(ch));

  switch (ch) {
    case 0:
      return current_token = {Kind::end};
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
    case '.':
      // number
      is.putback(ch);
      is >> current_token.number_value;
      current_token.kind = Kind::tk_int;  // TODO: ひとまずintだけ
      return current_token;
    default:
      if (isalpha(ch) || ch == '_') {
        // id, reserved(TODO)
        current_token.string_value = ch;
        while (is.get(ch)) {
          if (isalnum(ch) || ch == '_') {
            current_token.string_value += ch;
          } else {
            is.putback(ch);
            break;
          }
        }
        current_token.kind = {Kind::tk_id};
        return current_token;
      } else {
        // TODO: literal, ==, &&, ||
        // 1 char ascii symbol
        return current_token = {(Kind)ch};
      }
      // unexpected
      return current_token = {Kind::end};
  }
}
