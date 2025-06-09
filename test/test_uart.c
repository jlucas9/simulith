#include <unity.h>
#include "simulith_uart.h"
#include <string.h>

static uint8_t rx_data[256];
static size_t rx_len;
static uint8_t rx_port;

void setUp(void)
{
    printf("\ntest_uart.c setUp..\n");
    rx_len = 0;
    rx_port = 0;
    memset(rx_data, 0, sizeof(rx_data));
}

void tearDown(void)
{
    printf("\ntest_uart.c tearDown..\n");
    // Close all ports
    for (int i = 0; i < MAX_UART_PORTS * 2; i++)
    {
        simulith_uart_close(i);
    }
}

static int rx_callback(uint8_t port, const uint8_t *data, size_t len)
{
    rx_port = port;
    rx_len = len;
    memcpy(rx_data, data, len);
    return len;  // Return number of bytes processed
}

void test_uart_init_basic(void)
{
    int result = simulith_uart_init(0, rx_callback);
    TEST_ASSERT_EQUAL(0, result);

    result = simulith_uart_init(1, rx_callback);
    TEST_ASSERT_EQUAL(1, result);
}

void test_uart_init_auto_assignment(void)
{
    // Initialize first pair
    int port0 = simulith_uart_init(0, rx_callback);
    TEST_ASSERT_EQUAL(0, port0);
    
    int port1 = simulith_uart_init(1, rx_callback);
    TEST_ASSERT_EQUAL(1, port1);

    // Try to initialize port 0 again - should get next available pair
    int port0_new = simulith_uart_init(0, rx_callback);
    TEST_ASSERT_EQUAL(MAX_UART_PORTS, port0_new);

    // Initialize its pair
    int port1_new = simulith_uart_init(1, rx_callback);
    TEST_ASSERT_EQUAL(MAX_UART_PORTS + 1, port1_new);
}

void test_uart_send_receive_basic(void)
{
    simulith_uart_init(0, rx_callback);
    simulith_uart_init(1, rx_callback);

    uint8_t test_data[] = {0x12, 0x34, 0x56};
    int sent = simulith_uart_send(0, test_data, sizeof(test_data));

    TEST_ASSERT_EQUAL(sizeof(test_data), sent);
    TEST_ASSERT_EQUAL(1, rx_port);
    TEST_ASSERT_EQUAL(sizeof(test_data), rx_len);
    TEST_ASSERT_EQUAL_MEMORY(test_data, rx_data, sizeof(test_data));
}

void test_uart_send_receive_extended_ports(void)
{
    // Initialize extended ports (MAX_UART_PORTS and MAX_UART_PORTS + 1)
    int port0 = simulith_uart_init(0, rx_callback);
    int port1 = simulith_uart_init(1, rx_callback);
    TEST_ASSERT_EQUAL(0, port0);
    TEST_ASSERT_EQUAL(1, port1);

    int port2 = simulith_uart_init(0, rx_callback);  // Should get MAX_UART_PORTS
    int port3 = simulith_uart_init(1, rx_callback);  // Should get MAX_UART_PORTS + 1
    TEST_ASSERT_EQUAL(MAX_UART_PORTS, port2);
    TEST_ASSERT_EQUAL(MAX_UART_PORTS + 1, port3);

    // Test sending data between extended ports
    uint8_t test_data[] = {0xAA, 0xBB, 0xCC};
    int sent = simulith_uart_send(port2, test_data, sizeof(test_data));

    TEST_ASSERT_EQUAL(sizeof(test_data), sent);
    TEST_ASSERT_EQUAL(port3, rx_port);
    TEST_ASSERT_EQUAL(sizeof(test_data), rx_len);
    TEST_ASSERT_EQUAL_MEMORY(test_data, rx_data, sizeof(test_data));
}

void test_uart_send_receive_multiple_pairs(void)
{
    // Initialize multiple port pairs
    int ports[4];
    ports[0] = simulith_uart_init(0, rx_callback);  // Should get 0
    ports[1] = simulith_uart_init(1, rx_callback);  // Should get 1
    ports[2] = simulith_uart_init(0, rx_callback);  // Should get MAX_UART_PORTS
    ports[3] = simulith_uart_init(1, rx_callback);  // Should get MAX_UART_PORTS + 1

    // Test data for each pair
    uint8_t test_data1[] = {0x11, 0x22};
    uint8_t test_data2[] = {0x33, 0x44};

    // Test first pair
    simulith_uart_send(ports[0], test_data1, sizeof(test_data1));
    TEST_ASSERT_EQUAL(ports[1], rx_port);
    TEST_ASSERT_EQUAL_MEMORY(test_data1, rx_data, sizeof(test_data1));

    // Test second pair
    simulith_uart_send(ports[2], test_data2, sizeof(test_data2));
    TEST_ASSERT_EQUAL(ports[3], rx_port);
    TEST_ASSERT_EQUAL_MEMORY(test_data2, rx_data, sizeof(test_data2));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_uart_init_basic);
    RUN_TEST(test_uart_init_auto_assignment);
    RUN_TEST(test_uart_send_receive_basic);
    RUN_TEST(test_uart_send_receive_extended_ports);
    RUN_TEST(test_uart_send_receive_multiple_pairs);
    return UNITY_END();
}