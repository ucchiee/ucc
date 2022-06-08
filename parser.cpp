#include "parser.h"

#include <cstring>
#include <memory>
#include <vector>

#include "ast.h"
#include "lexer.h"
#include "parser.h"

using namespace std;
using namespace Ast;

vector<unique_ptr<Node>> node_vec;
vector<shared_ptr<parser::LVal>> lval_vec;

parser::Parser::Parser(Lexer::TokenStream &ts) : m_ts{ts} {}

void parser::Parser::program() {
  while (!m_ts.at_eof()) {
    node_vec.push_back(move(stmt()));
  }
}

unique_ptr<Node> parser::Parser::stmt() {
  unique_ptr<Node> node;
  if (m_ts.consume(Lexer::Kind::kw_return)) {
    node = create_node(NodeKind::nd_return, expr());
    m_ts.expect(';');

  } else if (m_ts.consume(Lexer::Kind::kw_if)) {
    // if
    m_ts.expect('(');
    node = create_node(NodeKind::nd_if, expr());
    m_ts.expect(')');
    node->add_child(stmt());
    // else
    if (m_ts.consume(Lexer::Kind::kw_else)) {
      // change node kind
      node->kind = NodeKind::nd_ifelse;
      node->add_child(stmt());
    }

  } else if (m_ts.consume(Lexer::Kind::kw_while)) {
    // while
    m_ts.expect('(');
    node = create_node(NodeKind::nd_while, expr());
    m_ts.expect(')');
    node->add_child(stmt());

  } else if (m_ts.consume(Lexer::Kind::kw_for)) {
    // ( expr? ; expr? ; expr? ) stmt
    node = create_node(NodeKind::nd_for);
    m_ts.expect('(');
    // first and second expr
    for (int i = 0; i < 2; i++) {
      if (m_ts.consume(';')) {
        node->add_child(create_node(Ast::NodeKind::nd_blank));
      } else {
        node->add_child(expr());
        m_ts.expect(';');
      }
    }
    // third expr
    if (m_ts.consume(')')) {
      node->add_child(create_node(Ast::NodeKind::nd_blank));
    } else {
      node->add_child(expr());
      m_ts.expect(')');
    }
    node->add_child(stmt());

  } else if (m_ts.consume('{')) {
    node = create_node(NodeKind::nd_compound);
    while (!m_ts.consume('}')) {
      node->add_child(stmt());
    }
  } else {
    node = expr();
    m_ts.expect(';');
  }
  return node;
}

unique_ptr<Node> parser::Parser::expr() { return assign(); }

unique_ptr<Node> parser::Parser::assign() {
  unique_ptr<Node> node = equality();
  if (m_ts.consume('=')) {
    return create_node(NodeKind::nd_assign, move(node), assign());
  } else {
    return move(node);
  }
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
  Lexer::Token tok = m_ts.consume_ident();
  if (tok.kind != Lexer::Kind::end) {
    node = make_unique<Node>();
    node->kind = NodeKind::nd_lval;
    shared_ptr<LVal> lval = parser::find_lval(tok);
    if (lval) {
      // This var have already appeared.
      node->offset = lval->offset;
    } else {
      // This var have not appeared.
      unique_ptr<LVal> lval = make_unique<LVal>();
      lval->name = tok.lexeme_string;
      lval->len = tok.len;
      if (lval_vec.size()) {
        // There is more than one var
        lval->offset = lval_vec.at(lval_vec.size() - 1)->offset + 8;
      } else {
        // There is more than one var
        lval->offset = 8;
      }
      node->offset = lval->offset;
      lval_vec.push_back(move(lval));
    }
    return node;
  }

  int val = m_ts.expect_number();
  return move(create_num(val));
}

shared_ptr<parser::LVal> parser::find_lval(Lexer::Token token) {
  // ToDo: lval_vec should be map
  for (int i = 0; i < lval_vec.size(); i++) {
    if (token.len == lval_vec.at(i)->len &&
        !memcmp(token.lexeme_string, lval_vec.at(i)->name, token.len)) {
      return lval_vec.at(i);
    }
  }
  return nullptr;
}
