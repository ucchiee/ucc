#pragma once
#include <memory>
#include <string>

#include "ast.h"

namespace codegen {

const std::string label_prefix = ".L";

void gen(std::unique_ptr<Ast::Node>);
void gen_lval(std::unique_ptr<Ast::Node>);
std::string create_label(std::string name);

}  // namespace codegen
