#pragma once
#include <memory>
#include <vector>

#include "lexer.h"

namespace symbol {

struct LVal {
  lexer::Token tok;
  int offset;
};

class SymTable {
 public:
  SymTable();
  SymTable(SymTable &&) = default;
  SymTable(const SymTable &) = default;
  SymTable &operator=(SymTable &&) = default;
  SymTable &operator=(const SymTable &) = default;
  ~SymTable();

  std::shared_ptr<symbol::LVal> find_lval(const lexer::Token &token);
  std::shared_ptr<symbol::LVal> register_lval(const lexer::Token &token);
  std::vector<std::shared_ptr<LVal>> &current();
  void begin_block();
  void end_block();
  void begin_funcdef();
  void end_funcdef();

 private:
  std::vector<std::vector<std::shared_ptr<LVal>>> lval_table;
};

}  // namespace symbol
