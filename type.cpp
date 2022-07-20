#include "type.h"

#include <memory>

namespace type {

Type::Type() { next = NULL; }

Type::~Type() {}

std::shared_ptr<Type> create_type(Kind kind, int size) {
  auto type = std::make_shared<Type>();
  type->kind = kind;
  type->size = size;
  return type;
}

std::shared_ptr<Type> add_type(std::shared_ptr<Type> type, Kind kind,
                               int size) {
  auto new_type = create_type(kind, size);
  new_type->next = move(type);
  return new_type;
}

std::shared_ptr<Type> add_int(std::shared_ptr<Type> type) {
  return add_type(type, Kind::type_int, 4);
}

std::shared_ptr<Type> add_ptr(std::shared_ptr<Type> type) {
  return add_type(type, Kind::type_ptr, 8);
}

bool operator==(Type type1, Type type2) {
  if (type1.kind == type2.kind && type1.next == NULL && type2.next == NULL) {
    return true;
  } else if (type1.next != NULL && type2.next != NULL) {
    return operator==(type1.next, type2.next);
  }
  return false;
}

bool operator==(std::shared_ptr<Type> type1, std::shared_ptr<Type> type2) {
  if (type1->kind == type2->kind && type2->next == NULL &&
      type2->next == NULL) {
    return true;
  } else if (type1->next != NULL && type2->next != NULL) {
    return operator==(type1, type2);
  }
  return false;
}

}  // namespace type
