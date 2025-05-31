#include "simulith.h"
#include "unity.h"
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define PUB_ADDR "ipc:///tmp/simulith_pub.ipc"
#define REP_ADDR "ipc:///tmp/simulith_rep.ipc"
#define INVALID_ADDR "invalid://address"

#define CLIENT_ID "test_client"
#define INTERVAL_NS 10 * 1000000  // 10 ms
#define TEST_TIME_S 5 // seconds

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

// Test invalid server initialization
void test_server_init_invalid_address(void) {
    int result = simulith_server_init(INVALID_ADDR, REP_ADDR, 1, INTERVAL_NS);
    TEST_ASSERT_EQUAL_INT(-1, result);
    
    result = simulith_server_init(PUB_ADDR, INVALID_ADDR, 1, INTERVAL_NS);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_server_init_invalid_params(void) {
    int result = simulith_server_init(PUB_ADDR, REP_ADDR, 0, INTERVAL_NS);
    TEST_ASSERT_EQUAL_INT(-1, result);
    
    result = simulith_server_init(PUB_ADDR, REP_ADDR, -1, INTERVAL_NS);
    TEST_ASSERT_EQUAL_INT(-1, result);
    
    result = simulith_server_init(PUB_ADDR, REP_ADDR, 1, 0);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

// Test invalid client initialization
void test_client_init_invalid_address(void) {
    int result = simulith_client_init(INVALID_ADDR, REP_ADDR, CLIENT_ID, INTERVAL_NS);
    TEST_ASSERT_EQUAL_INT(-1, result);
    
    result = simulith_client_init(PUB_ADDR, INVALID_ADDR, CLIENT_ID, INTERVAL_NS);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_client_init_invalid_params(void) {
    int result = simulith_client_init(PUB_ADDR, REP_ADDR, NULL, INTERVAL_NS);
    TEST_ASSERT_EQUAL_INT(-1, result);
    
    result = simulith_client_init(PUB_ADDR, REP_ADDR, "", INTERVAL_NS);
    TEST_ASSERT_EQUAL_INT(-1, result);
    
    result = simulith_client_init(PUB_ADDR, REP_ADDR, CLIENT_ID, 0);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

// Test handshake without server
void test_client_handshake_no_server(void) {
    simulith_client_init(PUB_ADDR, REP_ADDR, CLIENT_ID, INTERVAL_NS);
    int result = simulith_client_handshake();
    TEST_ASSERT_EQUAL_INT(-1, result);
    simulith_client_shutdown();
}

// Test multiple clients with same ID
void *duplicate_client_thread(void *arg) {
    sleep(1);  // Wait for first client to connect
    
    int result = simulith_client_init(PUB_ADDR, REP_ADDR, CLIENT_ID, INTERVAL_NS);
    TEST_ASSERT_EQUAL_INT(0, result);
    
    result = simulith_client_handshake();
    TEST_ASSERT_EQUAL_INT(-1, result);  // Should fail due to duplicate ID
    
    simulith_client_shutdown();
    return NULL;
}

void test_duplicate_client_ids(void) {
    pthread_t server, client1, client2;
    
    pthread_create(&server, NULL, server_thread, NULL);
    pthread_create(&client1, NULL, client_thread, NULL);
    pthread_create(&client2, NULL, duplicate_client_thread, NULL);
    
    sleep(3);  // Allow time for test to complete
    
    pthread_cancel(client2);
    pthread_cancel(client1);
    pthread_cancel(server);
    pthread_join(client2, NULL);
    pthread_join(client1, NULL);
    pthread_join(server, NULL);
}

int main(void) {
    UNITY_BEGIN();
    
    // Original test
    RUN_TEST(test_synchronization_tick_exchange);
    
    // Server initialization tests
    RUN_TEST(test_server_init_invalid_address);
    RUN_TEST(test_server_init_invalid_params);
    
    // Client initialization tests
    RUN_TEST(test_client_init_invalid_address);
    RUN_TEST(test_client_init_invalid_params);
    
    // Connection tests
    RUN_TEST(test_client_handshake_no_server);
    RUN_TEST(test_duplicate_client_ids);
    
    return UNITY_END();
}
