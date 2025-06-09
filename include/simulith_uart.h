#ifndef SIMULITH_UART_H
#define SIMULITH_UART_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_UART_PORTS 16
#define RX_BUFFER_SIZE 4096

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Callback function type for UART receive operations
     * @param port_id Port identifier
     * @param data Pointer to received data
     * @param len Number of bytes received
     * @return Number of bytes processed, -1 on error
     */
    typedef int (*simulith_uart_rx_callback)(uint8_t port_id, const uint8_t *data, size_t len);

    /**
     * @brief Initialize a UART port
     * @param port_id Port identifier (0-7)
     * @param config UART configuration structure
     * @param rx_cb Callback function for receive operations (NULL if not used)
     * @return 0 on success, -1 on failure
     * @note Each port automatically connects to the port with the same ID in other devices
     */
    int simulith_uart_init(uint8_t port_id, simulith_uart_rx_callback rx_cb);

    /**
     * @brief Send data over UART
     * @param port_id Port identifier
     * @param data Data to send
     * @param len Number of bytes to send
     * @return Number of bytes sent, -1 on failure
     */
    int simulith_uart_send(uint8_t port_id, const uint8_t *data, size_t len);

    /**
     * @brief Receive data from UART (non-blocking)
     * @param port_id Port identifier
     * @param data Buffer to store received data
     * @param max_len Maximum number of bytes to receive
     * @return Number of bytes received, -1 on failure
     */
    int simulith_uart_receive(uint8_t port_id, uint8_t *data, size_t max_len);

    /**
     * @brief Check if UART port has data available
     * @param port_id Port identifier
     * @return Number of bytes available, -1 on failure
     */
    int simulith_uart_available(uint8_t port_id);

    /**
     * @brief Close a UART port
     * @param port_id Port identifier
     * @return 0 on success, -1 on failure
     */
    int simulith_uart_close(uint8_t port_id);

#ifdef __cplusplus
}
#endif

#endif /* SIMULITH_UART_H */