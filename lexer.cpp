#include "lexer.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

#include "parser.h"

using namespace std;

extern vector<shared_ptr<parser::LVal>> lval_vec;

lexer::TokenStream::TokenStream(char* program)
    : m_program{program}, m_current_token_idx{0} {
  tokenize();
}

bool lexer::TokenStream::consume(lexer::Kind kind) {
  if (current().kind != kind) {
    return false;
  }
  m_current_token_idx++;
  return true;
}
bool lexer::TokenStream::consume(char kind) {
  if (current().kind != lexer::Kind(kind)) {
    return false;
  }
  m_current_token_idx++;
  return true;
}

lexer::Token lexer::TokenStream::consume_ident() {
  if (current().kind != lexer::Kind::tk_id) {
    return {lexer::Kind::end};
  }
  return m_token_vec[m_current_token_idx++];
}

void lexer::TokenStream::expect(char op) {
  if (current().kind != lexer::Kind(op)) {
    stringstream ss;
    ss << op << "is expected" << endl;
    error(ss.str());
  }
  m_current_token_idx++;
}

int lexer::TokenStream::expect_number() {
  if (current().kind != lexer::Kind::tk_int) {
    error("number is expected");
  }
  int val = current().lexeme_number;
  m_current_token_idx++;
  return val;
}

bool lexer::TokenStream::at_eof() {
  // do not forget Kind::end
  return !(m_current_token_idx < m_token_vec.size() - 1);
}

void lexer::TokenStream::debug_current() {
  cout << "kind: " << int(current().kind) << endl;
  cout << "lexeme_string: " << current().lexeme_string << endl;
  cout << "len: " << current().len << endl;
  cout << "lexeme_number: " << current().lexeme_number << endl;
}

void lexer::TokenStream::tokenize() {
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
      case '(':
      case ')':
      case '{':
      case '}':
      case ';':
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
          if (len == 6 && std::memcmp(tmp, "return", 6) == 0) {
            token = {Kind::kw_return, tmp, len};
          } else if (len == 2 && std::memcmp(tmp, "if", 2) == 0) {
            token = {Kind::kw_if, tmp, len};
          } else if (len == 4 && std::memcmp(tmp, "else", 4) == 0) {
            token = {Kind::kw_else, tmp, len};
          } else if (len == 5 && std::memcmp(tmp, "while", 5) == 0) {
            token = {Kind::kw_while, tmp, len};
          } else if (len == 3 && std::memcmp(tmp, "for", 3) == 0) {
            token = {Kind::kw_for, tmp, len};
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

const lexer::Token& lexer::TokenStream::current() {
  return m_token_vec.at(m_current_token_idx);
}

void lexer::TokenStream::error(string msg) {
  // determine num of spaces
  int loc = current().lexeme_string - m_program;
  string space = "";
  for (int i = 0; i < loc; i++) space += " ";

  // output error msg
  cerr << m_program << endl;
  cerr << space << "^ " << msg << endl;
  exit(1);
}
