#include "util.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *print_ID(void *) {
    pthread_t threadID = pthread_self();
    printf("Thread ID is %lu\n", (unsigned long)threadID);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <number of threads>\n", argv[0]);
        return 0;
    }

    long N;
    convert_to_number(argv[1], &N, LONG);

    pthread_t *threads = (pthread_t *)malloc(N * sizeof(pthread_t));
    abort_on_failed_allocation(threads);

    for (int i = 0; i < N; ++i) {
        int result = pthread_create(&threads[i], NULL, print_ID, NULL);

        if (result != 0) {
            fprintf(stderr, "Error creating thread\n");
            exit(1);
        }
    }

    for (int i = 0; i < N; ++i) {
        int result = pthread_join(threads[i], NULL);

        if (result != 0) {
            fprintf(stderr, "Error joining thread\n");
            exit(1);
        }
    }

    free(threads);

    printf("I started %li threads.\n", N);

    return 0;
}