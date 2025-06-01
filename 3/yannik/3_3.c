#include <float.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * 3.3 a)
 * Möglich: Wenn die Ergebnisse der Iterationen nicht voneinander abhängen und die
 * Ausführungszeit nicht durch I/O begrenzt ist sondern durch die CPU
 *
 * Lohnenswert: Wenn für ein
 * langsames Programm der Profiler zeigt, dass die Schleife einen wesentlichen Anteil an der
 * Ausführungszeit hat
 *
 * 3.3 c)
 * Speedups siehe 3_3_speedups.png. Er ist fast vernachlässigbar: Der Overhead für die
 * Threadkommunikation (insbesondere wohl das barrier_wait) überwiegt den
 * Geschwindigkeitsvorteil durch Parallelisierung
 *
 * 3.4 a)
 * Einzelne Blöcke im Cache haben oft eine Größe von etwa 64 Bytes und werden cache
 * lines genannt. Benachbarte Elemente eines Arrays, die von mehreren Threads genutzt werden, können
 * in derselben cache line liegen. Jeder Prozessor hat seine eigene, lokale Kopie des Caches.
 * Schreibt bspw. Thread 1 in die cache line auf seinem Prozessor, muss Thread 2 seine lokale Kopie
 * neu laden (um cache coherence zu erhalten), obwohl das gar nicht nötig wäre.
 *
 * 3.4 b) Mithilfe von posix_memalign kann ermittelt werden, wie groß eine cache line ist. Dann muss
 * jeder Thread seine Mutexes im Array mit leerem Speicher mindestens der Größe einer cache line
 * padden, bspw. indem zusätzliche, ungenutzte mutexes in das Array gelegt werden. Dann liegt jeder
 * mutex auf seiner eigenen cache line, was das Neuladen verhindert.
 *
 * 3.4 c) Das Lock ist der Teil vom Mutex, der regelmäßig beschrieben werden muss und false sharing
 * verursachen kann. Liegt das Lock auf dem Heap, sind die zu schreibenden Daten häufiger weit
 * auseinander (es sei denn malloc legt sie zufällig direkt nebeneinander, was unwahrscheinlich
 * ist). Das würde false sharing verhindern. Liegt das Lock im Mutex, wird false sharing
 * wahrscheinlicher.
 */

pthread_barrier_t barrier;

typedef struct {
    double *T_k;
    double *T_kn;
    double conductivity_constant;
    int start_index;
    int end_index;
    int grid_size;
    int num_time_steps;
    double delta_t;
} grid_t;

int min(int a, int b) {
    if (a < b)
        return a;

    return b;
}

void calculate_segments(int N, int segment_count, int *segments) {
    int segment_width = N / segment_count;
    int remainder = N % segment_count;

    for (int i = 0; i < segment_count; ++i)
        segments[i] = i * segment_width + min(i, remainder);
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

void *calc_next_step(void *arg) {
    grid_t grid = *(grid_t *)arg;

    for (int j = 0; j < grid.num_time_steps; ++j) {
        for (int i = grid.start_index; i < grid.end_index; i++) {
            double dTdt_i =
                grid.conductivity_constant * (-2 * grid.T_k[i] + grid.T_k[i != 0 ? i - 1 : 1] +
                                              grid.T_k[i != grid.grid_size - 1 ? i + 1 : i - 1]);
            grid.T_kn[i] = grid.T_k[i] + grid.delta_t * dTdt_i;
        }

        double *temp = grid.T_kn;
        grid.T_kn = grid.T_k;
        grid.T_k = temp;

        pthread_barrier_wait(&barrier);
    }

    return NULL;
}

void init_grid(grid_t *grid) {
    grid->grid_size = (30 * 1024 * 1024);
    grid->delta_t = 0.02;
    grid->conductivity_constant = 0.1;
    grid->num_time_steps = 300;

    // Current temperature
    grid->T_k = malloc(sizeof(double) * grid->grid_size);
    // Next temperature
    grid->T_kn = malloc(sizeof(double) * grid->grid_size);

    // Setting the initial values
    grid->T_k[0] = 1. / 2.;
    for (int i = 1; i < grid->grid_size; i++)
        grid->T_k[i] = 3.59 * grid->T_k[i - 1] * (1 - grid->T_k[i - 1]);
}

grid_t *create_thread_grids(grid_t base_grid, int number_of_threads) {
    int *segments = (int *)malloc(sizeof(int) * number_of_threads);
    calculate_segments(base_grid.grid_size, number_of_threads, segments);

    grid_t *grids = (grid_t *)malloc(number_of_threads * sizeof(grid_t));

    for (int i = 0; i < number_of_threads; ++i) {
        grids[i] = base_grid;
        grids[i].start_index = segments[i];
        grids[i].end_index = (i == number_of_threads - 1) ? base_grid.grid_size : segments[i + 1];
    }

    free(segments);

    return grids;
}

grid_t run_simulation(int number_of_threads) {
    grid_t base_grid;
    init_grid(&base_grid);
    grid_t *grids = create_thread_grids(base_grid, number_of_threads);

    pthread_t *threads = (pthread_t *)malloc(number_of_threads * sizeof(pthread_t));

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < number_of_threads; ++i)
        pthread_create(&threads[i], NULL, calc_next_step, &grids[i]);

    for (int i = 0; i < number_of_threads; ++i)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);

    print_elapsed_time(start, end);

    free(threads);
    free(grids);

    return base_grid;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <number of threads>\n", argv[0]);
        return 0;
    }

    int number_of_threads = atoi(argv[1]);
    pthread_barrier_init(&barrier, NULL, number_of_threads);

    grid_t base_grid = run_simulation(number_of_threads);

    // Computing statistics of the final temperature of the grid

    double T_max = DBL_MIN;
    double T_min = DBL_MAX;
    double T_average = 0;

    for (int i = 0; i < base_grid.grid_size; i++) {
        T_max = T_max > base_grid.T_k[i] ? T_max : base_grid.T_k[i];
        T_min = T_min < base_grid.T_k[i] ? T_min : base_grid.T_k[i];
    }

    for (int i = 0; i < base_grid.grid_size; i++)
        T_average += base_grid.T_k[i];

    T_average = T_average / base_grid.grid_size;

    printf("T_max: %f, T_min: %f, T_average: %f", T_max, T_min, T_average);

    free(base_grid.T_k);
    free(base_grid.T_kn);

    return 0;
}