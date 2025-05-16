#include "simulith.h"

#define MAX_CLIENTS 32

typedef struct {
    char id[64];
    int responded;
} ClientState;

static void *server_context = NULL;
static void *publisher = NULL;
static void *responder = NULL;
static uint64_t current_time_ns = 0;
static uint64_t tick_interval_ns = 0;
static int expected_clients = 0;
static ClientState client_states[MAX_CLIENTS];

int simulith_server_init(const char *pub_bind, const char *rep_bind, int client_count, uint64_t interval_ns) {
    expected_clients = client_count;
    tick_interval_ns = interval_ns;

    server_context = zmq_ctx_new();
    if (!server_context) {
        perror("zmq_ctx_new failed");
        return -1;
    }

    publisher = zmq_socket(server_context, ZMQ_PUB);
    if (!publisher || zmq_bind(publisher, pub_bind) != 0) {
        perror("Publisher socket setup failed");
        return -1;
    }

    responder = zmq_socket(server_context, ZMQ_REP);
    if (!responder || zmq_bind(responder, rep_bind) != 0) {
        perror("Responder socket setup failed");
        return -1;
    }

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        client_states[i].id[0] = '\0';
        client_states[i].responded = 0;
    }

    simulith_log("Simulith server initialized. Clients expected: %d\n", expected_clients);
    return 0;
}

static void broadcast_time() {
    zmq_send(publisher, &current_time_ns, sizeof(current_time_ns), 0);
    simulith_log("Broadcasted time: %lu ns\n", current_time_ns);
}

static int all_clients_responded() {
    int count = 0;
    for (int i = 0; i < expected_clients; ++i) {
        if (client_states[i].responded) count++;
    }
    return count == expected_clients;
}

static void reset_responses() {
    for (int i = 0; i < expected_clients; ++i) {
        client_states[i].responded = 0;
    }
}

static void handle_ack(const char *client_id) {
    for (int i = 0; i < expected_clients; ++i) {
        if (strcmp(client_states[i].id, client_id) == 0) {
            client_states[i].responded = 1;
            simulith_log("ACK matched existing client: %s\n", client_id);
            return;
        }
    }
    for (int i = 0; i < expected_clients; ++i) {
        if (client_states[i].id[0] == '\0') {
            strncpy(client_states[i].id, client_id, sizeof(client_states[i].id) - 1);
            client_states[i].responded = 1;
            simulith_log("Registered and acknowledged new client: %s\n", client_id);
            return;
        }
    }
    simulith_log("ACK received from unknown client (all slots full): %s\n", client_id);
}

void simulith_server_run(void) {
    while (1) {
        broadcast_time();
        reset_responses();

        while (!all_clients_responded()) {
            char buffer[64] = {0};
            int size = zmq_recv(responder, buffer, sizeof(buffer) - 1, 0);
            if (size > 0) {
                buffer[size] = '\0';
                handle_ack(buffer);
                zmq_send(responder, "ACK", 3, 0);
                simulith_log("Received ACK from client: %s\n", buffer);
            }
        }

        current_time_ns += tick_interval_ns;
        usleep(1000);  // Small delay to avoid CPU hog
    }
}

void simulith_server_shutdown(void) {
    if (publisher) zmq_close(publisher);
    if (responder) zmq_close(responder);
    if (server_context) zmq_ctx_term(server_context);
    publisher = NULL;
    responder = NULL;
    server_context = NULL;
    simulith_log("Simulith server shut down\n");
}
