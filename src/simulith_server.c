#include "simulith.h"

static void *server_context = NULL;
static void *server_publisher = NULL;
static uint64_t server_current_time_ns = 0;

int simulith_server_init(const char *bind_address) {
    server_context = zmq_ctx_new();
    if (!server_context) {
        perror("zmq_ctx_new failed");
        return -1;
    }

    server_publisher = zmq_socket(server_context, ZMQ_PUB);
    if (!server_publisher) {
        perror("zmq_socket failed");
        return -1;
    }

    if (zmq_bind(server_publisher, bind_address) != 0) {
        perror("zmq_bind failed");
        return -1;
    }

    simulith_log("Simulith server bound to %s\n", bind_address);
    return 0;
}

int simulith_server_publish_time(uint64_t time_ns) {
    server_current_time_ns = time_ns;
    int rc = zmq_send(server_publisher, &server_current_time_ns, sizeof(server_current_time_ns), 0);
    if (rc != sizeof(server_current_time_ns)) {
        perror("zmq_send failed");
        return -1;
    }
    simulith_log("Published time update: %lu ns\n", server_current_time_ns);
    return 0;
}

uint64_t simulith_server_get_time(void) {
    return server_current_time_ns;
}

void simulith_server_shutdown(void) {
    if (server_publisher) zmq_close(server_publisher);
    if (server_context) zmq_ctx_term(server_context);
    server_publisher = NULL;
    server_context = NULL;
    simulith_log("Simulith server shut down\n");
}
