#include "symtable.h"

#include <cstring>
#include <memory>

using namespace std;

namespace symbol {

Symbol::Symbol() { next = nullptr; }

Symbol::~Symbol() {}

shared_ptr<Symbol> create_symbol(const lexer::Token& tok,
                                 shared_ptr<type::Type> type, int offset,
                                 bool is_defined) {
  auto symbol = make_shared<Symbol>();
  symbol->tok = tok;
  symbol->offset = offset;
  symbol->type = type;
  symbol->is_defined = is_defined;
  return symbol;
}

shared_ptr<Symbol> add_symbol(shared_ptr<Symbol> symbol_base,
                              shared_ptr<Symbol> symbol) {
  if (symbol_base == nullptr) {
    return symbol;
  } else {
    symbol->next = symbol_base;
    return symbol;
  }
}

shared_ptr<Symbol> find_symbol(shared_ptr<Symbol> symbol,
                               const lexer::Token& tok) {
  if (symbol == nullptr) {
    return nullptr;
  } else if (symbol->tok == tok) {
    return symbol;
  } else {
    return find_symbol(symbol->next, tok);
  }
}

int size(shared_ptr<Symbol> symbol) {
  if (symbol == nullptr) {
    return 0;
  } else {
    return 1 + size(symbol->next);
  }
}

SymTable::SymTable() : m_local{} {}

SymTable::~SymTable() {}

shared_ptr<Symbol> SymTable::find_local(const lexer::Token& token) {
  for (auto i = m_local.size() - 1; i != -1; i--) {
    auto symbol = find_symbol(m_local.at(i), token);
    if (symbol != nullptr) {
      return symbol;
    }
  }
  return nullptr;
}

shared_ptr<Symbol> SymTable::find_local_current_scope(
    const lexer::Token& token) {
  return find_symbol(local_current(), token);
}

shared_ptr<Symbol> SymTable::register_local(
    pair<lexer::Token, shared_ptr<type::Type>> tok_type_pair) {
  auto [tok, type] = tok_type_pair;
  auto symbol = create_symbol(tok, type, get_last_offset() + type->get_total_size());

  // update current (add symbol)
  auto new_current = add_symbol(local_current(), symbol);
  m_local.pop_back();
  m_local.push_back(new_current);

  return symbol;
}

shared_ptr<Symbol> SymTable::find_global(const lexer::Token& token) {
  return find_symbol(m_global, token);
}

shared_ptr<Symbol> SymTable::register_global(
    pair<lexer::Token, shared_ptr<type::Type>> tok_type_pair, bool is_defined) {
  auto [tok, type] = tok_type_pair;
  auto symbol = create_symbol(tok, type, -1, is_defined);

  auto new_global = add_symbol(global_current(), symbol);
  m_global = new_global;

  return symbol;
}

shared_ptr<Symbol> SymTable::local_current() { return m_local.back(); }

shared_ptr<Symbol> SymTable::global_current() { return m_global; }

void SymTable::begin_block() { m_local.push_back(nullptr); }

void SymTable::end_block() { m_local.pop_back(); }

void SymTable::begin_funcdef() {
  m_local.clear();
  m_local.push_back(nullptr);
}

void SymTable::end_funcdef() {}

int SymTable::get_last_offset() {
  for (auto i = m_local.size() - 1; i != -1; i--) {
    if (m_local.at(i) == nullptr) continue;
    return m_local.at(i)->offset;
  }
  return 0;
}
}  // namespace symbol
