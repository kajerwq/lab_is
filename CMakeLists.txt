cmake_minimum_required(VERSION 3.20)
project(taskflow_osipov LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(BUILD_HTTP_SERVER "Build HTTP server" ON)

add_library(taskflow
    src/task_service.cpp
    src/in_memory_task_repository.cpp
    src/priority_strategy.cpp
)
target_include_directories(taskflow PUBLIC include)
target_compile_options(taskflow PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

if(ENABLE_ASAN AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(taskflow PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(taskflow PRIVATE -fsanitize=address)
endif()

add_executable(taskflow_cli app/main.cpp)
target_link_libraries(taskflow_cli PRIVATE taskflow)

enable_testing()
add_executable(taskflow_tests
    tests/test_main.cpp
    tests/test_task_service.cpp
    tests/test_patterns.cpp
)
target_link_libraries(taskflow_tests PRIVATE taskflow)
if(ENABLE_ASAN AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(taskflow_tests PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(taskflow_tests PRIVATE -fsanitize=address)
    target_compile_options(taskflow_cli PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(taskflow_cli PRIVATE -fsanitize=address)
endif()
add_test(NAME taskflow_tests COMMAND taskflow_tests)

if(BUILD_HTTP_SERVER)
    add_executable(taskflow_server api/taskflow_server.cpp)
    target_link_libraries(taskflow_server PRIVATE taskflow)
    if(ENABLE_ASAN AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(taskflow_server PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(taskflow_server PRIVATE -fsanitize=address)
    endif()
endif()
