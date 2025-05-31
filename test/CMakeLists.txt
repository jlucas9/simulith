# Unity source and include path
set(UNITY_SRC ../unity/unity.c)
set(UNITY_INC ../unity)

# Include paths for tests
include_directories(
    ${UNITY_INC}
    ../include
    ${ZeroMQ_INCLUDE_DIRS}
)

# Functional tests executable
add_executable(test_simulith test_simulith.c ${UNITY_SRC})
target_link_libraries(test_simulith simulith ${ZeroMQ_LIBRARIES} pthread)
add_test(NAME SimulithTest COMMAND test_simulith)
