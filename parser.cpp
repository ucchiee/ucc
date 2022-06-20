#include "parser.h"

#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "symtable.h"

using namespace std;
using namespace ast;

extern vector<shared_ptr<symbol::LVal>> lval_vec;

parser::Parser::Parser(lexer::TokenStream& ts) : m_ts{ts} {}

unique_ptr<Node> parser::Parser::program() {
  auto node = create_node(NodeKind::nd_program);
  while (!m_ts.at_eof()) {
    node->add_child(funcdef());
  }
  return node;
}

unique_ptr<Node> parser::Parser::funcdef() {
  // ident "(" param_decl ("," param_decl)* ")" compound_stmt
  unique_ptr<Node> node = create_node(NodeKind::nd_funcdef);
  lexer::Token tok = m_ts.expect_ident();
  node->tok = tok;

  auto lval_vec_bak = lval_vec;  // TODO: need to fix
  lval_vec = {};

  m_ts.expect('(');
  int num_arg = 0;
  while (!m_ts.consume(')')) {
    auto arg_child = param_decl();
    arg_child->arg_idx = num_arg++;
    node->add_child(move(arg_child));
    m_ts.consume(',');
  }
  auto node_compound = compound_stmt();

  node->total_size = lval_vec.size() * 8;

  // assign lval_vec to that of compound_stmt
  node_compound->local = lval_vec;
  lval_vec = lval_vec_bak;  // TODO: need to fix

  node->add_child(move(node_compound));

  return node;
}

unique_ptr<Node> parser::Parser::param_decl() {
  // ident
  unique_ptr<Node> node = create_node(NodeKind::nd_param_decl);
  lexer::Token tok = m_ts.expect_ident();

  // register arg as a local value for now
  // TODO:
  // In the future, I need to fix this behavior.
  shared_ptr<symbol::LVal> lval = symbol::find_lval(tok);
  if (!lval) {
    lval = symbol::register_lval(tok);
  }
  node->offset = lval->offset;
  node->tok = tok;
  return node;
}

unique_ptr<Node> parser::Parser::stmt() {
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
        node->add_child(create_node(ast::NodeKind::nd_blank));
      } else {
        node->add_child(expr());
        m_ts.expect(';');
      }
    }
    // third expr
    if (m_ts.consume(')')) {
      node->add_child(create_node(ast::NodeKind::nd_blank));
    } else {
      node->add_child(expr());
      m_ts.expect(')');
    }
    node->add_child(stmt());

  } else if (m_ts.consume('{')) {
    m_ts.push_back('{');
    auto lval_vec_bak = lval_vec;  // TODO: need to fix
    node = compound_stmt();
    node->local = lval_vec;
    lval_vec = lval_vec_bak;  // TODO: need to fix
  } else {
    node = expr();
    m_ts.expect(';');
  }
  return node;
}

unique_ptr<Node> parser::Parser::compound_stmt() {
  // compound
  unique_ptr<Node> node = create_node(NodeKind::nd_compound);
  m_ts.expect('{');
  while (!m_ts.consume('}')) {
    node->add_child(stmt());
  }
  return node;
}

unique_ptr<Node> parser::Parser::expr() { return assign(); }

unique_ptr<Node> parser::Parser::assign() {
  unique_ptr<Node> node = equality();
  if (m_ts.consume('=')) {
    return create_node(NodeKind::nd_assign, move(node), assign());
  } else if (m_ts.consume(lexer::Kind::op_add_into)) {
    return create_node(NodeKind::nd_add_into, move(node), assign());
  } else {
    return move(node);
  }
}

unique_ptr<ast::Node> parser::Parser::equality() {
  unique_ptr<Node> node = relational();

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

unique_ptr<ast::Node> parser::Parser::relational() {
  unique_ptr<Node> node = add();

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

unique_ptr<Node> parser::Parser::add() {
  unique_ptr<Node> node = mul();

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

unique_ptr<Node> parser::Parser::mul() {
  unique_ptr<Node> node = unary();

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

unique_ptr<Node> parser::Parser::unary() {
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

unique_ptr<Node> parser::Parser::primary() {
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
      node = ast::create_node(NodeKind::nd_funcall);
      node->tok = tok;
      while (!m_ts.consume(')')) {
        node->add_child(move(expr()));
        m_ts.consume(',');
      }
      if (node->child_vec.size() > 6) {
        cerr << "Max num of arguments is 6" << endl;
      }
      return node;
    } else {
      // ident
      node = ast::create_node(NodeKind::nd_lval);
      shared_ptr<symbol::LVal> lval = symbol::find_lval(tok);
      if (!lval) {
        lval = symbol::register_lval(tok);
      }
      node->offset = lval->offset;
      return node;
    }
  }

  // num
  int val = m_ts.expect_number();
  return move(create_num(val));
}
