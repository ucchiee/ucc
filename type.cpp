#include "type.h"

#include <memory>

using namespace std;

namespace type {

Type::Type() { m_next = NULL; }

Type::~Type() {}

shared_ptr<Type> create_type(Kind kind, int size) {
  auto type = make_shared<Type>();
  type->m_kind = kind;
  type->m_size = size;
  return type;
}

shared_ptr<Type> create_int() { return create_type(Kind::type_int, 4); }

shared_ptr<Type> create_ptr() { return create_type(Kind::type_ptr, 8); }

shared_ptr<Type> create_func() { return create_type(Kind::type_func, -1); }

shared_ptr<Type> add_type(shared_ptr<Type> type, Kind kind, int size) {
  auto new_type = create_type(kind, size);
  new_type->m_next = move(type);
  return new_type;
}

shared_ptr<Type> add_int(shared_ptr<Type> type) {
  return add_type(type, Kind::type_int, 4);
}

shared_ptr<Type> add_ptr(shared_ptr<Type> type) {
  return add_type(type, Kind::type_ptr, 8);
}

bool operator==(Type type1, Type type2) {
  if (type1.m_kind == Kind::type_func && type2.m_kind == Kind::type_func) {
    // function type
    if (operator==(type1.m_ret_type, type2.m_ret_type)) {
      return operator==(type1.m_args_type, type2.m_args_type);
    } else {
      return false;
    }
  } else if (type1.m_next == NULL && type2.m_next == NULL) {
    // primitive
    return type1.m_kind == type2.m_kind;
  } else if (type1.m_next != NULL && type2.m_next != NULL) {
    // pointer
    return operator==(type1.m_next, type2.m_next);
  }
  return false;
}

bool operator==(shared_ptr<Type> type1, shared_ptr<Type> type2) {
  return operator==(*type1, *type2);
}

bool operator==(vector<shared_ptr<Type>> vec_type1,
                vector<shared_ptr<Type>> vec_type2) {
  if (vec_type1.size() != vec_type2.size()) {
    return false;
  }
  for (int i = 0; i < vec_type1.size(); i++) {
    if (!operator==(vec_type1.at(i), vec_type2.at(i))) {
      return false;
    }
  }
  return true;
}

}  // namespace type
