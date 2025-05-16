#include "simulith.h"
#include "unity.h"
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define PUB_ADDR "tcp://127.0.0.1:5557"
#define REP_ADDR "tcp://127.0.0.1:5558"
#define CLIENT_ID "test_client"
#define INTERVAL_NS 100000000  // 100 ms

static int ticks_received = 0;

void setUp (void) {} /* Is run before every test, put unit init calls here. */
void tearDown (void) {} /* Is run after every test, put unit clean-up calls here. */

void on_tick(uint64_t time_ns) {
    simulith_log("Test on_tick: time_ns = %lu\n", time_ns);
    ticks_received++;
}

void *server_thread(void *arg) {
    simulith_log("[TEST] Starting server...\n");
    simulith_server_init(PUB_ADDR, REP_ADDR, 1, INTERVAL_NS);
    simulith_server_run();
    return NULL;
}

void *client_thread(void *arg) {
    sleep(1);  // Give server a moment to bind
    simulith_log("[TEST] Starting client...\n");
    simulith_client_init(PUB_ADDR, REP_ADDR, CLIENT_ID, INTERVAL_NS);
    simulith_client_run_loop(on_tick);
    return NULL;
}

void test_synchronization_tick_exchange(void) {
    pthread_t server, client;

    pthread_create(&server, NULL, server_thread, NULL);
    pthread_create(&client, NULL, client_thread, NULL);

    sleep(12);  // Allow some time for a few ticks to exchange

    simulith_log("[TEST] ticks_received = %d \n", ticks_received);
    TEST_ASSERT_GREATER_THAN(0, ticks_received);

    // Cancel threads to end test
    pthread_cancel(client);
    pthread_cancel(server);
    pthread_join(client, NULL);
    pthread_join(server, NULL);

    simulith_log("Ticks received during test: %d\n", ticks_received);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_synchronization_tick_exchange);
    return UNITY_END();
}
