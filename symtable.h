#pragma once
#include <memory>
#include <utility>
#include <vector>

#include "lexer.h"

namespace symbol {

class Symbol {
 public:
  Symbol();
  Symbol(Symbol &&) = default;
  Symbol(const Symbol &) = default;
  Symbol &operator=(Symbol &&) = default;
  Symbol &operator=(const Symbol &) = default;
  ~Symbol();

  std::shared_ptr<Symbol> next;

  lexer::Token tok;
  std::shared_ptr<type::Type> type;
  int offset;       // local

 private:
};

std::shared_ptr<Symbol> create_symbol(const lexer::Token &tok,
                                      std::shared_ptr<type::Type> type,
                                      int offset);
std::shared_ptr<Symbol> add_symbol(std::shared_ptr<Symbol>,
                                   std::shared_ptr<Symbol>);
std::shared_ptr<Symbol> find_symbol(std::shared_ptr<Symbol> symbol,
                                    const lexer::Token &tok);
int size(std::shared_ptr<Symbol>);

class SymTable {
 public:
  SymTable();
  SymTable(SymTable &&) = default;
  SymTable(const SymTable &) = default;
  SymTable &operator=(SymTable &&) = default;
  SymTable &operator=(const SymTable &) = default;
  ~SymTable();

  std::shared_ptr<symbol::Symbol> find_local(const lexer::Token &token);
  std::shared_ptr<symbol::Symbol> find_local_current_scope(
      const lexer::Token &token);
  std::shared_ptr<symbol::Symbol> register_local(
      std::pair<lexer::Token, std::shared_ptr<type::Type>>);
  std::shared_ptr<Symbol> local_current();
  void begin_block();
  void end_block();
  void begin_funcdef();
  void end_funcdef();

 private:
  std::vector<std::shared_ptr<Symbol>> m_local;  // local variables
  std::shared_ptr<Symbol> m_global;
  int get_last_offset();
};

}  // namespace symbol
