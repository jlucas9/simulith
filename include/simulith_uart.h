#ifndef SIMULITH_UART_H
#define SIMULITH_UART_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SIMULITH_UART_SUCCESS 0
#define SIMULITH_UART_ERROR  -1

#define SIMULITH_MAX_UART_PORTS 16
#define SIMULITH_UART_BUFFER_SIZE 4096

#define SIMULITH_UART_INITIALIZED 255

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Initialize a UART port
     * @param port_id Port identifier
     * @param handle Handle identifier
     * @return SIMULITH_UART_SUCCESS on success, SIMULITH_UART_ERROR on failure
     * @note Connects to port < SIMULITH_MAX_UART_PORTS, then port + SIMULITH_MAX_UART_PORTS
     */
    int simulith_uart_init(uint8_t port_id, int32_t* handle);

    /**
     * @brief Send data over UART
     * @param handle Identifier
     * @param data Data to send
     * @param len Number of bytes to send
     * @return Number of bytes sent, SIMULITH_UART_ERROR on failure
     */
    int simulith_uart_send(uint32_t handle, const uint8_t *data, size_t len);

    /**
     * @brief Receive data from UART (non-blocking)
     * @param handle Identifier
     * @param data Buffer to store received data
     * @param max_len Maximum number of bytes to receive
     * @return Number of bytes received, SIMULITH_UART_ERROR on failure
     */
    int simulith_uart_receive(uint32_t handle, uint8_t *data, size_t max_len);

    
    /**
     * @brief Check if UART port has data available
     * @param handle Identifier
     * @return Number of bytes available, SIMULITH_UART_ERROR on failure
     */
    int simulith_uart_available(uint32_t handle);

    /**
     * @brief Flush data to be received on UART port
     * @param handle Identifier
     * @return SIMULITH_UART_SUCCESS on success, SIMULITH_UART_ERROR on failure
     */
    int simulith_uart_flush(uint32_t handle);

    /**
     * @brief Close a UART port
     * @param handle Identifier
     * @return SIMULITH_UART_SUCCESS on success, SIMULITH_UART_ERROR on failure
     */
    int simulith_uart_close(uint32_t handle);

#ifdef __cplusplus
}
#endif

#endif /* SIMULITH_UART_H */