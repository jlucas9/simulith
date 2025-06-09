#include "simulith_uart.h"
#include "simulith.h"
#include <string.h>

typedef struct
{
    bool                      initialized;
    simulith_uart_rx_callback rx_callback;
    uint8_t                   rx_buffer[RX_BUFFER_SIZE];
    size_t                    rx_buffer_head;
    size_t                    rx_buffer_tail;
} uart_port_t;

// Array size is doubled to allow for MAX_UART_PORTS pairs
static uart_port_t uart_ports[MAX_UART_PORTS * 2] = {0};

int simulith_uart_init(uint8_t port_id, simulith_uart_rx_callback rx_cb)
{
    if (port_id >= MAX_UART_PORTS)
    {
        simulith_log("Invalid UART port ID: %d\n", port_id);
        return -1;
    }

    // Check if this port number is already connected
    uint8_t actual_port = port_id;
    uint8_t pair_port = (port_id % 2 == 0) ? port_id + 1 : port_id - 1;

    if (uart_ports[port_id].initialized && uart_ports[pair_port].initialized)
    {
        // For extended ports, we want:
        // port 0 -> 16 (MAX_UART_PORTS + 0)
        // port 1 -> 17 (MAX_UART_PORTS + 1)
        actual_port = MAX_UART_PORTS + port_id;

        // If these ports are also taken, find next available pair
        if (uart_ports[actual_port].initialized)
        {
            simulith_log("No available UART ports\n");
            return -1;
        }
    }

    uart_port_t *port = &uart_ports[actual_port];

    if (port->initialized)
    {
        simulith_log("UART port %d already initialized\n", actual_port);
        return -1;
    }

    // Initialize port structure
    port->rx_callback = rx_cb;
    port->rx_buffer_head = 0;
    port->rx_buffer_tail = 0;
    port->initialized = true;

    simulith_log("UART port %d initialized (requested %d)\n", actual_port, port_id);

    return actual_port;  // Return the actual port number assigned
}

int simulith_uart_send(uint8_t port_id, const uint8_t *data, size_t len)
{
    if (port_id >= MAX_UART_PORTS * 2 || !uart_ports[port_id].initialized)
    {
        return -1;
    }

    if (!data || len == 0)
    {
        return -1;
    }

    // Log the data being sent
    simulith_log("UART%d TX: ", port_id);
    for (size_t i = 0; i < len; i++)
    {
        simulith_log("%02X ", data[i]);
    }
    simulith_log("\n");

    // Send to the paired port (port pairs are 0-1, 2-3, 4-5, etc.)
    uint8_t target_port = (port_id % 2 == 0) ? port_id + 1 : port_id - 1;
    if (target_port < MAX_UART_PORTS * 2 && uart_ports[target_port].initialized && uart_ports[target_port].rx_callback)
    {
        uart_ports[target_port].rx_callback(target_port, data, len);
    }

    return len;
}

int simulith_uart_receive(uint8_t port_id, uint8_t *data, size_t max_len)
{
    if (port_id >= MAX_UART_PORTS * 2 || !uart_ports[port_id].initialized || !data || max_len == 0)
    {
        return -1;
    }

    uart_port_t *port = &uart_ports[port_id];
    
    // Calculate bytes available in the circular buffer
    size_t bytes_available;
    if (port->rx_buffer_head >= port->rx_buffer_tail)
    {
        bytes_available = port->rx_buffer_head - port->rx_buffer_tail;
    }
    else
    {
        bytes_available = RX_BUFFER_SIZE - port->rx_buffer_tail + port->rx_buffer_head;
    }
    
    // Limit bytes to read
    size_t bytes_to_read = (bytes_available < max_len) ? bytes_available : max_len;
    
    // Read data from the circular buffer
    for (size_t i = 0; i < bytes_to_read; i++)
    {
        data[i] = port->rx_buffer[port->rx_buffer_tail];
        port->rx_buffer_tail = (port->rx_buffer_tail + 1) % RX_BUFFER_SIZE;
    }

    // Log received data
    if (bytes_to_read > 0)
    {
        simulith_log("UART%d RX: ", port_id);
        for (size_t i = 0; i < bytes_to_read; i++)
        {
            simulith_log("%02X ", data[i]);
        }
        simulith_log("\n");
    }

    return bytes_to_read;
}

int simulith_uart_available(uint8_t port_id)
{
    if (port_id >= MAX_UART_PORTS * 2 || !uart_ports[port_id].initialized)
    {
        return -1;
    }

    uart_port_t *port = &uart_ports[port_id];
    
    // Calculate bytes available in the circular buffer
    if (port->rx_buffer_head >= port->rx_buffer_tail)
    {
        return port->rx_buffer_head - port->rx_buffer_tail;
    }
    else
    {
        return RX_BUFFER_SIZE - port->rx_buffer_tail + port->rx_buffer_head;
    }
}

int simulith_uart_close(uint8_t port_id)
{
    if (port_id >= MAX_UART_PORTS * 2 || !uart_ports[port_id].initialized)
    {
        return -1;
    }

    uart_ports[port_id].initialized = false;
    simulith_log("UART port %d closed\n", port_id);
    return 0;
}