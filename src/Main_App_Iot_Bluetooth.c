#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <iot_bluetooth.h>

// Pin thread to specific CPU core
void pin_thread_to_core(int core)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);

    pthread_t current = pthread_self();
    if (pthread_setaffinity_np(current, sizeof(cpu_set_t), &cpuset) != 0)
    {
        perror("pthread_setaffinity_np");
    }

    printf("Thread pinned to CPU %d (running on %d)\n",
           core, sched_getcpu());
}

void *cloud_forwarder_thread(void *arg)
{
    pin_thread_to_core(1); // Pin to CPU 0
    int client_id = 0;
    printf("cloud forwareder started\n");

    client_id = ipc_socket_client_init(8080);

    cloud_forwader(client_id);
}

void *ble_scanner_thread(void *arg)
{
    pin_thread_to_core(0); // Pin to CPU 0

    int server_id;

    printf("BLE Scanner thread started\n");

    server_id = ipc_socket_server_init(8080);

    bluetooth_app(server_id);
}

int main(void)
{
    pthread_t ble_thread, cloud_thread, processor_thread;

    printf("Starting multi-threaded BLE pipeline...\n");
    printf("Number of CPUs: %d\n", sysconf(_SC_NPROCESSORS_ONLN));

    // Create threads
    pthread_create(&ble_thread, NULL, ble_scanner_thread, NULL);

    sleep(1);

    pthread_create(&cloud_thread, NULL, cloud_forwarder_thread, NULL);

    // pthread_create(&processor_thread, NULL, data_processor_thread, NULL);

    // Wait for completion
    pthread_join(ble_thread, NULL);
    pthread_join(cloud_thread, NULL);

    // pthread_join(processor_thread, NULL);

    printf("All threads completed\n");

    return 0;
}
