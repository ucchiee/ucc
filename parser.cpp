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
unique_ptr<ast::Node> node_func;

char l_rc_update[10] = {"rc_update"};
char l_rc_delete[10] = {"rc_delete"};

Parser::Parser(lexer::TokenStream& ts) : m_ts{ts} {}

unique_ptr<Node> Parser::program() {
  auto node = create_node(NodeKind::nd_program);
  while (!m_ts.at_eof()) {
    symtable.begin_funcdef();

    auto [tok, type] = declarator(type_specifier(), true);
    auto symbol = symtable.find_global(tok);

    if (m_ts.consume(';')) {
      // global variable or function declaration
      if (type->is_kind_of(type::Kind::type_func)) {
        // Function declaration
        if (symbol) {
          m_ts.push_back(';');
          if (symbol->is_defined) {
            m_ts.error("function with the same name is already defined.");
          } else {
            m_ts.error("function with the same name is already declared.");
          }
        }
        symtable.register_global({tok, type}, false);
      } else {
        // TODO: Global variables.
        if (symbol) {
          m_ts.push_back(';');
          m_ts.error("Reuse of the same name.");
        }
        auto node_gval = create_node(NodeKind::nd_gval_def, type);
        node_gval->tok = tok;
        node->add_child(move(node_gval));
        symtable.register_global({tok, type}, false);
      }

    } else {
      // function definition
      // First change the type of node
      node_func->kind = ast::NodeKind::nd_funcdef;

      // Check whether this is already defined.
      // And then, register as global.
      if (symbol) {
        if (symbol->is_defined) {
          m_ts.error("Already defined function");
        } else if (*symbol->type != *type) {
          m_ts.error("Function type is not as same as declared");
        }
      }
      symtable.register_global({tok, type}, true);

      // Parse function body and set total size
      auto node_compound = compound_stmt();
      node_func->total_size = symtable.get_last_offset();

      // Register info of local variables
      node_compound->local = symtable.local_current();
      node_func->add_child(move(node_compound));

      // Add node_func as a child of the top node.
      node->add_child(move(node_func));
    }

    symtable.end_funcdef();
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
    shared_ptr<type::Type> type, bool global) {
  for (;;) {
    if (m_ts.consume('*')) {
      type = type::add_ptr(type);
    } else if (m_ts.consume('@')) {
      type = type::add_m_ptr(type);
    } else {
      break;
    }
  }
  lexer::Token tok = m_ts.expect_ident();
  if (m_ts.consume('[')) {
    // Array
    int arr_size = m_ts.expect_number();
    type = type::create_arr(move(type), arr_size);
    m_ts.expect(']');

  } else if (m_ts.consume('(')) {
    if (!global) m_ts.error("Function definition/declaration is not allowed");
    // Function definition or declaration

    // Renew global variable `node_func`.
    // Change this node type if this turns out to be function definition.
    // See Parser::program().
    node_func = create_node(NodeKind::nd_funcdecl);
    node_func->tok = tok;

    // Create type.
    auto func_type = type::create_func();
    func_type->m_ret_type = type;

    // Parse function arguments.
    int num_arg = 0;
    while (!m_ts.consume(')')) {
      auto [tok, param_type] = param_decl();
      func_type->m_args_type.push_back(param_type);

      auto arg_child = register_args_as_local({tok, param_type});
      arg_child->arg_idx = num_arg++;
      // Register local variables to the global varialbe `node_func`
      node_func->add_child(move(arg_child));
      m_ts.consume(',');
    }
    node_func->type = func_type;
    // Need to return func_type.
    type = func_type;
  }
  return {tok, type};
}

pair<lexer::Token, shared_ptr<type::Type>> Parser::param_decl() {
  return declarator(type_specifier());
}

unique_ptr<Node> Parser::register_args_as_local(
    pair<lexer::Token, shared_ptr<type::Type>> tok_type_pair) {
  // type_specifier declarator
  auto node = create_node(NodeKind::nd_arg_decl);
  auto [tok, type] = tok_type_pair;

  // register arg as a local value for now
  // TODO:
  // In the future, I need to fix this behavior.
  if (symtable.find_local(tok)) {
    m_ts.error("Redefinition of arguments");
  }
  auto lval = symtable.register_local({tok, type});
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
    // Call to rc_delete to decrement reference counter.
    for (auto&& node_call : create_rc_delete_calls()) {
      node->add_child(move(node_call));
    }
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
    if (symtable.find_local_current_scope(tok)) {
      m_ts.error("Redefinition of ident");
    }
    auto lval = symtable.register_local({tok, type});
    m_ts.expect(';');
  }
  while (!m_ts.consume('}')) {
    node->add_child(stmt());
  }

  // Call to rc_delete to decrement reference counter.
  for (auto&& node_call : create_rc_delete_calls()) {
    node->add_child(move(node_call));
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

    // Check type (if type is m_ptr, then call rc_update)
    if (type->is_m_ptr()) {
      // Call rc_update to assign m_ptr.
      auto node_addr =
          create_node(NodeKind::nd_addr, type::add_ptr(type_l), move(node));
      // Create nd_funcall node.
      auto node_func = create_node(NodeKind::nd_funcall, type);
      node_func->add_child(move(node_addr));
      node_func->add_child(move(node_r));
      lexer::Token tok = {
          lexer::Kind::tk_id, l_rc_update,
          static_cast<int>(strlen(l_rc_update)),  // that is, 9
      };
      node_func->tok = tok;
      return node_func;
    } else {
      // Normal assignment
      return create_node(NodeKind::nd_assign, type, move(node), move(node_r));
    }
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

unique_ptr<Node> Parser::add(bool convert_arr) {
  auto node = mul(convert_arr);

  for (;;) {
    if (m_ts.consume('+')) {
      auto node_r = mul(convert_arr);
      auto [type_l, type_r] = convert_type(node->type, node_r->type);
      auto type = check_and_merge_type(type_l, type_r, lexer::Kind('+'));
      if (type_l->is_ptr()) {
        node_r->val *= type_l->m_next->get_size();
      } else if (type_r->is_ptr()) {
        node->val *= type_r->m_next->get_size();
      }
      node = create_node(NodeKind::nd_add, type, move(node), move(node_r));
    } else if (m_ts.consume('-')) {
      auto node_r = mul(convert_arr);
      auto [type_l, type_r] = convert_type(node->type, node_r->type);
      auto type = check_and_merge_type(type_l, type_r, lexer::Kind('-'));
      if (type_l->is_ptr()) {
        node_r->val *= type_l->m_next->get_size();
      }
      node = create_node(NodeKind::nd_sub, type, move(node), move(node_r));
    } else {
      return node;
    }
  }
}

unique_ptr<Node> Parser::mul(bool convert_arr) {
  auto node = unary(convert_arr);

  for (;;) {
    if (m_ts.consume('*')) {
      auto node_r = unary(convert_arr);
      auto [type_l, type_r] = convert_type(node->type, node_r->type);
      auto type = check_and_merge_type(type_l, type_r, lexer::Kind('*'));
      node = create_node(NodeKind::nd_mul, type, move(node), move(node_r));
    } else if (m_ts.consume('/')) {
      auto node_r = unary(convert_arr);
      auto [type_l, type_r] = convert_type(node->type, node_r->type);
      auto type = check_and_merge_type(type_l, type_r, lexer::Kind('/'));
      node = create_node(NodeKind::nd_div, type, move(node), move(node_r));
    } else {
      return node;
    }
  }
}

unique_ptr<Node> Parser::unary(bool convert_arr) {
  if (m_ts.consume('+')) {
    return unary(convert_arr);
  } else if (m_ts.consume('-')) {
    auto node_r = unary(convert_arr);
    auto node_l = create_num(0, node_r->type);
    auto type = node_r->type;
    return create_node(NodeKind::nd_sub, type, move(node_l), move(node_r));
  } else if (m_ts.consume('*')) {
    auto node = unary(convert_arr);
    if (!node->type->is_ptr()) {
      m_ts.error("ptr/arr is expected");
    }
    auto type = node->type;
    return create_node(NodeKind::nd_deref, type->m_next, move(node));
  } else if (m_ts.consume('&')) {
    auto node = unary(false);
    auto type = node->type;
    return create_node(NodeKind::nd_addr, type::add_ptr(type), move(node));
  } else if (m_ts.consume('@')) {
    auto node = unary(convert_arr);
    if (!node->type->is_m_ptr()) {
      m_ts.error("m_ptr is expected");
    }
    auto type = node->type;
    return create_node(NodeKind::nd_deref, type->m_next, move(node));
  } else if (m_ts.consume(lexer::Kind::kw_sizeof)) {
    bool has_p = m_ts.consume('(');
    auto node = add(false);
    if (has_p) m_ts.expect(')');
    return ast::create_num(node->type->get_size());
  }
  return primary(convert_arr);
}

unique_ptr<Node> Parser::primary(bool convert_arr) {
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
      auto symbol = symtable.find_local(tok);
      if (symbol) {
        // local variable
        node = create_node(NodeKind::nd_lval);
      } else {
        symbol = symtable.find_global(tok);
        if (symbol) {
          // global variable
          node = create_node(NodeKind::nd_gval);
        } else {
          m_ts.error("Not defined symbol.");
        }
      }
      node->tok = symbol->tok;
      node->type = symbol->type;
      // convert arr to ptr
      if (convert_arr && node->type->is_arr()) {
        auto type = type::arr_to_ptr(node->type);
        node = create_node(NodeKind::nd_addr, type, move(node));
      }
      node->offset = symbol->offset;

      if (m_ts.consume('[')) {
        auto type = node->type;
        auto node_add = create_node(NodeKind::nd_add, type, move(node), expr());
        node = create_node(NodeKind::nd_deref, type->m_next, move(node_add));
        m_ts.consume(']');
      }
      return node;
    }
  }

  // num
  int val = m_ts.expect_number();
  node = create_num(val);
  if (m_ts.consume('[')) {
    auto node_expr = expr();
    auto type = node_expr->type;
    auto node_add =
        create_node(NodeKind::nd_add, type, move(node), move(node_expr));
    node = create_node(NodeKind::nd_deref, type->m_next, move(node_add));
    m_ts.consume(']');
  }
  return node;
}

pair<shared_ptr<type::Type>, shared_ptr<type::Type>> Parser::convert_type(
    shared_ptr<type::Type> type_l, shared_ptr<type::Type> type_r) {
  type_l = type::arr_to_ptr(move(type_l));
  type_r = type::arr_to_ptr(move(type_r));
  return {type_l, type_r};
}

shared_ptr<type::Type> Parser::check_and_merge_type(
    shared_ptr<type::Type> type1, shared_ptr<type::Type> type2,
    lexer::Kind op) {
  if (type1->is_ptr() && type2->is_ptr()) {
    if (op == (lexer::Kind)'-') {
      // return type::create_int();
      m_ts.error("This operation is not supported yet.");
    } else if ((int)op == '=') {
      return type1;
    } else {
      m_ts.error("type is incompatible");
    }

  } else if (type1->is_ptr()) {
    if (type2->is_kind_of(type::Kind::type_func)) {
      m_ts.error("type is incompatible");
    }
    if (op == (lexer::Kind)'+' || op == (lexer::Kind)'-') {
      return type1;
    } else {
      m_ts.error("type is incompatible");
    }

  } else if (type2->is_ptr()) {
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

std::vector<unique_ptr<ast::Node>> create_rc_delete_calls() {
  vector<unique_ptr<ast::Node>> v;
  // Call to rc_delete to decrement reference counter.
  for (auto symbol : symtable.find_all_mptr_in_current_scope()) {
    // Create nd_lval node.
    auto node_lval = create_node(NodeKind::nd_lval_m_ptr);
    node_lval->tok = symbol->tok;
    node_lval->type = symbol->type;  // type_m_ptr
    node_lval->offset = symbol->offset;

    // Create nd_funcall node.
    auto node_funcall = create_node(NodeKind::nd_funcall, move(node_lval));
    node_funcall->type = type::create_int();  // TODO: Should be void.
    lexer::Token tok_func = {lexer::Kind::tk_id, l_rc_delete,
                             static_cast<int>(strlen(l_rc_delete))};
    node_funcall->tok = tok_func;

    // Add to the compound node
    v.push_back(move(node_funcall));
  }
  return v;
}

}  // namespace parser
