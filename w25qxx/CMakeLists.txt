add_library(w25qxx w25qxx.c w25qxx_Demo.c)
target_include_directories(w25qxx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})