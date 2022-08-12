#include "parser.h"

#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "symtable.h"
#include "type.h"

using namespace std;
using namespace ast;

namespace parser {

symbol::SymTable symtable;

Parser::Parser(lexer::TokenStream& ts) : m_ts{ts} {}

unique_ptr<Node> Parser::program() {
  auto node = create_node(NodeKind::nd_program);
  while (!m_ts.at_eof()) {
    node->add_child(funcdef());
  }
  return node;
}

shared_ptr<type::Type> Parser::type_specifier() {
  if (m_ts.consume(lexer::Kind::kw_int)) {
    return type::create_int();
  } else {
    m_ts.error("unknown type");
    return NULL;  // never reached
  }
}

pair<lexer::Token, shared_ptr<type::Type>> Parser::declarator(
    shared_ptr<type::Type> type) {
  while (m_ts.consume('*')) {
    type = type::add_ptr(type);
  }
  lexer::Token tok = m_ts.expect_ident();
  if (m_ts.consume('[')) {
    int arr_size = m_ts.expect_number();
    type = type::create_arr(move(type), arr_size);
    m_ts.expect(']');
  }
  return {tok, type};
}

pair<lexer::Token, shared_ptr<type::Type>> Parser::param_decl() {
  return declarator(type_specifier());
}

unique_ptr<Node> Parser::funcdef() {
  // ident "(" param_decl ("," param_decl)* ")" compound_stmt
  auto node = create_node(NodeKind::nd_funcdef);
  auto [tok, type] = param_decl();

  auto func_type = type::create_func();
  func_type->m_ret_type = type;
  // function type is holded by node
  node->tok = tok;

  symtable.begin_funcdef();

  m_ts.expect('(');
  int num_arg = 0;
  while (!m_ts.consume(')')) {
    auto [tok, type] = param_decl();
    func_type->m_args_type.push_back(type);

    auto arg_child = register_args_as_local({tok, type});
    arg_child->arg_idx = num_arg++;
    node->add_child(move(arg_child));
    m_ts.consume(',');
  }
  // register and check(TODO) function definition
  node->type = func_type;
  auto symbol = symtable.find_global(tok);

  if (m_ts.consume(';')) {
    // function prototype
    symtable.register_global({tok, func_type}, false);
    // change node type
    node->kind = NodeKind::nd_funcdecl;
  } else {
    // function definition
    // check whether this is already definded
    if (symbol) {
      if (symbol->is_defined) {
        m_ts.error("Already defined function");
      } else if (*symbol->type != *func_type) {
        m_ts.error("Function type is not as same as declared");
      }
    }

    symtable.register_global({tok, func_type}, true);
    auto node_compound = compound_stmt();

    // TODO : this should be changed, a little bit ugly
    // node->total_size = symbol::size(symtable.local_current()) * 8;
    node->total_size = symtable.get_last_offset();

    // assign lval_vec to that of compound_stmt
    node_compound->local = symtable.local_current();

    node->add_child(move(node_compound));
  }

  symtable.end_funcdef();
  return node;
}

unique_ptr<Node> Parser::register_args_as_local(
    pair<lexer::Token, shared_ptr<type::Type>> tok_type_pair) {
  // type_specifier declarator
  auto node = create_node(NodeKind::nd_arg_decl);
  auto [tok, type] = tok_type_pair;

  // register arg as a local value for now
  // TODO:
  // In the future, I need to fix this behavior.
  auto lval = symtable.find_local(tok);
  if (lval) {
    m_ts.error("Redefinition of arguments");
  }
  lval = symtable.register_local({tok, type});
  node->offset = lval->offset;
  node->tok = tok;
  node->type = type;
  return node;
}

unique_ptr<Node> Parser::stmt() {
  unique_ptr<Node> node;
  if (m_ts.consume(lexer::Kind::kw_return)) {
    // return
    node = create_node(NodeKind::nd_return, expr());
    m_ts.expect(';');

  } else if (m_ts.consume(lexer::Kind::kw_if)) {
    // if
    m_ts.expect('(');
    node = create_node(NodeKind::nd_if, expr());
    m_ts.expect(')');
    node->add_child(stmt());
    // else
    if (m_ts.consume(lexer::Kind::kw_else)) {
      // change node kind
      node->kind = NodeKind::nd_ifelse;
      node->add_child(stmt());
    }

  } else if (m_ts.consume(lexer::Kind::kw_while)) {
    // while
    m_ts.expect('(');
    node = create_node(NodeKind::nd_while, expr());
    m_ts.expect(')');
    node->add_child(stmt());

  } else if (m_ts.consume(lexer::Kind::kw_for)) {
    // ( expr? ; expr? ; expr? ) stmt
    node = create_node(NodeKind::nd_for);
    m_ts.expect('(');
    // first and second expr
    for (int i = 0; i < 2; i++) {
      if (m_ts.consume(';')) {
        node->add_child(create_node(NodeKind::nd_blank));
      } else {
        node->add_child(expr());
        m_ts.expect(';');
      }
    }
    // third expr
    if (m_ts.consume(')')) {
      node->add_child(create_node(NodeKind::nd_blank));
    } else {
      node->add_child(expr());
      m_ts.expect(')');
    }
    node->add_child(stmt());

  } else if (m_ts.consume('{')) {
    m_ts.push_back('{');

    symtable.begin_block();

    node = compound_stmt();
    node->local = symtable.local_current();

    symtable.end_block();
  } else {
    node = expr();
    m_ts.expect(';');
  }
  return node;
}

unique_ptr<Node> Parser::compound_stmt() {
  // compound
  auto node = create_node(NodeKind::nd_compound);
  m_ts.expect('{');
  while (m_ts.consume(lexer::Kind::kw_int)) {
    m_ts.push_back(lexer::Kind::kw_int);
    auto [tok, type] = param_decl();
    auto lval = symtable.find_local_current_scope(tok);
    if (lval) {
      m_ts.error("Redefinition of ident");
    }
    lval = symtable.register_local({tok, type});
    m_ts.expect(';');
  }
  while (!m_ts.consume('}')) {
    node->add_child(stmt());
  }
  return node;
}

unique_ptr<Node> Parser::expr() { return assign(); }

unique_ptr<Node> Parser::assign() {
  auto node = equality();
  if (m_ts.consume('=')) {
    auto node_r = assign();
    auto [type_l, type_r] = convert_type(node->type, node_r->type);
    auto type = check_and_merge_type(type_l, type_r, (lexer::Kind)'=');
    return create_node(NodeKind::nd_assign, type, move(node), move(node_r));
  } else if (m_ts.consume(lexer::Kind::op_add_into)) {
    auto node_r = assign();
    auto [type_l, type_r] = convert_type(node->type, node_r->type);
    auto type = check_and_merge_type(type_l, type_r, lexer::Kind::op_add_into);
    return create_node(NodeKind::nd_add_into, type, move(node), move(node_r));
  } else {
    return node;
  }
}

unique_ptr<Node> Parser::equality() {
  auto node = relational();

  for (;;) {
    if (m_ts.consume(lexer::Kind::op_eq)) {
      auto node_r = relational();
      auto [type_l, type_r] = convert_type(node->type, node_r->type);
      check_and_merge_type(type_l, type_r, lexer::Kind::op_eq);
      node = create_node(NodeKind::nd_eq, type::create_int(), move(node),
                         move(node_r));
    } else if (m_ts.consume(lexer::Kind::op_ne)) {
      auto node_r = relational();
      auto [type_l, type_r] = convert_type(node->type, node_r->type);
      check_and_merge_type(type_l, type_r, lexer::Kind::op_ne);
      node = create_node(NodeKind::nd_ne, type::create_int(), move(node),
                         move(node_r));
    } else {
      return node;
    }
  }
}

unique_ptr<Node> Parser::relational() {
  auto node = add();

  for (;;) {
    if (m_ts.consume(lexer::Kind::op_lt)) {
      auto node_r = add();
      auto [type_l, type_r] = convert_type(node->type, node_r->type);
      auto type = check_and_merge_type(type_l, type_r, lexer::Kind::op_lt);
      node = create_node(NodeKind::nd_lt, type, move(node), move(node_r));
    } else if (m_ts.consume(lexer::Kind::op_le)) {
      auto node_r = add();
      auto [type_l, type_r] = convert_type(node->type, node_r->type);
      auto type = check_and_merge_type(type_l, type_r, lexer::Kind::op_le);
      node = create_node(NodeKind::nd_le, type, move(node), move(node_r));
    } else if (m_ts.consume(lexer::Kind::op_gt)) {
      auto node_l = add();
      auto [type_l, type_r] = convert_type(node_l->type, node->type);
      auto type = check_and_merge_type(type_l, type_r, lexer::Kind::op_gt);
      node = create_node(NodeKind::nd_lt, type, move(node_l), move(node));
    } else if (m_ts.consume(lexer::Kind::op_ge)) {
      auto node_l = add();
      auto [type_l, type_r] = convert_type(node_l->type, node->type);
      auto type = check_and_merge_type(type_l, type_r, lexer::Kind::op_ge);
      node = create_node(NodeKind::nd_le, type, move(node_l), move(node));
    } else {
      return node;
    }
  }
}

unique_ptr<Node> Parser::add() {
  auto node = mul();

  for (;;) {
    if (m_ts.consume('+')) {
      auto node_r = mul();
      auto [type_l, type_r] = convert_type(node->type, node_r->type);
      auto type = check_and_merge_type(type_l, type_r, lexer::Kind('+'));
      if (type_l->is_ptr_arr()) {
        node_r->val *= type_l->m_next->get_size();
      } else if (type_r->is_ptr_arr()) {
        node->val *= type_r->m_next->get_size();
      }
      node = create_node(NodeKind::nd_add, type, move(node), move(node_r));
    } else if (m_ts.consume('-')) {
      auto node_r = mul();
      auto [type_l, type_r] = convert_type(node->type, node_r->type);
      auto type = check_and_merge_type(type_l, type_r, lexer::Kind('-'));
      if (type_l->is_ptr_arr()) {
        node_r->val *= type_l->m_next->get_size();
      }
      node = create_node(NodeKind::nd_sub, type, move(node), move(node_r));
    } else {
      return node;
    }
  }
}

unique_ptr<Node> Parser::mul() {
  auto node = unary();

  for (;;) {
    if (m_ts.consume('*')) {
      auto node_r = unary();
      auto [type_l, type_r] = convert_type(node->type, node_r->type);
      auto type = check_and_merge_type(type_l, type_r, lexer::Kind('*'));
      node = create_node(NodeKind::nd_mul, type, move(node), move(node_r));
    } else if (m_ts.consume('/')) {
      auto node_r = unary();
      auto [type_l, type_r] = convert_type(node->type, node_r->type);
      auto type = check_and_merge_type(type_l, type_r, lexer::Kind('/'));
      node = create_node(NodeKind::nd_div, type, move(node), move(node_r));
    } else {
      return node;
    }
  }
}

unique_ptr<Node> Parser::unary() {
  if (m_ts.consume('+')) {
    return unary();
  } else if (m_ts.consume('-')) {
    auto node_r = unary();
    auto node_l = create_num(0, node_r->type);
    auto type = node_r->type;
    return create_node(NodeKind::nd_sub, type, move(node_l), move(node_r));
  } else if (m_ts.consume('*')) {
    auto node = unary();
    if (!node->type->is_ptr_arr()) {
      m_ts.error("ptr/arr is expected");
    }
    auto type = node->type;
    return create_node(NodeKind::nd_deref, type->m_next, move(node));
  } else if (m_ts.consume('&')) {
    auto node = unary();
    auto type = node->type;
    return create_node(NodeKind::nd_addr, type::add_ptr(type), move(node));
  } else if (m_ts.consume(lexer::Kind::kw_sizeof)) {
    bool has_p = m_ts.consume('(');
    auto node = add();
    if (has_p) m_ts.expect(')');
    return ast::create_num(node->type->get_size());
  }
  return primary();
}

unique_ptr<Node> Parser::primary() {
  unique_ptr<Node> node;
  // '(' expr ')'
  if (m_ts.consume('(')) {
    node = expr();
    m_ts.expect(')');
    return node;
  }
  // ident ('(' ')')?
  lexer::Token tok = m_ts.consume_ident();
  if (tok.kind != lexer::Kind::end) {
    if (m_ts.consume('(')) {
      // funcall, ident '(' ')'
      node = create_node(NodeKind::nd_funcall);

      // check function is declared
      auto symbol = symtable.find_global(tok);
      if (symbol) {
        node->type = symbol->type->m_ret_type;
      } else {
        // implicit declaration of function
        node->type = type::create_int();
      }
      node->tok = tok;

      size_t i = 0;
      while (!m_ts.consume(')')) {
        // type check of arguments
        auto arg_node = expr();
        if (symbol) {
          // Only if function as already defined or declared.
          if (*(arg_node->type) != *(symbol->type->m_args_type.at(i++))) {
            m_ts.error("Argument is not the same as declared.");
          }
        }
        node->add_child(move(arg_node));
        m_ts.consume(',');
      }
      if (node->child_vec.size() > 6) {
        m_ts.error("Max num of arguments is 6");
      }
      return node;
    } else {
      // ident
      node = create_node(NodeKind::nd_lval);
      auto symbol = symtable.find_local(tok);
      if (!symbol) {
        m_ts.error("Not defined ident");
      }
      node->tok = symbol->tok;
      node->type = symbol->type;
      node->offset = symbol->offset;
      return node;
    }
  }

  // num
  int val = m_ts.expect_number();
  return create_num(val);
}

pair<shared_ptr<type::Type>, shared_ptr<type::Type>> Parser::convert_type(
    shared_ptr<type::Type> type_l, shared_ptr<type::Type> type_r) {
  return {type_l, type_r};
}

shared_ptr<type::Type> Parser::check_and_merge_type(
    shared_ptr<type::Type> type1, shared_ptr<type::Type> type2,
    lexer::Kind op) {
  if (type1->is_ptr_arr() && type2->is_ptr_arr()) {
    if (op == (lexer::Kind)'-') {
      // return type::create_int();
      m_ts.error("This operation is not supported yet.");
    } else if ((int)op == '=') {
      return type1;
    } else {
      m_ts.error("type is incompatible");
    }

  } else if (type1->is_ptr_arr()) {
    if (type2->is_kind_of(type::Kind::type_func)) {
      m_ts.error("type is incompatible");
    }
    if (op == (lexer::Kind)'+' || op == (lexer::Kind)'-') {
      return type1;
    } else {
      m_ts.error("type is incompatible");
    }

  } else if (type2->is_ptr_arr()) {
    if (type1->is_kind_of(type::Kind::type_func)) {
      m_ts.error("type is incompatible");
    }
    if (op == (lexer::Kind)'+') {
      return type2;
    } else {
      m_ts.error("type is incompatible");
    }

  } else {
    if (*type1 == *type2) {
      return type1;
    } else {
      m_ts.error("type is incompatible");
    }
  }
  return type1;
}
}  // namespace parser
