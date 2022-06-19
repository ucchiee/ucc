#include "lval.h"

#include <cstring>
#include <memory>

using namespace std;

vector<shared_ptr<symbol::LVal>> lval_vec;

shared_ptr<symbol::LVal> symbol::find_lval(const lexer::Token& token) {
  // ToDo: lval_vec should be map
  for (int i = 0; i < lval_vec.size(); i++) {
    if (token.len == lval_vec.at(i)->len &&
        !memcmp(token.lexeme_string, lval_vec.at(i)->name, token.len)) {
      return lval_vec.at(i);
    }
  }
  return nullptr;
}

shared_ptr<symbol::LVal> symbol::register_lval(const lexer::Token& token) {
  // ToDo: lval_vec should be map
  shared_ptr<LVal> lval = make_shared<LVal>();
  lval->name = token.lexeme_string;
  lval->len = token.len;
  if (lval_vec.size()) {
    // There is more than one var
    lval->offset = lval_vec.at(lval_vec.size() - 1)->offset + 8;
  } else {
    // This var is the first one
    lval->offset = 8;
  }
  lval_vec.push_back(move(lval));
  return lval;
}
