#pragma once
#include <list>
#include <memory>
#include <vector>

namespace type {

enum class Kind : int {
  type_func,
  type_int,
  type_ptr,
  type_arr,
};

class Type {
 public:
  Type();
  Type(Type &&) = default;
  Type(const Type &) = default;
  Type &operator=(Type &&) = default;
  Type &operator=(const Type &) = default;
  ~Type();

  bool is_kind_of(Kind);
  bool is_ptr();
  bool is_arr();
  size_t get_size();

  friend std::shared_ptr<Type> create_type(Kind, size_t);
  friend std::shared_ptr<Type> create_arr(std::shared_ptr<Type> type,
                                          size_t arr_size);
  friend bool operator==(const Type &, const Type &);

  // for pointer
  std::shared_ptr<Type> m_next;
  // for function
  std::shared_ptr<Type> m_ret_type;
  std::vector<std::shared_ptr<Type>> m_args_type;

 private:
  Kind m_kind;
  size_t m_size;
  size_t m_arr_size;
};

std::shared_ptr<Type> create_type(Kind, size_t);
std::shared_ptr<Type> create_int();
std::shared_ptr<Type> create_ptr();
std::shared_ptr<Type> create_func();
std::shared_ptr<Type> create_arr(std::shared_ptr<Type> type, size_t arr_size);
std::shared_ptr<Type> arr_to_ptr(std::shared_ptr<Type> type);

std::shared_ptr<Type> add_type(std::shared_ptr<Type> type, Kind kind, int size);
std::shared_ptr<Type> add_int(std::shared_ptr<Type> type);
std::shared_ptr<Type> add_ptr(std::shared_ptr<Type> type);

bool operator==(const Type &, const Type &);
bool operator!=(const Type &, const Type &);

}  // namespace type
