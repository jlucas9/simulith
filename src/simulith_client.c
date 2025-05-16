#include "simulith.h"

static void *client_context = NULL;
static void *subscriber = NULL;
static void *requester = NULL;
static char client_id[64];
static uint64_t update_rate_ns = 0;

int simulith_client_init(const char *pub_addr, const char *rep_addr, const char *id, uint64_t rate_ns) {
    strncpy(client_id, id, sizeof(client_id) - 1);
    update_rate_ns = rate_ns;

    client_context = zmq_ctx_new();
    if (!client_context) {
        perror("zmq_ctx_new failed");
        return -1;
    }

    subscriber = zmq_socket(client_context, ZMQ_SUB);
    if (!subscriber || zmq_connect(subscriber, pub_addr) != 0) {
        perror("Subscriber socket setup failed");
        return -1;
    }
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0);  // Subscribe to all messages

    requester = zmq_socket(client_context, ZMQ_REQ);
    if (!requester || zmq_connect(requester, rep_addr) != 0) {
        perror("Requester socket setup failed");
        return -1;
    }

    simulith_log("Simulith client [%s] initialized with update rate %lu ns\n", client_id, update_rate_ns);
    return 0;
}

void simulith_client_run_loop(simulith_tick_callback on_tick) {
    while (1) {
        uint64_t time_ns;
        int recv_bytes = zmq_recv(subscriber, &time_ns, sizeof(time_ns), 0);
        if (recv_bytes == sizeof(time_ns)) {
            if (on_tick) {
                on_tick(time_ns);
            }

            // Send acknowledgment
            char reply[16] = {0};
            zmq_send(requester, client_id, strlen(client_id), 0);
            zmq_recv(requester, reply, sizeof(reply) - 1, 0);  // wait for server ACK

            simulith_log("Client [%s] sent ACK and received response: %s\n", client_id, reply);
        }
    }
}

void simulith_client_shutdown(void) {
    if (subscriber) zmq_close(subscriber);
    if (requester) zmq_close(requester);
    if (client_context) zmq_ctx_term(client_context);
    subscriber = NULL;
    requester = NULL;
    client_context = NULL;
    simulith_log("Simulith client [%s] shut down\n", client_id);
}
