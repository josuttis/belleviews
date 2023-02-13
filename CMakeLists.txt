cmake_minimum_required(VERSION 3.15)
project(BelleViews)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BELLEVIEW_WITH_TESTS "Enables tests in BelleViews library" ON)

if (BELLEVIEW_WITH_TESTS)
    enable_testing()
endif(BELLEVIEW_WITH_TESTS)

add_subdirectory(sources)
