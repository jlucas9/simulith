cmake_minimum_required(VERSION 3.10)
project(Simulith C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find ZeroMQ via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZeroMQ REQUIRED libzmq)

include_directories(
    include
    unity
    ${ZeroMQ_INCLUDE_DIRS}
)

# Simulith library sources
set(SIMULITH_SOURCES
    src/simulith_common.c
    src/simulith_client.c
    src/simulith_server.c
)

# Build Simulith static library
add_library(simulith STATIC ${SIMULITH_SOURCES})
target_link_libraries(simulith ${ZeroMQ_LIBRARIES})

# Unit test executable
enable_testing()
add_executable(test_simulith test/test_simulith.c unity/unity.c)
target_include_directories(test_simulith PRIVATE
    unity
    include
    ${ZeroMQ_INCLUDE_DIRS}
)
target_link_libraries(test_simulith
    simulith
    ${ZeroMQ_LIBRARIES}
    pthread
)
add_test(NAME SimulithTest COMMAND test_simulith)
