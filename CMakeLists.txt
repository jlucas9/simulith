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
    src/simulith_i2c.c
    src/simulith_uart.c
)

# Build Simulith static library
add_library(simulith STATIC ${SIMULITH_SOURCES})
target_link_libraries(simulith ${ZeroMQ_LIBRARIES})

# Add tests subdirectory
enable_testing()
add_subdirectory(test)
