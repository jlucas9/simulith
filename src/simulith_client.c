#include "simulith.h"

static void *client_context = NULL;
static void *client_subscriber = NULL;
static uint64_t client_current_time_ns = 0;

int simulith_client_init(const char *connect_address) {
    client_context = zmq_ctx_new();
    if (!client_context) {
        perror("zmq_ctx_new failed");
        return -1;
    }

    client_subscriber = zmq_socket(client_context, ZMQ_SUB);
    if (!client_subscriber) {
        perror("zmq_socket failed");
        return -1;
    }

    if (zmq_connect(client_subscriber, connect_address) != 0) {
        perror("zmq_connect failed");
        return -1;
    }

    if (zmq_setsockopt(client_subscriber, ZMQ_SUBSCRIBE, "", 0) != 0) {
        perror("zmq_setsockopt failed");
        return -1;
    }

    simulith_log("Simulith client connected to %s\n", connect_address);
    return 0;
}

int simulith_client_wait_for_time(uint64_t expected_time_ns) {
    while (1) {
        uint64_t received_time_ns = 0;
        int size = zmq_recv(client_subscriber, &received_time_ns, sizeof(received_time_ns), 0);
        if (size != sizeof(received_time_ns)) {
            perror("zmq_recv failed or size mismatch");
            continue;
        }

        client_current_time_ns = received_time_ns;
        simulith_log("Received time update: %lu ns\n", client_current_time_ns);

        if (client_current_time_ns >= expected_time_ns) break;
    }
    return 0;
}

uint64_t simulith_client_get_time(void) {
    return client_current_time_ns;
}

void simulith_client_shutdown(void) {
    if (client_subscriber) zmq_close(client_subscriber);
    if (client_context) zmq_ctx_term(client_context);
    client_subscriber = NULL;
    client_context = NULL;
    simulith_log("Simulith client shut down\n");
}
