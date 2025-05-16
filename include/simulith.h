#ifndef SIMULITH_H
#define SIMULITH_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <zmq.h>

#ifdef __cplusplus
extern "C" {
#endif

// Simulith version
#define SIMULITH_VERSION "0.0.0"

// Logging function shared by client and server
void simulith_log(const char *fmt, ...);

// Client API
int simulith_client_init(const char *connect_address);
int simulith_client_wait_for_time(uint64_t expected_time_ns);
uint64_t simulith_client_get_time(void);
void simulith_client_shutdown(void);

// Server API
int simulith_server_init(const char *bind_address);
int simulith_server_publish_time(uint64_t time_ns);
uint64_t simulith_server_get_time(void);
void simulith_server_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif // SIMULITH_H
