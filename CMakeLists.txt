cmake_minimum_required(VERSION 3.13)
project(hashmaps)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_FLAGS "-O3")
set(CMAKE_CXX_FLAGS "-O3")

set(SEPARATE_CHAINING implementations/separate_chaining/hashmap_sc.c implementations/separate_chaining/hashmap_sc.h)
set(LINEAR_PROBING implementations/linear_probing/hashmap_lp.c implementations/linear_probing/hashmap_lp.h)
set(QUADRATIC_PROBING implementations/quadratic_probing/hashmap_qp.c implementations/quadratic_probing/hashmap_qp.h)
set(DOUBLE_HASHING implementations/double_hashing/hashmap_dh.c implementations/double_hashing/hashmap_dh.h)

add_executable(separate_chaining_test implementations/separate_chaining/hashmap_sc_test.c ${SEPARATE_CHAINING})
add_executable(linear_probing_test implementations/linear_probing/hashmap_lp_test.c ${LINEAR_PROBING})
add_executable(quadratic_probing_test implementations/quadratic_probing/hashmap_qp_test.c ${QUADRATIC_PROBING})
add_executable(double_hashing_test implementations/double_hashing/hashmap_dh_test.c ${DOUBLE_HASHING})

add_executable(performance_test performance_test.cpp ${SEPARATE_CHAINING} ${LINEAR_PROBING} ${QUADRATIC_PROBING} ${DOUBLE_HASHING})