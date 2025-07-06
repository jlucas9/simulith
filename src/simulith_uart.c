#include "simulith_uart.h"
#include "simulith.h"

typedef struct
{
    uint8_t                   init;
    uint8_t                   buffer[SIMULITH_UART_BUFFER_SIZE];
    uint16_t                  buffer_len;
} uart_port_t;

// Array size is doubled to allow for MAX_UART_PORTS pairs
static uart_port_t uart_ports[SIMULITH_MAX_UART_PORTS * 2] = {0};

int simulith_uart_init(uint8_t port_id, int32_t* handle)
{
    uint8_t pair_port = port_id + SIMULITH_MAX_UART_PORTS;

    // Confirm received port ID valid
    if (port_id >= SIMULITH_MAX_UART_PORTS)
    {
        simulith_log("simulith_uart_init: Invalid UART port ID: %d\n", port_id);
        return -1;
    }

    // Check if already connected
    if (uart_ports[port_id].init != SIMULITH_UART_INITIALIZED)
    {
        *handle = port_id;
        simulith_log("simulith_uart_init: First connection to UART %d. \n", *handle);
    }
    else if(uart_ports[pair_port].init != SIMULITH_UART_INITIALIZED)
    {
        *handle = pair_port;
        simulith_log("simulith_uart_init: Second connection to UART %d. \n", *handle);
    }
    else
    {
        simulith_log("simulith_uart_init: UART initialization failed! Both UART connections of %d already initialized!\n", port_id);
        return SIMULITH_UART_ERROR;
    }

    // Register UART
    uart_ports[*handle].init = SIMULITH_UART_INITIALIZED;
    memset(uart_ports[*handle].buffer, 0, sizeof(uart_ports[*handle].buffer));
    uart_ports[*handle].buffer_len = 0;

    // Return success
    simulith_log("simulith_uart_init: UART handle %d initialized (requested %d)\n", *handle, port_id);
    return SIMULITH_UART_SUCCESS;
}

int simulith_uart_send(uint32_t handle, const uint8_t *data, size_t len)
{
    uint8_t port = 0;
    uint16_t send_size = len;

    // Confirm port valid
    if (handle >= (SIMULITH_MAX_UART_PORTS * 2))
    {
        simulith_log("simulith_uart_send: Invalid UART handle %d\n", handle);
        return SIMULITH_UART_ERROR;
    }

    // Confirm sending side initialized
    if (uart_ports[handle].init != SIMULITH_UART_INITIALIZED)
    {
        simulith_log("simulith_uart_send: Uninitialized UART handle %d\n", handle);
        return SIMULITH_UART_ERROR;
    }

    // Determine destination port
    if (handle >= SIMULITH_MAX_UART_PORTS)
    {
        port = handle - SIMULITH_MAX_UART_PORTS;
    }
    else
    {
        port = handle + SIMULITH_MAX_UART_PORTS;
    }

    // Confirm receiving side initialized
    if (uart_ports[port].init != SIMULITH_UART_INITIALIZED)
    {
        simulith_log("simulith_uart_send: Receiving side %d not initialized\n", port);
        return SIMULITH_UART_ERROR;
    }

    // Check if UART would overflow and truncate if so
    if ((uart_ports[port].buffer_len + send_size) > SIMULITH_UART_BUFFER_SIZE)
    {
        send_size = SIMULITH_UART_BUFFER_SIZE - uart_ports[port].buffer_len;
        simulith_log("simulith_uart_send: UART %d would overflow, sending %d / %d\n", port, send_size, len);
    }

    // Set data in receiving side
    memcpy(&(uart_ports[port].buffer[uart_ports[port].buffer_len]), data, len);
    uart_ports[port].buffer_len += len;

    // Log the data being sent
    simulith_log("UART TX[%d]: ", port);
    for (size_t i = 0; i < send_size; i++)
    {
        simulith_log("%02X ", data[i]);
    }
    simulith_log("\n");

    // Return the length of data sent
    return send_size;
}

int simulith_uart_receive(uint32_t handle, uint8_t *data, size_t max_len)
{
    uint16_t receive_size = 0;

    // Confirm port valid
    if (handle >= (SIMULITH_MAX_UART_PORTS * 2))
    {
        simulith_log("simulith_uart_send: Invalid UART handle %d\n", handle);
        return SIMULITH_UART_ERROR;
    }

    //simulith_log("simulith_uart_receive: uart_ports[%d].buffer_len = %d\n", handle, uart_ports[handle].buffer_len);

    // Determine length of receipt data
    if (max_len >= (uart_ports[handle].buffer_len))
    {
        // Move data
        receive_size = uart_ports[handle].buffer_len;
        memcpy(uart_ports[handle].buffer, data, receive_size);

        // Update length
        uart_ports[handle].buffer_len = 0;
    }
    else
    {
        // Move data
        receive_size = max_len;
        memcpy(uart_ports[handle].buffer, data, receive_size);
        
        // Shift remaining data forward
        memcpy(uart_ports[handle].buffer, &uart_ports[handle].buffer[receive_size], receive_size);
        uart_ports[handle].buffer_len = uart_ports[handle].buffer_len - max_len;
    }
    
    // Log received data
    if (receive_size > 0)
    {
        simulith_log("UART RX[%d]: ", handle);
        for (size_t i = 0; i < receive_size; i++)
        {
            simulith_log("%02X ", data[i]);
        }
        simulith_log("\n");
    }
    return receive_size;
}

int simulith_uart_available(uint32_t handle)
{
    // Confirm port valid
    if (handle >= (SIMULITH_MAX_UART_PORTS * 2))
    {
        simulith_log("simulith_uart_available: Invalid UART handle %d\n", handle);
        return SIMULITH_UART_ERROR;
    }

    // Confirm port initialized
    if (uart_ports[handle].init != SIMULITH_UART_INITIALIZED)
    {
        simulith_log("simulith_uart_available: Uninitialized UART handle %d\n", handle);
        return SIMULITH_UART_ERROR;
    }
    //simulith_log("simulith_uart_available: uart_ports[%d].buffer_len = %d\n", handle, uart_ports[handle].buffer_len);
    return uart_ports[handle].buffer_len;
}

int simulith_uart_flush(uint32_t handle)
{
    // Confirm port valid
    if (handle >= (SIMULITH_MAX_UART_PORTS * 2))
    {
        simulith_log("simulith_uart_available: Invalid UART handle %d\n", handle);
        return SIMULITH_UART_ERROR;
    }

    // Confirm port initialized
    if (uart_ports[handle].init != SIMULITH_UART_INITIALIZED)
    {
        simulith_log("simulith_uart_available: Uninitialized UART handle %d\n", handle);
        return SIMULITH_UART_ERROR;
    }

    // Set current buffer length to zero
    uart_ports[handle].buffer_len = 0;
    return SIMULITH_UART_SUCCESS;
}

int simulith_uart_close(uint32_t handle)
{
    // Confirm port valid
    if (handle >= (SIMULITH_MAX_UART_PORTS * 2))
    {
        //simulith_log("simulith_uart_close: Invalid UART handle %d\n", handle);
        return SIMULITH_UART_ERROR;
    }

    // Confirm port initialized
    if (uart_ports[handle].init != SIMULITH_UART_INITIALIZED)
    {
        //simulith_log("simulith_uart_close: Uninitialized UART handle %d\n", handle);
        return SIMULITH_UART_ERROR;
    }

    // Close out uart
    uart_ports[handle].init = 0;
    memset(uart_ports[handle].buffer, 0, sizeof(uart_ports[handle].buffer));
    uart_ports[handle].buffer_len = 0;
    simulith_log("UART port %d closed\n", handle);
    return SIMULITH_UART_SUCCESS;
}
