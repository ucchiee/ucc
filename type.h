#pragma once
#include <list>

namespace type {

enum class Kind : int {
  type_int,
  type_ptr,
};

class Type {
 public:
  Type();
  Type(Type &&) = default;
  Type(const Type &) = default;
  Type &operator=(Type &&) = default;
  Type &operator=(const Type &) = default;
  ~Type();

  void add_int();
  void add_ptr();

  const std::list<Kind> &get_type_list();

 private:
  void add_type(Kind kind);
  std::list<Kind> m_type_list;
};

bool operator==(Type, Type);

}  // namespace type
