cmake_minimum_required(VERSION 3.1)
project(ucchiee_c_compiler CXX)
set(CMAKE_CXX_STANDARD 20)
add_subdirectory(fmt)
add_executable(ucc main.cpp lexer.cpp ast.cpp parser.cpp gen.cpp symtable.cpp type.cpp)
target_link_libraries(ucc fmt)
