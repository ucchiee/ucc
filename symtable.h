#pragma once
#include <memory>

#include "lexer.h"

namespace symbol {

struct LVal {
  lexer::Token tok;
  int offset;
};

std::shared_ptr<symbol::LVal> find_lval(const lexer::Token &token);
std::shared_ptr<symbol::LVal> register_lval(const lexer::Token &token);

}  // namespace symbol
