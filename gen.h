#pragma once
#include <memory>

#include "ast.h"

namespace codegen {

void gen(std::unique_ptr<Ast::Node>);

}
