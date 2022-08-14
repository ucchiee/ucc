#pragma once
#include <memory>
#include <string>

#include "ast.h"

namespace codegen {

const std::string label_prefix = ".L";

void gen(std::unique_ptr<ast::Node>);
void gen_lval(std::unique_ptr<ast::Node>);
std::string create_label(std::string name);
void set_idx_size(size_t size);

}  // namespace codegen
