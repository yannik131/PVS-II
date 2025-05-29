#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <time.h>
#include <pthread.h>

#ifdef __APPLE__
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int trip_count;
} pthread_barrier_t;

int pthread_barrier_init(pthread_barrier_t *barrier, void *attr, unsigned count) {
    barrier->count = 0;
    barrier->trip_count = count;
    pthread_mutex_init(&barrier->mutex, NULL);
    pthread_cond_init(&barrier->cond, NULL);
    return 0;
}

int pthread_barrier_wait(pthread_barrier_t *barrier) {
    pthread_mutex_lock(&barrier->mutex);
    barrier->count++;
    if (barrier->count >= barrier->trip_count) {
        barrier->count = 0;
        pthread_cond_broadcast(&barrier->cond);
    } else {
        pthread_cond_wait(&barrier->cond, &barrier->mutex);
    }
    pthread_mutex_unlock(&barrier->mutex);
    return 0;
}

int pthread_barrier_destroy(pthread_barrier_t *barrier) {
    pthread_mutex_destroy(&barrier->mutex);
    pthread_cond_destroy(&barrier->cond);
    return 0;
}
#endif

/**
 * 3.3 a)
 * Loop 3, 4 & 5 (siehe Kommentare) parallelisierbar
 * Loop 1: sequenzielle Abhängigkeit von i --> i - 1
 * Loop 2: zeitliche Abhängigkeit (Swapping T_kn and T_k)
 * 
 * 
 * 3.3 c)
 * 8 Kerne:
 * Sequential: T_max: 0.650660, T_min: 0.635110, T_average: 0.646300
 * Sequential: 87.771750s
 * Sequential: T_max: 0.650660, T_min: 0.635110, T_average: 0.646300
 * Parallel: 31.615235s
 * 
 * --> Speedup 87.77/31.61 = 2.77
 * 
 * 24 Kerne:
 * Sequential: T_max: 0.650660, T_min: 0.635110, T_average: 0.646300
 * Sequential: 87.490963s
 * Sequential: T_max: 0.650660, T_min: 0.635110, T_average: 0.646300
 * Parallel: 37.772890s
 * 
 * --> 87.49/37.77 = 2.31
 * 
 * Das Programm ist nicht vollständig parallelisierbar, da der Loop 2 weiterhin sequenziell ausgeführt werden muss.
 * Deswegen ist der Speedup deutlich geringer als die Anzahl der genutzten Threads
 * Im 2. Fall mit 24 verwendeten Kernen ist der häufige Kontextwechsel bei den barriers größer als der Nutzen der
 * Parallelisierung im Vergleich zur Verwendung von 8 Threads
 */



int grid_size = (32 * 1024 * 1024);
int num_time_steps = 3000;
double delta_t = 0.02;
double conductivity_constant = 0.1;

int num_threads = 1;

double *T_k,* T_kn;
pthread_barrier_t barrier;

typedef struct _thread_args_t
{
    // input
    int thread_id;

    // output
    double T_max;
    double T_min;
    double T_sum;
} thread_args_t;


void print_elapsed_time(char* description, struct timespec start, struct timespec end) {
    const double time_ns = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    const char *time_units[] = {"ns", "us", "ms", "s"};

    int i = 0;
    double converted_time = time_ns;
    while (converted_time > 1e3 && i < (sizeof(time_units) / sizeof(time_units[0])) - 1) {
        converted_time /= 1e3;
        ++i;
    }

    printf("%s: %lf%s\n", description, converted_time, time_units[i]);
}

void* run_temperature_sim(void *args) {
    thread_args_t *thread_args = (thread_args_t*) args;

    int num_cells_per_thread = grid_size / num_threads;
    int start_cell = thread_args->thread_id * num_cells_per_thread;
    int end_cell;
    if (thread_args->thread_id == num_threads - 1)
        end_cell = grid_size;
    else
        end_cell = (thread_args->thread_id + 1) * num_cells_per_thread;
    
    // Time Loop
    // Loop 2
    for (int k = 0; k < num_time_steps; k++) {
        // System Loop
        // Computes the next temperature for each grid cell
        // Loop 3
        for (int i = start_cell; i < end_cell; i++) {
            // Computing the temporal derivate of the ith grid cell 
            double dTdt_i = conductivity_constant * (-2*T_k[i] +
                T_k[i != 0 ? i - 1 : 1] +
                T_k[i != grid_size - 1 ? i + 1 : i - 1]);

            // Using explicit Euler method to compute the next temperature
            // of the ith grid cell
            T_kn[i] = T_k[i] + delta_t * dTdt_i;
        }

        pthread_barrier_wait(&barrier);

        if (thread_args->thread_id == 0) {
            // Swapping T_kn and T_k
            double* temp = T_kn;
            T_kn = T_k;
            T_k = temp;
        }

        pthread_barrier_wait(&barrier);
    } 

    // Computing statistics for each thread in its range
    double T_max = DBL_MIN;
    double T_min = DBL_MAX;
    double T_sum = 0;

    // Loop 4
    for (int i = start_cell; i < end_cell; i++)
    {
        T_max = T_max > T_k[i] ? T_max : T_k[i];
        T_min = T_min < T_k[i] ? T_min : T_k[i];
    }

    // Loop 5
    for (int i = start_cell; i < end_cell; i++)
       T_sum += T_k[i];
    
    thread_args->T_max = T_max;
    thread_args->T_min = T_min;
    thread_args->T_sum = T_sum;

    return NULL;
}

int calc_parallel() {
    // Setting the initial values
    T_k[0] = 1. / 2.;
    // Loop 1
    for (int i = 1; i < grid_size; i++) {
        T_k[i] = 3.59 * T_k[i - 1] * (1 - T_k[i - 1]);
    }

    pthread_barrier_init(&barrier, NULL, num_threads);

    pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    thread_args_t *thread_args = (thread_args_t *)malloc(num_threads * sizeof(thread_args_t));

    for(int i = 0; i < num_threads; i++) {
        thread_args[i].thread_id = i;
        pthread_create(&threads[i], NULL, run_temperature_sim, &thread_args[i]);
    }

    // Wait for the threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Computing statistics of the final temperature of the grid
    double T_max = DBL_MIN;
    double T_min = DBL_MAX;
    double T_average = 0;

    for (int i = 0; i < num_threads; i++)
    {
        T_max = T_max > thread_args[i].T_max ? T_max : thread_args[i].T_max;
        T_min = T_min < thread_args[i].T_min ? T_min : thread_args[i].T_min;
    }

    for (int i = 0; i < num_threads; i++)
       T_average += thread_args[i].T_sum;

    T_average = T_average / grid_size;

    printf("Sequential: T_max: %f, T_min: %f, T_average: %f\n", T_max, T_min, T_average);


    return 0;
}

int calc_sequencial() {
    // Setting the initial values
    T_k[0] = 1. / 2.;
    // Loop 1
    for (int i = 1; i < grid_size; i++) {
        T_k[i] = 3.59 * T_k[i - 1] * (1 - T_k[i - 1]);
    }

    // Time loop 
    // Loop 2
    for (int k = 0; k < num_time_steps; k++)
    {
        // System loop
        // Computes the next temperature for each grid cell
        // Loop 3
        for (int i = 0; i < grid_size; i++)
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

    // Computing statistics of the final temperature of the grid
    double T_max = DBL_MIN;
    double T_min = DBL_MAX;
    double T_average = 0;

    // Loop 4
    for (int i = 0; i < grid_size; i++)
    {
        T_max = T_max > T_k[i] ? T_max : T_k[i];
        T_min = T_min < T_k[i] ? T_min : T_k[i];
    }

    // Loop 5
    for (int i = 0; i < grid_size; i++)
       T_average += T_k[i];

    T_average = T_average / grid_size;

    printf("Sequential: T_max: %f, T_min: %f, T_average: %f\n", T_max, T_min, T_average);
    return 0;
}


int main(int argc, char** args)
{
    if (argc != 2) {
        printf("Usage: %s <number of threads>\n", args[0]);
        return 0;
    }

    num_threads = atoi(args[1]);

    // Current temperature
    T_k = malloc(sizeof(double) * grid_size);
    // Next temperature
    T_kn = malloc(sizeof(double) * grid_size);


   struct timespec start, end;
   clock_gettime(CLOCK_MONOTONIC, &start);
   calc_sequencial();
   clock_gettime(CLOCK_MONOTONIC, &end);
   print_elapsed_time("Sequential", start, end);

   clock_gettime(CLOCK_MONOTONIC, &start);
   calc_parallel();
   clock_gettime(CLOCK_MONOTONIC, &end);
   print_elapsed_time("Parallel", start, end);
}