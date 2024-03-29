#include "lexer.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

#include "parser.h"
#include "symtable.h"

using namespace std;

namespace lexer {

bool operator==(Token tok1, Token tok2) {
  if (tok1.kind != tok2.kind) {
    return false;
  } else if (memcmp(tok1.lexeme_string, tok2.lexeme_string, tok1.len)) {
    return false;
  } else {
    return true;
  }
}

TokenStream::TokenStream(char* program)
    : m_program{program}, m_current_token_idx{0} {
  tokenize();
}

bool TokenStream::consume(Kind kind) {
  if (current().kind != kind) {
    return false;
  }
  m_current_token_idx++;
  return true;
}
bool TokenStream::consume(char kind) {
  if (current().kind != Kind(kind)) {
    return false;
  }
  m_current_token_idx++;
  return true;
}

Token TokenStream::consume_ident() {
  if (current().kind != Kind::tk_id) {
    return {Kind::end};
  }
  return m_token_vec[m_current_token_idx++];
}

void TokenStream::expect(char op) {
  if (current().kind != Kind(op)) {
    stringstream ss;
    ss << op << "is expected" << endl;
    error(ss.str());
  }
  m_current_token_idx++;
}

int TokenStream::expect_number() {
  if (current().kind != Kind::tk_int) {
    error("number is expected");
  }
  int val = current().lexeme_number;
  m_current_token_idx++;
  return val;
}

Token TokenStream::expect_ident() {
  if (current().kind != Kind::tk_id) {
    error("ident is expected");
  }
  return m_token_vec[m_current_token_idx++];
}

void TokenStream::push_back(Kind kind) {
  m_current_token_idx--;
  if (current().kind != kind) {
    error("failed to push_back");
  }
}
void TokenStream::push_back(char kind) {
  m_current_token_idx--;
  if (current().kind != Kind(kind)) {
    error("failed to push_back");
  }
}

bool TokenStream::at_eof() {
  // do not forget Kind::end
  return !(m_current_token_idx < m_token_vec.size() - 1);
}

void TokenStream::debug_current() {
  string str;
  str = {current().lexeme_string, (unsigned long)current().len};
  cerr << "kind: " << int(current().kind) << endl;
  cerr << "tok: " << str << endl;
  cerr << "lexeme_number: " << current().lexeme_number << endl;
}

void TokenStream::dump() {
  int backup = m_current_token_idx;
  string str;
  m_current_token_idx = 0;
  while (!at_eof()) {
    str = {current().lexeme_string, (unsigned long)current().len};
    cerr << str << endl;
    m_current_token_idx++;
  }
  m_current_token_idx = backup;
}

void TokenStream::tokenize() {
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
        if ((p + 1) && *(p + 1) == '=') {
          token = {Kind::op_add_into, p, 2};
          ++ ++p;
          m_token_vec.push_back(token);
          continue;
        }
      case '-':
      case '*':
      case '/':
      case '&':
      case '(':
      case ')':
      case '[':
      case ']':
      case '{':
      case '}':
      case ';':
      case ',':
      case '@':
        m_token_vec.push_back({Kind(*p), p, 1});
        ++p;
        continue;
      case '=':
        if ((p + 1) && *(p + 1) == '=') {
          token = {Kind::op_eq, p, 2};
          ++ ++p;
          m_token_vec.push_back(token);
          continue;
        } else {  // assign
          m_token_vec.push_back({(Kind)*p, p, 1});
          ++p;
          continue;
        }
      case '!':
        if ((p + 1) && *(p + 1) == '=') {
          token = {Kind::op_ne, p, 2};
          ++ ++p;
          m_token_vec.push_back(token);
          continue;
        }
      case '<':
        if ((p + 1) && *(p + 1) == '=') {  // <=
          token = {Kind::op_le, p, 2};
          ++ ++p;
          m_token_vec.push_back(token);
          continue;
        } else {  // <
          token = {Kind::op_lt, p, 1};
          ++p;
          m_token_vec.push_back(token);
          continue;
        }
      case '>':
        if ((p + 1) && *(p + 1) == '=') {  // >=
          token = {Kind::op_ge, p, 2};
          ++ ++p;
          m_token_vec.push_back(token);
          continue;
        } else {  // <
          token = {Kind::op_gt, p, 1};
          ++p;
          m_token_vec.push_back(token);
          continue;
        }
      default:
        if (isalpha(*p) || *p == '_') {
          // id, reserved(TODO)
          char* tmp = p;
          while (isalnum(*p) || *p == '_') ++p;
          int len = int(p - tmp);

          // Check whether this is reserved.
          if (len == 6 && memcmp(tmp, "return", 6) == 0) {
            token = {Kind::kw_return, tmp, len};
          } else if (len == 2 && memcmp(tmp, "if", 2) == 0) {
            token = {Kind::kw_if, tmp, len};
          } else if (len == 4 && memcmp(tmp, "else", 4) == 0) {
            token = {Kind::kw_else, tmp, len};
          } else if (len == 5 && memcmp(tmp, "while", 5) == 0) {
            token = {Kind::kw_while, tmp, len};
          } else if (len == 3 && memcmp(tmp, "for", 3) == 0) {
            token = {Kind::kw_for, tmp, len};
          } else if (len == 3 && memcmp(tmp, "int", 3) == 0) {
            token = {Kind::kw_int, tmp, len};
          } else if (len == 6 && memcmp(tmp, "sizeof", 6) == 0) {
            token = {Kind::kw_sizeof, tmp, len};
          } else {
            token = {Kind::tk_id, tmp, len};
          }
          m_token_vec.push_back(token);
          continue;
        }
        // unexpected
        m_token_vec.push_back({Kind::end});
        return;
    }
  }
}

const Token& TokenStream::current() {
  return m_token_vec.at(m_current_token_idx);
}

void TokenStream::error(string msg) {
  // determine num of spaces
  int loc = current().lexeme_string - m_program;
  string space = "";
  for (int i = 0; i < loc; i++) space += " ";

  // output error msg
  cerr << m_program << endl;
  cerr << space << "^ " << msg << endl;
  exit(1);
}
}  // namespace lexer
