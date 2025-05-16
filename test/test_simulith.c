#include "simulith.h"
#include "unity.h"
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define PUB_ADDR "ipc:///tmp/simulith_pub.ipc"
#define REP_ADDR "ipc:///tmp/simulith_rep.ipc"

#define CLIENT_ID "test_client"
#define INTERVAL_NS 10 * 1000000  // 10 ms
#define TEST_TIME_S 10 // seconds

static int ticks_received = 0;

void setUp (void) {
    // Remove IPC socket files before each test to avoid conflicts
    unlink(PUB_ADDR);
    unlink(REP_ADDR);
}
void tearDown (void) {
    // Clean up IPC socket files after test
    unlink(PUB_ADDR);
    unlink(REP_ADDR);
}

void on_tick(uint64_t time_ns) {
    //simulith_log("Test on_tick: time_ns = %lu\n", time_ns);
    ticks_received++;
}

void *server_thread(void *arg) {
    simulith_server_init(PUB_ADDR, REP_ADDR, 1, INTERVAL_NS);
    simulith_server_run();  // runs indefinitely
    return NULL;
}

void *client_thread(void *arg) {
    sleep(1);  // Wait for server to be ready

    simulith_client_init(PUB_ADDR, REP_ADDR, CLIENT_ID, INTERVAL_NS);

    // Perform handshake before running the tick loop
    if (simulith_client_handshake() != 0) {
        fprintf(stderr, "Client handshake failed\n");
        return NULL;
    }

    simulith_client_run_loop(on_tick);  // runs indefinitely
    return NULL;
}

void test_synchronization_tick_exchange(void) {
    pthread_t server, client;

    pthread_create(&server, NULL, server_thread, NULL);
    pthread_create(&client, NULL, client_thread, NULL);

    sleep(TEST_TIME_S);  // Allow some time for a few ticks to exchange

    TEST_ASSERT_GREATER_THAN(0, ticks_received);

    // Cancel threads to end test
    pthread_cancel(client);
    pthread_cancel(server);
    pthread_join(client, NULL);
    pthread_join(server, NULL);

    simulith_log("Ticks received during test: %d\n", ticks_received);

    double simulated_time_seconds = ((double)ticks_received * (double)INTERVAL_NS) / 1e9;
    double interval_ms = (double)INTERVAL_NS / 1e6;
    simulith_log("Test ran for %d seconds real time, simulating %.3f seconds via %lu ticks with an interval of %.2f ms\n", TEST_TIME_S, simulated_time_seconds, (unsigned long)ticks_received, interval_ms);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_synchronization_tick_exchange);
    return UNITY_END();
}
