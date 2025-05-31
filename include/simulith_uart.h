#ifndef SIMULITH_UART_H
#define SIMULITH_UART_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief UART configuration structure
 */
typedef struct {
    uint32_t baud_rate;     /**< Baud rate (e.g., 9600, 115200) */
    uint8_t data_bits;      /**< Data bits (5-8) */
    uint8_t stop_bits;      /**< Stop bits (1-2) */
    uint8_t parity;         /**< 0=none, 1=odd, 2=even */
    uint8_t flow_control;   /**< 0=none, 1=hardware, 2=software */
} simulith_uart_config_t;

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
 */
int simulith_uart_init(uint8_t port_id, const simulith_uart_config_t *config, simulith_uart_rx_callback rx_cb);

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

// Constants for UART configuration
#define SIMULITH_UART_PARITY_NONE 0
#define SIMULITH_UART_PARITY_ODD  1
#define SIMULITH_UART_PARITY_EVEN 2

#define SIMULITH_UART_FLOW_NONE     0
#define SIMULITH_UART_FLOW_HARDWARE 1
#define SIMULITH_UART_FLOW_SOFTWARE 2

#ifdef __cplusplus
}
#endif

#endif /* SIMULITH_UART_H */ 