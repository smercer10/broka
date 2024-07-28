add_library(broka_lib order.cpp order_book.cpp)

target_include_directories(broka_lib PRIVATE ${CMAKE_SOURCE_DIR}/include/broka)

target_compile_features(broka_lib PRIVATE cxx_std_20)
