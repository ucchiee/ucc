cmake_minimum_required(VERSION 3.1)
project(ucchiee_c_compiler CXX)
set(CMAKE_CXX_STANDARD 20)
add_executable(ucc main.cpp lexer.cpp ast.cpp parser.cpp gen.cpp)

# Enable the testing features.
enable_testing()

# Add the test which is simply run the application.
add_test(NAME run_test COMMAND ./test.sh)
