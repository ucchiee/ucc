#include "type.h"

#include <memory>

using namespace std;

namespace type {

Type::Type() { m_next = nullptr; }

Type::~Type() {}

bool Type::is_kind_of(Kind kind) { return m_kind == kind; }

bool Type::is_ptr() { return is_kind_of(Kind::type_ptr); }

bool Type::is_m_ptr() { return is_kind_of(Kind::type_m_ptr); }

bool Type::is_arr() { return is_kind_of(Kind::type_arr); }

size_t Type::get_size() { return m_size; }

shared_ptr<Type> create_type(Kind kind, size_t size) {
  auto type = make_shared<Type>();
  type->m_kind = kind;
  type->m_size = size;
  return type;
}

shared_ptr<Type> create_int() { return create_type(Kind::type_int, 4); }

shared_ptr<Type> create_ptr() { return create_type(Kind::type_ptr, 8); }

shared_ptr<Type> create_func() { return create_type(Kind::type_func, -1); }

shared_ptr<Type> create_arr(shared_ptr<Type> type, size_t arr_size) {
  auto type_arr = create_type(Kind::type_arr, type->get_size() * arr_size);
  type_arr->m_arr_size = arr_size;
  type_arr->m_next = move(type);
  return type_arr;
}

shared_ptr<Type> arr_to_ptr(shared_ptr<Type> type) {
  if (type->is_arr()) {
    type = add_ptr(type->m_next);
  }
  return type;
}

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

shared_ptr<Type> add_m_ptr(shared_ptr<Type> type) {
  return add_type(type, Kind::type_m_ptr, 8);
}

bool operator==(const Type &type1, const Type &type2) {
  if (type1.m_kind == Kind::type_func && type2.m_kind == Kind::type_func) {
    // function type
    if (!operator==(*type1.m_ret_type, *type2.m_ret_type)) {
      return false;
      return operator==(type1.m_args_type, type2.m_args_type);
    } else if (type1.m_args_type.size() != type2.m_args_type.size()) {
      return false;
    } else {
      for (int i = 0; i < type1.m_args_type.size(); i++) {
        if (!operator==(type1.m_args_type.at(i), type2.m_args_type.at(i))) {
          return false;
        }
      }
      return true;
    }
  } else if (type1.m_next == nullptr && type2.m_next == nullptr) {
    // primitive
    return type1.m_kind == type2.m_kind;
  } else if (type1.m_next != nullptr && type2.m_next != nullptr) {
    // pointer
    return operator==(*type1.m_next, *type2.m_next);
  }
  return false;
}

bool operator!=(const Type &type1, const Type &type2) {
  return !operator==(type1, type2);
}

}  // namespace type
