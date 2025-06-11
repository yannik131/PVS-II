#include "util.h"
#include "vector.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

pthread_barrier_t barrier_count;
pthread_barrier_t barrier_gather;

/**
 * 4.1 a)
 * Mehrere Möglichkeiten:
 *  1. 10 Threads laufen für jede Ziffer parallel durch das gesamte Array und füllen die bins auf.
 * Einfachste Variante, da hier außer einer Barriere vor der Sammelphase kaum synchronisiert werden
 * muss, weil jeder Thread auf sein eigenes Array schreibt
 *  2. Das Array wird für n Threads in n Partitionen unterteilt, die jeder Thread jeweils einmal
 * iteriert. Jeder Thread schreibt auf seine eigene Kopie aller 10 Arrays für die bins. Die
 * Sammelphase wird dann kompliziert, da dann 10*n Arrays zusammengeführt werden müssen, aber das
 * ganze skaliert besser als 1.
 *  3. Wie 2., aber alle Threads teilen sich die 10 Arrays. Ständiges locken/unlocken wäre
 * erforderlich, was den Performancegewinn beeinträchtigt -> Mach ich nicht
 *
 * -> Variante 2. wird implementiert
 *
 * Vorgehen:
 * - Teile das Array in n gleich große Unterarrays/Segmente auf
 * - Jedem Thread wird ein Unterarray zugewiesen in aufsteigender Reihenfolge bezogen auf die
 * Segmentposition im Array
 * - Jeder Thread erhält eine Instanz von thread_info_t wo ein Zeiger auf das zu sortierende Array
 * enthalten ist und ein Zeiger auf ein Array mit 10 vector Instanzen
 * - Jeder Thread durchläuft die n < n_iterations Schleife mit einer Barriere am Schleifenende
 * - Nach dem Erreichen der Barriere sendet ein Thread ein Signal an den main-thread, der alle bins
 * in aufsteigender Reihenfolge für jede Ziffer durchiteriert und so das Array wieder auffüllt
 * (iteriere die n arrays für bin 0, dann bin 1, ..., bin 9)
 * -
 */

typedef struct {
    int *array;
    int start_index;
    int end_index;
    vector *vectors[10];
    int n_iterations;
} thread_info_t;

void *radix_sort_worker(void *arg) {
    thread_info_t *thread_info = (thread_info_t *)arg;

    for (int n = 0; n < thread_info->n_iterations; ++n) {
        // counting phase
        for (int i = thread_info->start_index; i < thread_info->end_index; ++i) {
            int digit = nth_digit(thread_info->array[i], n);
            vector_push_back(thread_info->vectors[digit], thread_info->array[i]);
        }

        pthread_barrier_wait(&barrier_count);
        pthread_barrier_wait(&barrier_gather);

        for (int i = 0; i < 10; ++i)
            vector_clear(thread_info->vectors[i]);
    }

    return NULL;
}

void calculate_segments(int N, int segment_count, int *segments) {
    int segment_width = N / segment_count;
    int remainder = N % segment_count;

    for (int i = 0; i < segment_count; ++i)
        segments[i] = i * segment_width + min(i, remainder);
}

void sort_array(int *array, int array_size, int number_of_threads) {
    if (number_of_threads <= 0 || number_of_threads > array_size) {
        fprintf(stderr, "Invalid number of threads\n");
        exit(1);
    }

    // Calculate digit count
    int max_value = -1;
    for (int i = 0; i < array_size; ++i) {
        assert(array[i] > 0);
        if (array[i] > max_value)
            max_value = array[i];
    }
    int n_iterations = digit_count(max_value);

    pthread_t *threads = (pthread_t *)malloc(number_of_threads * sizeof(pthread_t));
    thread_info_t *thread_infos =
        (thread_info_t *)malloc(number_of_threads * sizeof(thread_info_t));
    int *segments = (int *)malloc(number_of_threads * sizeof(int));
    calculate_segments(array_size, number_of_threads, segments);

    for (int i = 0; i < number_of_threads; ++i) {
        thread_infos[i].array = array;
        thread_infos[i].start_index = segments[i];
        thread_infos[i].end_index = (i == number_of_threads - 1) ? array_size : segments[i + 1];

        for (int j = 0; j < 10; ++j)
            thread_infos[i].vectors[j] = vector_create();

        thread_infos[i].n_iterations = n_iterations;

        pthread_create(&threads[i], NULL, radix_sort_worker, &thread_infos[i]);
    }

    for (int n = 0; n < n_iterations; ++n) {
        pthread_barrier_wait(&barrier_count);

        // collecting phase
        int current_index = 0;
        for (int i = 0; i < 10; ++i) {
            for (int j = 0; j < number_of_threads; ++j) {
                const int *data = vector_data(thread_infos[j].vectors[i]);
                const int size = vector_size(thread_infos[j].vectors[i]);

                for (int k = 0; k < size; ++k)
                    array[current_index++] = data[k];
            }
        }

        pthread_barrier_wait(&barrier_gather);
    }

    for (int i = 0; i < number_of_threads; ++i)
        pthread_join(threads[i], NULL);

    // Cleanup
    for (int i = 0; i < number_of_threads; ++i) {
        for (int j = 0; j < 10; ++j)
            vector_free(thread_infos[i].vectors[j]);
    }

    free(thread_infos);
    free(segments);
    free(threads);
}

int main(int argc, char **argv) {
    if (argc != 2 && argc != 3) {
        printf("Usage: %s <number of threads> [-n <int>]\n", argv[0]);
        return 0;
    }

    vector *vector;

    if (argc == 2) {
        const char *path = "numbers.txt";
        vector = read_numbers_from_file(path);
    } else {
        int n = atoi(argv[2]);
        vector = generate_random_array(n);
    }

    int *array = vector_data(vector);

    int number_of_threads = atoi(argv[1]);
    pthread_barrier_init(&barrier_count, NULL, number_of_threads + 1);
    pthread_barrier_init(&barrier_gather, NULL, number_of_threads + 1);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    sort_array(array, vector_size(vector), number_of_threads);
    clock_gettime(CLOCK_MONOTONIC, &end);

    assert(array_is_sorted(array, vector_size(vector)));

    print_elapsed_time(start, end);

    vector_free(vector);
}
