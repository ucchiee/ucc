#include "type.h"

namespace type {

Type::Type() {}

Type::~Type() {}

void Type::add_int() { add_type(Kind::type_int); }
void Type::add_ptr() { add_type(Kind::type_int); }

void Type::add_type(Kind kind) { m_type_list.push_front(kind); }

const std::list<Kind>& Type::get_type_list() { return m_type_list; }

bool operator==(Type type1, Type type2) {
  auto type_list1 = type1.get_type_list();
  auto type_list2 = type2.get_type_list();
  if (type_list1.size() != type_list2.size()) {
    return false;
  }

  auto iter1 = type_list1.begin();
  auto iter2 = type_list2.begin();
  for (; iter1 != type_list1.end(); iter1++, iter2++) {
    if (*iter1 != *iter2) {
      return false;
    }
  }
  return true;
}

}  // namespace type
