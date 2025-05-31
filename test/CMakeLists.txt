# Unity source and include path
set(UNITY_SRC ../unity/unity.c)
set(UNITY_INC ../unity)

# Include paths for tests
include_directories(
    ${UNITY_INC}
    ../include
    ${ZeroMQ_INCLUDE_DIRS}
)

# Main simulith tests executable
add_executable(test_simulith test_simulith.c ${UNITY_SRC})
target_link_libraries(test_simulith simulith ${ZeroMQ_LIBRARIES} pthread)
add_test(NAME SimulithTest COMMAND test_simulith)

# I2C tests executable
add_executable(test_i2c test_i2c.c ${UNITY_SRC})
target_link_libraries(test_i2c simulith ${ZeroMQ_LIBRARIES})
add_test(NAME I2CTest COMMAND test_i2c)

# UART tests executable
add_executable(test_uart test_uart.c ${UNITY_SRC})
target_link_libraries(test_uart simulith ${ZeroMQ_LIBRARIES})
add_test(NAME UARTTest COMMAND test_uart)
