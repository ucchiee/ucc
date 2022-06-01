#include "parser.h"

#include <memory>

#include "ast.h"
#include "lexer.h"

using namespace std;
using namespace Ast;

parser::Parser::Parser(Lexer::TokenStream &ts) : m_ts{ts} {}

unique_ptr<Node> parser::Parser::expr() {
  return equality();
}

unique_ptr<Ast::Node> parser::Parser::equality() {
  unique_ptr<Node> node = relational();

  for (;;) {
    if (m_ts.consume(Lexer::Kind::op_eq)) {
      node = create_node(NodeKind::nd_eq, move(node), relational());
    } else if (m_ts.consume(Lexer::Kind::op_neq)) {
      node = create_node(NodeKind::nd_neq, move(node), relational());
    } else {
      return move(node);
    }
  }
}

unique_ptr<Ast::Node> parser::Parser::relational() {
  unique_ptr<Node> node = add();

  for (;;) {
    if (m_ts.consume(Lexer::Kind::op_le)) {
      node = create_node(NodeKind::nd_le, move(node), add());
    } else if (m_ts.consume(Lexer::Kind::op_leq)) {
      node = create_node(NodeKind::nd_leq, move(node), add());
    } else if (m_ts.consume(Lexer::Kind::op_gr)) {
      node = create_node(NodeKind::nd_le, add(), move(node));
    } else if (m_ts.consume(Lexer::Kind::op_greq)) {
      node = create_node(NodeKind::nd_leq, add(), move(node));
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
  }
  return primary();
}

unique_ptr<Node> parser::Parser::primary() {
  unique_ptr<Node> node;
  if (m_ts.consume('(')) {
    node = expr();
    m_ts.expect(')');
    return node;
  }

  int val = m_ts.expect_number();
  return move(create_num(val));
}
