#include "symtable.h"

#include <cstring>
#include <memory>

using namespace std;

vector<shared_ptr<symbol::LVal>> lval_vec;

shared_ptr<symbol::LVal> symbol::find_lval(const lexer::Token& token) {
  // TODO: should lval_vec be map
  for (auto lval : lval_vec) {
    if (token.len == lval->tok.len &&
        !memcmp(token.lexeme_string, lval->tok.lexeme_string, token.len)) {
      return lval;
    }
  }
  return nullptr;
}

shared_ptr<symbol::LVal> symbol::register_lval(const lexer::Token& token) {
  // ToDo: lval_vec should be map
  auto lval = make_shared<LVal>();
  lval->tok = token;
  if (lval_vec.size()) {
    // There is more than one var
    lval->offset = lval_vec.at(lval_vec.size() - 1)->offset + 8;
  } else {
    // This var is the first one
    lval->offset = 8;
  }
  lval_vec.push_back(lval);
  return lval;
}
