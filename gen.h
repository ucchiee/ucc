#pragma once
#include <memory>

#include "ast.h"

namespace codegen {

void gen(std::unique_ptr<Ast::Node>);
void gen_lval(std::unique_ptr<Ast::Node>);

}
