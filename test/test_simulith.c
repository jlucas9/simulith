#include "unity.h"
#include "simulith.h"
#include <pthread.h>
#include <unistd.h>

#define SIMULITH_ADDRESS "tcp://127.0.0.1:5555"

void setUp (void) {} /* Is run before every test, put unit init calls here. */
void tearDown (void) {} /* Is run after every test, put unit clean-up calls here. */

static void *server_thread_func(void *arg) {
    (void)arg;
    TEST_ASSERT_EQUAL(0, simulith_server_init(SIMULITH_ADDRESS));

    // Publish time increments for test
    for (uint64_t t = 1000; t <= 5000; t += 1000) {
        simulith_server_publish_time(t);
        usleep(100000); // Sleep 100ms to allow client to receive
    }

    simulith_server_shutdown();
    return NULL;
}

void test_simulith_client_server_interaction(void) {
    pthread_t server_thread;

    // Start server in background thread
    TEST_ASSERT_EQUAL(0, pthread_create(&server_thread, NULL, server_thread_func, NULL));

    // Give server a moment to start
    usleep(200000);

    TEST_ASSERT_EQUAL(0, simulith_client_init(SIMULITH_ADDRESS));

    // Wait for time updates from server and validate
    for (uint64_t expected = 1000; expected <= 5000; expected += 1000) {
        TEST_ASSERT_EQUAL(0, simulith_client_wait_for_time(expected));
        uint64_t current_time = simulith_client_get_time();
        TEST_ASSERT_TRUE(current_time >= expected);
    }

    simulith_client_shutdown();

    pthread_join(server_thread, NULL);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_simulith_client_server_interaction);
    return UNITY_END();
}
