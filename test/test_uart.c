#include "simulith_uart.h"
#include "unity.h"
#include <string.h>

void setUp(void) {
    // Setup code if needed
}

void tearDown(void) {
    // Cleanup code if needed
}

static int rx_data_received = 0;
static uint8_t rx_buffer[256];

static int test_uart_rx_cb(uint8_t port_id, const uint8_t *data, size_t len) {
    if (len <= sizeof(rx_buffer)) {
        memcpy(rx_buffer, data, len);
        rx_data_received = len;
        return len;
    }
    return -1;
}

void test_uart_init(void) {
    simulith_uart_config_t config = {
        .baud_rate = 115200,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = SIMULITH_UART_PARITY_NONE,
        .flow_control = SIMULITH_UART_FLOW_NONE
    };
    
    // Test invalid port ID
    TEST_ASSERT_EQUAL_INT(-1, simulith_uart_init(8, &config, test_uart_rx_cb));
    
    // Test NULL config
    TEST_ASSERT_EQUAL_INT(-1, simulith_uart_init(0, NULL, test_uart_rx_cb));
    
    // Test invalid baud rate
    config.baud_rate = 1234;
    TEST_ASSERT_EQUAL_INT(-1, simulith_uart_init(0, &config, test_uart_rx_cb));
    
    // Test invalid data bits
    config.baud_rate = 115200;
    config.data_bits = 9;
    TEST_ASSERT_EQUAL_INT(-1, simulith_uart_init(0, &config, test_uart_rx_cb));
    
    // Test successful initialization
    config.data_bits = 8;
    TEST_ASSERT_EQUAL_INT(0, simulith_uart_init(0, &config, test_uart_rx_cb));
    
    // Test duplicate initialization
    TEST_ASSERT_EQUAL_INT(-1, simulith_uart_init(0, &config, test_uart_rx_cb));
    
    // Clean up
    simulith_uart_close(0);
}

void test_uart_send_receive(void) {
    simulith_uart_config_t config = {
        .baud_rate = 115200,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = SIMULITH_UART_PARITY_NONE,
        .flow_control = SIMULITH_UART_FLOW_NONE
    };
    
    // Initialize UART
    TEST_ASSERT_EQUAL_INT(0, simulith_uart_init(0, &config, test_uart_rx_cb));
    
    // Test send operation
    const uint8_t test_data[] = {0x12, 0x34, 0x56, 0x78};
    TEST_ASSERT_EQUAL_INT(sizeof(test_data), simulith_uart_send(0, test_data, sizeof(test_data)));
    
    // Verify received data through callback
    TEST_ASSERT_EQUAL_INT(sizeof(test_data), rx_data_received);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(test_data, rx_buffer, sizeof(test_data));
    
    // Clean up
    simulith_uart_close(0);
}

void test_uart_invalid_operations(void) {
    simulith_uart_config_t config = {
        .baud_rate = 115200,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = SIMULITH_UART_PARITY_NONE,
        .flow_control = SIMULITH_UART_FLOW_NONE
    };
    
    uint8_t data[16];
    
    // Test operations on uninitialized port
    TEST_ASSERT_EQUAL_INT(-1, simulith_uart_send(0, data, sizeof(data)));
    TEST_ASSERT_EQUAL_INT(-1, simulith_uart_receive(0, data, sizeof(data)));
    TEST_ASSERT_EQUAL_INT(-1, simulith_uart_available(0));
    TEST_ASSERT_EQUAL_INT(-1, simulith_uart_close(0));
    
    // Initialize port
    TEST_ASSERT_EQUAL_INT(0, simulith_uart_init(0, &config, test_uart_rx_cb));
    
    // Test invalid parameters
    TEST_ASSERT_EQUAL_INT(-1, simulith_uart_send(0, NULL, 16));
    TEST_ASSERT_EQUAL_INT(-1, simulith_uart_send(0, data, 0));
    TEST_ASSERT_EQUAL_INT(-1, simulith_uart_receive(0, NULL, 16));
    TEST_ASSERT_EQUAL_INT(-1, simulith_uart_receive(0, data, 0));
    
    // Clean up
    simulith_uart_close(0);
}

void test_uart_multiple_ports(void) {
    simulith_uart_config_t config = {
        .baud_rate = 115200,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = SIMULITH_UART_PARITY_NONE,
        .flow_control = SIMULITH_UART_FLOW_NONE
    };
    
    // Initialize multiple ports
    for (uint8_t i = 0; i < 3; i++) {
        TEST_ASSERT_EQUAL_INT(0, simulith_uart_init(i, &config, test_uart_rx_cb));
    }
    
    // Test sending data on each port
    const uint8_t test_data[] = {0xAA, 0xBB};
    for (uint8_t i = 0; i < 3; i++) {
        TEST_ASSERT_EQUAL_INT(sizeof(test_data), simulith_uart_send(i, test_data, sizeof(test_data)));
    }
    
    // Clean up
    for (uint8_t i = 0; i < 3; i++) {
        simulith_uart_close(i);
    }
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_uart_init);
    RUN_TEST(test_uart_send_receive);
    RUN_TEST(test_uart_invalid_operations);
    RUN_TEST(test_uart_multiple_ports);
    
    return UNITY_END();
} 