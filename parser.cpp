#include "parser.h"

#include <memory>

#include "ast.h"
#include "lexer.h"

using namespace std;
using namespace Ast;

parser::Parser::Parser(Lexer::TokenStream &ts) : m_ts{ts} {}

unique_ptr<Node> parser::Parser::expr() {
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
  unique_ptr<Node> node = primary();

  for (;;) {
    if (m_ts.consume('*')) {
      node = create_node(NodeKind::nd_mul, move(node), primary());
    } else if (m_ts.consume('/')) {
      node = create_node(NodeKind::nd_div, move(node), primary());
    } else {
      return move(node);
    }
  }
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
