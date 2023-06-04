cmake_minimum_required(VERSION 3.10)
project(gpwatchdog)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

add_subdirectory(u-bus)

# common
file(GLOB_RECURSE gpwd_common_src ${CMAKE_CURRENT_LIST_DIR}/common/**/*.cpp ${CMAKE_CURRENT_LIST_DIR}/common/**/*.hpp)

add_library(gpwd_common STATIC ${gpwd_common_src})
target_link_libraries(gpwd_common
    PUBLIC
        ubus)
target_include_directories(gpwd_common
    PUBLIC
        common/include)

# gpwd server
file(GLOB_RECURSE gpwd_server_src ${CMAKE_CURRENT_LIST_DIR}/server/**/*.cpp ${CMAKE_CURRENT_LIST_DIR}/server/**/*.hpp)

add_executable(gpwd_server
    ${gpwd_server_src})

target_link_libraries(gpwd_server
    PUBLIC
        ubus
        gpwd_common)

# gpwd cli
add_executable(gpwd_cli
    cli/src/main.cpp)

target_link_libraries(gpwd_cli
    PUBLIC
        ubus
        gpwd_common
        CLI11
        modern-json)

# gpwd client
add_library(gpwd_client
    STATIC
        client/src/gpwd_client.cpp)

target_link_libraries(gpwd_client
    PUBLIC
        ubus
        gpwd_common)

target_include_directories(gpwd_client
    PUBLIC
        client/include)

# target_include_directories(gpwd_server
#     PUBLIC
#         common/include)

# add_library(gpwd_client STATIC)
# target_link_libraries(gpwd_client
#     PUBLIC
#         ubus)

# add_executable(gpwd_cli
# )