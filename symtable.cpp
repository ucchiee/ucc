#include "symtable.h"

#include <cstring>
#include <memory>

using namespace std;

symbol::SymTable::SymTable() : lval_table{} {}

symbol::SymTable::~SymTable() {}

shared_ptr<symbol::LVal> symbol::SymTable::find_lval(
    const lexer::Token& token) {
  // TODO: should element of lval_table be map
  for (auto i = lval_table.size() - 1; i != -1; i--) {
    for (auto lval : lval_table.at(i)) {
      if (token.len == lval->tok.len &&
          !memcmp(token.lexeme_string, lval->tok.lexeme_string, token.len)) {
        return lval;
      }
    }
  }
  return nullptr;
}

shared_ptr<symbol::LVal> symbol::SymTable::register_lval(
    const lexer::Token& token) {
  // TODO: should element of lval_table be map
  auto lval = make_shared<LVal>();
  lval->tok = token;
  size_t size = current().size();
  if (size != 0) {
    // There is more than one var
    lval->offset = current().at(size - 1)->offset + 8;
  } else {
    // This var is the first one
    lval->offset = 8;
  }
  current().push_back(lval);
  return lval;
}

vector<shared_ptr<symbol::LVal>>& symbol::SymTable::current() {
  return lval_table.at(lval_table.size() - 1);
}

void symbol::SymTable::begin_block() { lval_table.push_back({}); }

void symbol::SymTable::end_block() { lval_table.pop_back(); }

void symbol::SymTable::begin_funcdef() {
  lval_table.clear();
  lval_table.push_back({});
}

void symbol::SymTable::end_funcdef() {}
