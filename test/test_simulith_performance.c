#include "unity.h"
#include "simulith.h"
#include <pthread.h>
#include <unistd.h>

#define SIMULITH_ADDRESS "tcp://127.0.0.1:5555"
#define NUM_CLIENTS 4
#define NUM_UPDATES 100

typedef struct {
    int id;
    volatile int ready;
    volatile int done;
} client_thread_arg_t;

void setUp (void) {} /* Is run before every test, put unit init calls here. */
void tearDown (void) {} /* Is run after every test, put unit clean-up calls here. */

static void *server_thread_func(void *arg) {
    (void)arg;
    TEST_ASSERT_EQUAL(0, simulith_server_init(SIMULITH_ADDRESS));

    for (uint64_t t = 1; t <= NUM_UPDATES; t++) {
        simulith_server_publish_time(t);
        usleep(1000); // 1 ms between updates (simulate faster than real-time)
    }

    simulith_server_shutdown();
    return NULL;
}

static void *client_thread_func(void *arg) {
    client_thread_arg_t *cta = (client_thread_arg_t *)arg;
    TEST_ASSERT_EQUAL(0, simulith_client_init(SIMULITH_ADDRESS));
    cta->ready = 1;

    for (uint64_t expected = 1; expected <= NUM_UPDATES; expected++) {
        TEST_ASSERT_EQUAL(0, simulith_client_wait_for_time(expected));
        uint64_t current_time = simulith_client_get_time();
        TEST_ASSERT_TRUE(current_time >= expected);
    }

    simulith_client_shutdown();
    cta->done = 1;
    return NULL;
}

void test_simulith_multi_client_performance(void) {
    pthread_t server_thread;
    pthread_t client_threads[NUM_CLIENTS];
    client_thread_arg_t args[NUM_CLIENTS];

    // Start server
    TEST_ASSERT_EQUAL(0, pthread_create(&server_thread, NULL, server_thread_func, NULL));

    // Start clients
    for (int i = 0; i < NUM_CLIENTS; i++) {
        args[i].id = i;
        args[i].ready = 0;
        args[i].done = 0;
        TEST_ASSERT_EQUAL(0, pthread_create(&client_threads[i], NULL, client_thread_func, &args[i]));
    }

    // Wait for clients to be ready
    for (int i = 0; i < NUM_CLIENTS; i++) {
        while (!args[i].ready) { usleep(1000); }
    }

    // Measure start time
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    // Wait for clients to finish receiving all updates
    for (int i = 0; i < NUM_CLIENTS; i++) {
        while (!args[i].done) { usleep(1000); }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    // Calculate elapsed time in milliseconds
    double elapsed_ms = (end_time.tv_sec - start_time.tv_sec) * 1000.0
                      + (end_time.tv_nsec - start_time.tv_nsec) / 1e6;

    printf("Multi-client performance test completed in %.2f ms\n", elapsed_ms);

    pthread_join(server_thread, NULL);
    for (int i = 0; i < NUM_CLIENTS; i++) {
        pthread_join(client_threads[i], NULL);
    }

    // Example assertion: performance target (adjust as needed)
    TEST_ASSERT_TRUE(elapsed_ms < 5000); // 5 seconds max for 100 updates with 4 clients
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_simulith_multi_client_performance);
    return UNITY_END();
}
