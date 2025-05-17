#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * Output:
    Mutex: sum = 1048576
    Elapsed time: 9.607800ms
    Partial sums: sum = 1048576
    Elapsed time: 362.100000us
 * The mutex version is somewhat (~10x) slower because we have a total of 2**20 calls to
 * pthread_mutex_lock and pthread_mutex_unlock each and each thread has to wait for the other
 * making the calculation essentially sequential
 */

int sum = 0;
const int N = 1048576;
int *array;
int sum1 = 0, sum2 = 0;
pthread_mutex_t mutex;

void *thread1_mutex(void *arg) {
    for (int i = 0; i < N / 2; ++i) {
        pthread_mutex_lock(&mutex);
        sum += array[i];
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void *thread2_mutex(void *arg) {
    for (int i = N / 2; i < N; ++i) {
        pthread_mutex_lock(&mutex);
        sum += array[i];
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void *thread1_partial(void *arg) {
    for (int i = 0; i < N / 2; ++i)
        sum1 += array[i];

    return NULL;
}

void *thread2_partial(void *arg) {
    for (int i = N / 2; i < N; ++i)
        sum2 += array[i];

    return NULL;
}

void print_elapsed_time(struct timespec start, struct timespec end) {
    const double time_ns = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    const char *time_units[] = {"ns", "us", "ms", "s"};

    int i = 0;
    double converted_time = time_ns;
    while (converted_time > 1e3 && i < (sizeof(time_units) / sizeof(time_units[0])) - 1) {
        converted_time /= 1e3;
        ++i;
    }

    printf("Elapsed time: %lf%s\n", converted_time, time_units[i]);
}

int main() {
    array = (int *)malloc(sizeof(int) * N);
    for (int i = 0; i < N; ++i)
        array[i] = 1;

    pthread_t threads[4];

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_create(&threads[0], NULL, thread1_mutex, NULL);
    pthread_create(&threads[1], NULL, thread2_mutex, NULL);

    for (int i = 0; i < 2; ++i)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("Mutex: sum = %d\n", sum);
    print_elapsed_time(start, end);

    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_create(&threads[2], NULL, thread1_partial, NULL);
    pthread_create(&threads[3], NULL, thread2_partial, NULL);

    for (int i = 2; i < 4; ++i)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("Partial sums: sum = %d\n", sum1 + sum2);
    print_elapsed_time(start, end);

    free(array);
}