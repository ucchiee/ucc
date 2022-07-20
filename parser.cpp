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

  auto node_compound = compound_stmt();

  // TODO : this should be changed, a little bit ugly
  node->total_size = symbol::size(symtable.local_current()) * 8;

  // assign lval_vec to that of compound_stmt
  node_compound->local = symtable.local_current();
  symtable.end_funcdef();

  node->add_child(move(node_compound));

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
    return create_node(NodeKind::nd_assign, move(node), assign());
  } else if (m_ts.consume(lexer::Kind::op_add_into)) {
    return create_node(NodeKind::nd_add_into, move(node), assign());
  } else {
    return move(node);
  }
}

unique_ptr<Node> Parser::equality() {
  auto node = relational();

  for (;;) {
    if (m_ts.consume(lexer::Kind::op_eq)) {
      node = create_node(NodeKind::nd_eq, move(node), relational());
    } else if (m_ts.consume(lexer::Kind::op_ne)) {
      node = create_node(NodeKind::nd_ne, move(node), relational());
    } else {
      return move(node);
    }
  }
}

unique_ptr<Node> Parser::relational() {
  auto node = add();

  for (;;) {
    if (m_ts.consume(lexer::Kind::op_lt)) {
      node = create_node(NodeKind::nd_lt, move(node), add());
    } else if (m_ts.consume(lexer::Kind::op_le)) {
      node = create_node(NodeKind::nd_le, move(node), add());
    } else if (m_ts.consume(lexer::Kind::op_gt)) {
      node = create_node(NodeKind::nd_lt, add(), move(node));
    } else if (m_ts.consume(lexer::Kind::op_ge)) {
      node = create_node(NodeKind::nd_le, add(), move(node));
    } else {
      return move(node);
    }
  }
}

unique_ptr<Node> Parser::add() {
  auto node = mul();

  for (;;) {
    if (m_ts.consume('+')) {
      node = create_node(NodeKind::nd_add, move(node), mul());
    } else if (m_ts.consume('-')) {
      node = create_node(NodeKind::nd_sub, move(node), mul());
    } else {
      return move(node);
    }
  }
}

unique_ptr<Node> Parser::mul() {
  auto node = unary();

  for (;;) {
    if (m_ts.consume('*')) {
      node = create_node(NodeKind::nd_mul, move(node), unary());
    } else if (m_ts.consume('/')) {
      node = create_node(NodeKind::nd_div, move(node), unary());
    } else {
      return move(node);
    }
  }
}

unique_ptr<Node> Parser::unary() {
  if (m_ts.consume('+')) {
    return primary();
  } else if (m_ts.consume('-')) {
    return create_node(NodeKind::nd_sub, create_num(0), primary());
  } else if (m_ts.consume('*')) {
    return create_node(NodeKind::nd_deref, unary());
  } else if (m_ts.consume('&')) {
    return create_node(NodeKind::nd_addr, unary());
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
      node->tok = tok;
      while (!m_ts.consume(')')) {
        node->add_child(move(expr()));
        m_ts.consume(',');
      }
      if (node->child_vec.size() > 6) {
        cerr << "Max num of arguments is 6" << endl;
      }
      // TODO : return type
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
  return move(create_num(val));
}
}  // namespace parser
