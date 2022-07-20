#pragma once
#include <list>
#include <memory>
#include <vector>

namespace type {

enum class Kind : int {
  type_func,
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

  friend std::shared_ptr<Type> create_type(Kind, int);
  friend bool operator==(Type, Type);

  // for pointer
  std::shared_ptr<Type> m_next;
  // for function
  std::shared_ptr<Type> m_ret_type;
  std::vector<std::shared_ptr<Type>> m_args_type;

 private:
  Kind m_kind;
  int m_size;
};

std::shared_ptr<Type> create_type(Kind, int);
std::shared_ptr<Type> create_int();
std::shared_ptr<Type> create_ptr();
std::shared_ptr<Type> create_func();

std::shared_ptr<Type> add_type(std::shared_ptr<Type> type, Kind kind, int size);
std::shared_ptr<Type> add_int(std::shared_ptr<Type> type);
std::shared_ptr<Type> add_ptr(std::shared_ptr<Type> type);

bool operator==(Type, Type);
bool operator==(std::shared_ptr<Type>, std::shared_ptr<Type>);
bool operator==(std::vector<std::shared_ptr<Type>>,
                std::vector<std::shared_ptr<Type>>);

}  // namespace type
