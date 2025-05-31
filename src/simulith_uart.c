#include "simulith_uart.h"
#include "simulith.h"
#include <string.h>

#define MAX_UART_PORTS 8
#define RX_BUFFER_SIZE 1024

typedef struct {
    bool initialized;
    simulith_uart_config_t config;
    simulith_uart_rx_callback rx_callback;
    uint8_t rx_buffer[RX_BUFFER_SIZE];
    size_t rx_buffer_head;
    size_t rx_buffer_tail;
} uart_port_t;

static uart_port_t uart_ports[MAX_UART_PORTS] = {0};

static bool is_valid_config(const simulith_uart_config_t *config) {
    if (!config) return false;
    
    // Validate baud rate (common values)
    switch (config->baud_rate) {
        case 9600:
        case 19200:
        case 38400:
        case 57600:
        case 115200:
            break;
        default:
            return false;
    }
    
    // Validate data bits (5-8)
    if (config->data_bits < 5 || config->data_bits > 8) return false;
    
    // Validate stop bits (1-2)
    if (config->stop_bits < 1 || config->stop_bits > 2) return false;
    
    // Validate parity
    if (config->parity > SIMULITH_UART_PARITY_EVEN) return false;
    
    // Validate flow control
    if (config->flow_control > SIMULITH_UART_FLOW_SOFTWARE) return false;
    
    return true;
}

int simulith_uart_init(uint8_t port_id, const simulith_uart_config_t *config, simulith_uart_rx_callback rx_cb) {
    if (port_id >= MAX_UART_PORTS) {
        simulith_log("Invalid UART port ID: %d\n", port_id);
        return -1;
    }
    
    if (!is_valid_config(config)) {
        simulith_log("Invalid UART configuration for port %d\n", port_id);
        return -1;
    }
    
    uart_port_t *port = &uart_ports[port_id];
    
    if (port->initialized) {
        simulith_log("UART port %d already initialized\n", port_id);
        return -1;
    }
    
    // Initialize port structure
    memcpy(&port->config, config, sizeof(simulith_uart_config_t));
    port->rx_callback = rx_cb;
    port->rx_buffer_head = 0;
    port->rx_buffer_tail = 0;
    port->initialized = true;
    
    simulith_log("UART port %d initialized: %d baud, %d-%d-%c\n",
                 port_id,
                 config->baud_rate,
                 config->data_bits,
                 config->stop_bits,
                 config->parity == 0 ? 'N' : (config->parity == 1 ? 'O' : 'E'));
    
    return 0;
}

int simulith_uart_send(uint8_t port_id, const uint8_t *data, size_t len) {
    if (port_id >= MAX_UART_PORTS || !uart_ports[port_id].initialized) {
        return -1;
    }
    
    if (!data || len == 0) {
        return -1;
    }
    
    uart_port_t *port = &uart_ports[port_id];
    
    // In a real implementation, we would handle flow control here
    // For simulation, we just log the data
    simulith_log("UART%d TX: ", port_id);
    for (size_t i = 0; i < len; i++) {
        simulith_log("%02X ", data[i]);
    }
    simulith_log("\n");
    
    // If there's a receive callback, simulate loopback
    if (port->rx_callback) {
        return port->rx_callback(port_id, data, len);
    }
    
    return len;
}

int simulith_uart_receive(uint8_t port_id, uint8_t *data, size_t max_len) {
    if (port_id >= MAX_UART_PORTS || !uart_ports[port_id].initialized || !data || max_len == 0) {
        return -1;
    }
    
    uart_port_t *port = &uart_ports[port_id];
    size_t bytes_available = (port->rx_buffer_head - port->rx_buffer_tail) % RX_BUFFER_SIZE;
    size_t bytes_to_read = (bytes_available < max_len) ? bytes_available : max_len;
    
    for (size_t i = 0; i < bytes_to_read; i++) {
        data[i] = port->rx_buffer[port->rx_buffer_tail];
        port->rx_buffer_tail = (port->rx_buffer_tail + 1) % RX_BUFFER_SIZE;
    }
    
    return bytes_to_read;
}

int simulith_uart_available(uint8_t port_id) {
    if (port_id >= MAX_UART_PORTS || !uart_ports[port_id].initialized) {
        return -1;
    }
    
    uart_port_t *port = &uart_ports[port_id];
    return (port->rx_buffer_head - port->rx_buffer_tail) % RX_BUFFER_SIZE;
}

int simulith_uart_close(uint8_t port_id) {
    if (port_id >= MAX_UART_PORTS || !uart_ports[port_id].initialized) {
        return -1;
    }
    
    uart_ports[port_id].initialized = false;
    simulith_log("UART port %d closed\n", port_id);
    return 0;
} 