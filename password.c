#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <stdatomic.h>

#define PASSWORD "Penis"
#define ASCII_START 32
#define ASCII_END 126
#define THREAD_COUNT 12
#define PRINT_INTERVAL 1 // Print every n attempts

typedef struct {
    char start_char;
    int thread_id;
} ThreadData;

atomic_long attempts = 0; // Atomic variable for attempts
atomic_int found = 0; // Atomic variable for found flag

void* brute_force(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int length = strlen(PASSWORD);
    char attempt[length + 1];
    attempt[length] = '\0';

    // Initialize the attempt with the starting character
    for (int i = 0; i < length; i++) {
        attempt[i] = ASCII_START;
    }
    attempt[0] = data->start_char;

    while (atomic_load(&found) == 0) {
        // Check if the current attempt matches the password
        if (memcmp(attempt, PASSWORD, length) == 0) {
            atomic_store(&found, 1);
            printf("PASSWORD FOUND by thread %d: %s\n", data->thread_id, attempt);
            return NULL;
        }

        // Increment the attempt
        int index = length - 1;
        while (index >= 0) {
            if (attempt[index] < ASCII_END) {
                attempt[index]++;
                break;
            } else {
                attempt[index] = ASCII_START;
                index--;
            }
        }

        // If we wrapped around all characters, exit
        if (index < 0) return NULL;

        // Increment the attempts count
        long current_attempts = atomic_fetch_add(&attempts, 1);
        if (current_attempts % PRINT_INTERVAL == 0) {
            printf("Attempt #%ld: %s\n", current_attempts, attempt);
        }
    }
    return NULL;
}

int main() {
    pthread_t threads[THREAD_COUNT];
    ThreadData thread_data[THREAD_COUNT];

    printf("Starting brute-force attack with %d threads...\n", THREAD_COUNT);
    clock_t start_time = clock();

    for (int i = 0; i < THREAD_COUNT; i++) {
        thread_data[i].start_char = ASCII_START + (i * (ASCII_END - ASCII_START) / THREAD_COUNT);
        thread_data[i].thread_id = i;
        pthread_create(&threads[i], NULL, brute_force, &thread_data[i]);
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    printf("Total attempts: %ld\n", atomic_load(&attempts)); // Use atomic_load to get the value
    printf("Time taken: %.2f seconds\n", time_taken);

    return 0;
}