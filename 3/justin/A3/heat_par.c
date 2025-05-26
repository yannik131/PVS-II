/*
    a)
    for (int i = 1; i < grid_size; i++)
        // ...
    Nicht parallelisierbar, da das Ergbnis der vorherigen Iteration benötigt wird (T_k[i] = ... T_k[i-1] ...).

    for (int k = 0; k < num_time_steps; k++)
    {
        // ...
    }
    Nicht parallelisierbar, da die Berechnung von T_kn auf T_k basiert (was dem T_kn der letzten Iteration entspricht).

    for (int i = 0; i < grid_size; i++)
    {
        // ...
    }
    Parallelisierbar, da T_k sich in der Schleife nicht ändert.
    Die Parallelisierung ist auch lohnenswert, da diese Schleife num_time_steps-Mal verwendet wird.

    for (int i = 0; i < grid_size; i++)
    {
        // ...
    }
    Parallelsierbar, wenn das setzen von (z.B. globalen) T_max und T_min z.B. durch Mutexe geschützt wird.
    Aber nicht lohnenswert, da die Schleife nur einmal verwendet wird.

    for (int i = 0; i < grid_size; i++)
        // ...
    Parallelsierbar, wenn das setzen von T_average z.B. durch einen Mutex geschützt wird.
    Aber nicht lohnenswert, da die Schleife nur einmal verwendet wird.

    c)
    Benötigte Zeit auf btpoolai52 (24 Worker-Threads):
    Parallel: ~42 s
    Sequential: ~210 s
    Speedup:    210 / 42 = 5

    Der Speedup ist geringer, da Overhead (für die Barriere und Erstellung/Joinen der Threads) anfällt.
    Zudem ist nicht das gesamte Program parallelisierbar.
*/

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

typedef struct _targs_t {
    double *T_k, *T_kn;
    long start, end;
} targs_t;

// make the constants global
// (otherwise we would have to send them in the targs for every thread)
int num_time_steps = 3000;
int grid_size = (32 * 1024 * 1024);
double delta_t = 0.02;
double conductivity_constant = 0.1;

void* thread_inner(void* _args);
void print_arr(double* A) {
    for (int i = 0; i < grid_size; i++) {
        printf("%f%s", A[i], (i == grid_size - 1)? "\n" : ", ");
    }
}

pthread_barrier_t barrier;

int main(int argc, char** args)
{
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Current temperature
    double* T_k = malloc(sizeof(double) * grid_size);
    // Next temperature
    double* T_kn = malloc(sizeof(double) * grid_size);

    // Setting the initial values
    T_k[0] = 1. / 2.;
    for (int i = 1; i < grid_size; i++)
        T_k[i] = 3.59 * T_k[i - 1] * (1 - T_k[i - 1]);

    // Get number of online logical processors
    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);

    pthread_t threads[nprocs];
    targs_t targs[nprocs];
    pthread_barrier_init(&barrier, NULL, nprocs);

    long length = grid_size / nprocs;

    for (int i = 0; i < nprocs; i++) {
        // set the targs values
        targs[i].T_k = T_k;
        targs[i].T_kn = T_kn;
        targs[i].start = i * length;
        targs[i].end = (i + 1) * length + ((i == nprocs - 1) ? grid_size % nprocs : 0);

        // printf("i = %d, start = %ld, end = %ld\n", i, targs[i].start, targs[i].end);

        pthread_create(&threads[i], NULL, thread_inner, &targs[i]);
    }

    for (int i = 0; i < nprocs; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);

    // Computing statistics of the final temperature of the grid

    double T_max = DBL_MIN;
    double T_min = DBL_MAX;
    double T_average = 0;

    for (int i = 0; i < grid_size; i++)
    {
        T_max = T_max > T_k[i] ? T_max : T_k[i];
        T_min = T_min < T_k[i] ? T_min : T_k[i];
    }

    for (int i = 0; i < grid_size; i++)
       T_average += T_k[i];

    T_average = T_average / grid_size;

    printf("T_max: %f, T_min: %f, T_average: %f\n", T_max, T_min, T_average);

    gettimeofday(&end, NULL);
    long long time_spent_us = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
    long long time_spent_ms = time_spent_us / 1000;
    printf("Time spent (Parallel): %lld ms\n", time_spent_ms);

    return 0;
}

void* thread_inner(void* _args) {
    targs_t* args = (targs_t*) _args;

    double *T_k = args->T_k, *T_kn = args->T_kn;
    long start = args->start, end = args->end;

    for (int k = 0; k < num_time_steps; k++)
    {
        pthread_barrier_wait(&barrier);
        for (long i = start; i < end; i++)
        {
            // Computing the temporal derivate of the ith grid cell 
            double dTdt_i = conductivity_constant * (-2*T_k[i] +
                T_k[i != 0 ? i - 1 : 1] +
                T_k[i != grid_size - 1 ? i + 1 : i - 1]);

            // Using explicit Euler method to compute the next temperature
            // of the ith grid cell
            T_kn[i] = T_k[i] + delta_t * dTdt_i;
        }

        // Swapping T_kn and T_k
        double* temp = T_kn;
        T_kn = T_k;
        T_k = temp;
    }

    return NULL;
}