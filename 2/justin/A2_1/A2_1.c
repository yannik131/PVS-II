/*
    Time measurements on a AMD Ryzen 7 7800 X3D
a)
    Time spent for slow version: ~18ms
b)
    Time spent for fast version: ~1ms
c)
    Die Version aus b) ist deutlich schneller (~18x), da die Threads seltener warten müssen, um den Mutex zu bekommen.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

struct args {
    double* a;
    double* b;
    unsigned int start;
    unsigned int end;
    pthread_mutex_t* mutex;
    double* sum;
};

const char USAGE[75] = "./A2_1 [0 or 1]\n0: Claim mutex every time\n1: Claim mutex just once\n";

struct args set_args(double* a, double* b, unsigned int start, unsigned int end, pthread_mutex_t* mutex, double* sum);
void* scalar_slow(void* _args);
void* scalar_fast(void* _args);

int main(int argc, char** argv) {
    int version = -1;

    if (argc != 2) {
        printf("%s", USAGE);
        return 1;
    }
    else {
        version = atoi(argv[1]);
        if (version != 0 && version != 1) {
            printf("%s", USAGE);
            return 1;
        }
    }

    // select the chosen version
    void* (*fptr)(void*) = version ? &scalar_fast : &scalar_slow;

    // the length of the arrays is 2^20
    const unsigned int n = 1 << 20;

    // initialize a and b with random numbers between -50 and 50
    double *a = malloc(sizeof(double) * n), *b = malloc(sizeof(double) * n);
    // we use a constant seed to be able to compare the different versions
    srand(0);
    for (unsigned int i = 0; i < n; i++) {
        a[i] = rand() / (double) RAND_MAX * 100.0 - 50.0;
        b[i] = rand() / (double) RAND_MAX * 100.0 - 50.0;
    }

    // setup to test the given version
    pthread_t threads[2];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    double sum = 0;
    struct args args_0 = set_args(a, b, 0, n / 2, &mutex, &sum);
    struct args args_1 = set_args(a, b, n / 2, n, &mutex, &sum);

    // setup for time measurement
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // execute the calculation
    pthread_create(&threads[0], NULL, fptr, &args_0);
    pthread_create(&threads[1], NULL, fptr, &args_1);

    // join the threads
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    gettimeofday(&end, NULL);
    printf("Result: %f\n", sum);
    long long time_spent_us = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
    printf("Time spent for the %s version: %lld s %lld ms\n", version? "fast" : "slow", time_spent_us / 1000000ll, time_spent_us / 1000ll);

    free(b);
    free(a);

    return 0;
}

/*
    Creates and returns an args struct with the given values.
*/
struct args set_args(double* a, double* b, unsigned int start, unsigned int end, pthread_mutex_t* mutex, double* sum) {
    struct args _args;
    _args.a = a; _args.b = b;
    _args.start = start; _args.end = end;
    _args.mutex = mutex; _args.sum = sum;
    return _args;
}

/*
    Adds the scalar product of (a[start],...,a[end]) and (b[start],...,b[end]) to sum.
    The sum is updated in every step by locking the mutex (because multiple threads may be calculating the total scalar product).

    The argument _args has to be of type struct args.
    a and b have to be allocated arrays at least of size end.
    Any thread using sum has to use the given mutex.
*/
void* scalar_slow(void* _args) {
    struct args* args = (struct args*) _args;

    for (unsigned int i = args->start; i < args->end; i++) {
        pthread_mutex_lock(args->mutex);
        *(args->sum) += args->a[i] * args->b[i];
        pthread_mutex_unlock(args->mutex);
    }

    return NULL;
}

/*
    Adds the scalar product of (a[start],...,a[end]) and (b[start],...,b[end]) to sum.
    The sum is updated once in the end to add the local sum.

    The argument _args has to be of type struct args.
    a and b have to be allocated arrays at least of size end.
    Any thread using sum has to use the given mutex.
*/
void* scalar_fast(void* _args) {
    struct args* args = (struct args*) _args;

    double local_sum = 0;
    for (unsigned int i = args->start; i < args->end; i++) {
        local_sum += args->a[i] * args->b[i];
    }

    pthread_mutex_lock(args->mutex);
    *(args->sum) += local_sum;
    pthread_mutex_unlock(args->mutex);

    return NULL;
}