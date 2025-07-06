#include <unity.h>
#include "simulith_uart.h"
#include <string.h>

static int32_t uart_a_handles[SIMULITH_MAX_UART_PORTS];
static int32_t uart_b_handles[SIMULITH_MAX_UART_PORTS];

void setUp(void)
{
    printf("\ntest_uart.c setUp..\n");
    memset(uart_a_handles, 0, sizeof(uart_a_handles));
    memset(uart_b_handles, 0, sizeof(uart_b_handles));
}

void tearDown(void)
{
    printf("\ntest_uart.c tearDown..\n");
    // Close all ports
    for (int i = 0; i < SIMULITH_MAX_UART_PORTS; i++)
    {
        simulith_uart_close(uart_a_handles[i]);
        simulith_uart_close(uart_b_handles[i]);
    }
}

void test_uart_init(void)
{
    int result;

    result = simulith_uart_init(0, &uart_a_handles[0]);
    TEST_ASSERT_EQUAL(SIMULITH_UART_SUCCESS, result);

    result = simulith_uart_init(0, &uart_b_handles[0]);
    TEST_ASSERT_EQUAL(SIMULITH_UART_SUCCESS, result);

    result = simulith_uart_init(1, &uart_a_handles[1]);
    TEST_ASSERT_EQUAL(SIMULITH_UART_SUCCESS, result);

    result = simulith_uart_init(1, &uart_b_handles[1]);
    TEST_ASSERT_EQUAL(SIMULITH_UART_SUCCESS, result);

    result = simulith_uart_init(SIMULITH_MAX_UART_PORTS-1, &uart_a_handles[SIMULITH_MAX_UART_PORTS-1]);
    TEST_ASSERT_EQUAL(SIMULITH_UART_SUCCESS, result);

    result = simulith_uart_init(SIMULITH_MAX_UART_PORTS-1, &uart_b_handles[SIMULITH_MAX_UART_PORTS-1]);
    TEST_ASSERT_EQUAL(SIMULITH_UART_SUCCESS, result);
}

void test_uart_send_receive(void)
{
    int result;
    uint8_t test_data[] = {0x12, 0x34, 0x56};
    uint8_t rx_data[sizeof(test_data)];

    // Initialize A side
    result = simulith_uart_init(0, &uart_a_handles[0]);
    TEST_ASSERT_EQUAL(0, uart_a_handles[0]);
    TEST_ASSERT_EQUAL(SIMULITH_UART_SUCCESS, result);
    
    // Initialize B side
    result = simulith_uart_init(0, &uart_b_handles[0]);
    TEST_ASSERT_EQUAL(0 + SIMULITH_MAX_UART_PORTS, uart_b_handles[0]);
    TEST_ASSERT_EQUAL(SIMULITH_UART_SUCCESS, result);

    // Send from A to B
    result = simulith_uart_send(uart_a_handles[0], test_data, sizeof(test_data));
    TEST_ASSERT_EQUAL(sizeof(test_data), result);

    // Confirm available on A
    result = simulith_uart_available(uart_a_handles[0]);
    TEST_ASSERT_EQUAL(0, result);

    // Confirm available on B
    result = simulith_uart_available(uart_b_handles[0]);
    TEST_ASSERT_EQUAL(sizeof(test_data), result);

    // Receive data on B
    result = simulith_uart_receive(uart_b_handles[0], rx_data, sizeof(rx_data));
    TEST_ASSERT_EQUAL(sizeof(test_data), result);

    // Send from B to A
    result = simulith_uart_send(uart_b_handles[0], rx_data, sizeof(rx_data));
    TEST_ASSERT_EQUAL(sizeof(rx_data), result);

    // Confirm available on A
    result = simulith_uart_available(uart_a_handles[0]);
    TEST_ASSERT_EQUAL(sizeof(test_data), result);
    
    // Confirm available on B
    result = simulith_uart_available(uart_b_handles[0]);
    TEST_ASSERT_EQUAL(0, result);

    // Receive data on A
    result = simulith_uart_receive(uart_a_handles[0], rx_data, sizeof(rx_data));
    TEST_ASSERT_EQUAL(sizeof(test_data), result);

    // Confirm available on A
    result = simulith_uart_available(uart_a_handles[0]);
    TEST_ASSERT_EQUAL(0, result);
    
    // Confirm available on B
    result = simulith_uart_available(uart_b_handles[0]);
    TEST_ASSERT_EQUAL(0, result);

    // Close A
    result = simulith_uart_close(uart_a_handles[0]);
    TEST_ASSERT_EQUAL(SIMULITH_UART_SUCCESS, result);

    // Close B
    result = simulith_uart_close(uart_b_handles[0]);
    TEST_ASSERT_EQUAL(SIMULITH_UART_SUCCESS, result);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_uart_init);
    RUN_TEST(test_uart_send_receive);
    return UNITY_END();
}