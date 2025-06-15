#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h>
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

int NUMBER_OF_THREADS = 8;
int NUMBER_FORMAT = 10;

typedef struct {
    int* data;
    int size;
} IntArray;

typedef struct _thread_args_t
{
    IntArray *array;
    int* bucket_sizes_per_thread;
    int iterations;
    int thread_id;
    int start_index;
    int end_index;
    pthread_barrier_t* barrier;
    pthread_mutex_t* mutex;
} thread_args_t;

void print_array(IntArray array, char* prefix) {
    if (prefix == NULL) {
        printf("\n[");
    } else {
        printf("%s\n[", prefix);
    }
    for (int i = 0; i < array.size; i++) {
        if (i + 1 < array.size) {
            printf("%d, ", array.data[i]);
        } else {
            printf("%d]\n\n", array.data[i]);
        }
    }
}

int number_to_digits(int number) {
    int digits = 1;
    if(number < 0) { 
        number = (number == INT_MIN) ? INT_MAX : - number;
    }

    while (number > 9) {
        number /= NUMBER_FORMAT;
        digits ++;
    }
    return digits;
}

int max(IntArray *array) {
    int max_number = INT_MIN;
    for (int i = 0; i < array->size; i++) {
        max_number = array->data[i] > max_number ? array->data[i] : max_number;
    }
    return max_number;
}

int get_digit(int number, int digit) {
    int res = 0;
    for (int i = 0; i< digit; i++) {
        res = number % NUMBER_FORMAT;
        number /= NUMBER_FORMAT;
    }
    return res;
}

void push(IntArray * bucket, int value) {
    bucket->data[bucket->size++] = value;
}

void radix_partitioning(IntArray *array, IntArray* buckets[], int iteration_digit) {
    for (int i = 0; i < array->size; i++) {
        int value = array->data[i];
        push(buckets[get_digit(value, iteration_digit)], value);
    }
}

void radix_collection(IntArray *array, IntArray* buckets[]) {
    int pos = 0;
    for (int i = 0; i < NUMBER_FORMAT; i ++) {
        for(int j = 0; j < buckets[i]->size; j++) {
            array->data[pos++] = buckets[i]->data[j];
        }
        // bucket memory is the same for all iterations. The free is after radix is completed
        buckets[i]->size = 0;
    }
}

/**
 * Sorts the given array inline
 */
void radix(IntArray *array) {
    int iterations = number_to_digits(max(array));

    IntArray* buckets[NUMBER_FORMAT];

    for (int i = 0; i < NUMBER_FORMAT; i++) {
        buckets[i] = malloc(sizeof(IntArray));
        if (buckets[i] == NULL) {
            printf("Memory allocation failed for bucket[%d]\n", i);
            exit(EXIT_FAILURE);
        }
        // This allocates more memory than needed, but makes the following steps easier
        buckets[i]->data = malloc(sizeof(int) * array->size);
        if (buckets[i]->data == NULL) {
            printf("Memory allocation failed for bucket[%d]->data\n", i);
            exit(EXIT_FAILURE);
        }
        buckets[i]->size = 0;
    }

    // digits
    for (int i = 1; i <= iterations; i++) {
        radix_partitioning(array, buckets, i);
        radix_collection(array, buckets);
    }

    for (int i = 0; i < NUMBER_FORMAT; i++) {
        free(buckets[i]->data);
    }
}

int sum(int* array, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += array[i];
    }
    return sum;
}

void * parallel_radix_thread(void* args) {
    thread_args_t* thread_args = (thread_args_t *) args;

    IntArray* buckets[NUMBER_FORMAT];
    for (int i = 0; i < NUMBER_FORMAT; i++) {
        buckets[i] = malloc(sizeof(IntArray));
        if (buckets[i] == NULL) {
            printf("Memory allocation failed for bucket[%d]\n", i);
            exit(EXIT_FAILURE);
        }
        // This allocates more memory than needed, but makes the following steps easier
        buckets[i]->data = malloc(sizeof(int) * thread_args->end_index - thread_args->start_index + 1);
        if (buckets[i]->data == NULL) {
            printf("Memory allocation failed for bucket[%d]->data\n", i);
            exit(EXIT_FAILURE);
        }
        buckets[i]->size = 0;
    }

    IntArray subarray;
    subarray.data = &thread_args->array->data[thread_args->start_index];
    subarray.size = thread_args->end_index - thread_args->start_index;
    for (int iteration = 1; iteration <= thread_args->iterations; iteration++) {
        if (thread_args->thread_id == 0) {
            for (int i = 0; i < NUMBER_FORMAT * NUMBER_OF_THREADS; i++)
            {
                thread_args->bucket_sizes_per_thread[i] = 0;
            }
        }
        pthread_barrier_wait(thread_args->barrier);
        radix_partitioning(&subarray, buckets, iteration);
        for (int i = 0; i < NUMBER_FORMAT; i++) {
            thread_args->bucket_sizes_per_thread[(NUMBER_OF_THREADS * i) + thread_args->thread_id] = buckets[i]->size;
        }

        pthread_barrier_wait(thread_args->barrier);

        for (int i = 0; i < NUMBER_FORMAT; i++) {
            int pos = sum(thread_args->bucket_sizes_per_thread, (NUMBER_OF_THREADS * i) + thread_args->thread_id);
            for (int j = 0; j < buckets[i]->size; j++) {
                thread_args->array->data[pos++] = buckets[i]->data[j];
            }
            buckets[i]->size = 0;
        }

        pthread_barrier_wait(thread_args->barrier);
    }

    for (int i = 0; i < NUMBER_FORMAT; i++) {
        free(buckets[i]->data);
    }

    return NULL;
}

void parallel_radix(IntArray *array) {
    int iterations = number_to_digits(max(array));

    pthread_t* threads = malloc(sizeof(pthread_t) * NUMBER_OF_THREADS);
    thread_args_t* args = malloc(sizeof(thread_args_t) * NUMBER_OF_THREADS);

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, NUMBER_OF_THREADS);

    int* bucket_sizes_per_thread = malloc(sizeof(int) * NUMBER_FORMAT * NUMBER_OF_THREADS);

    int n_array = array->size / NUMBER_OF_THREADS;
    for(int i = 0; i < NUMBER_OF_THREADS; i++) {
        args[i].array = array;
        args[i].barrier = &barrier;
        args[i].bucket_sizes_per_thread = bucket_sizes_per_thread;
        args[i].mutex = &mutex;
        args[i].iterations = iterations;
        args[i].thread_id = i;
        args[i].start_index = i* n_array;
        args[i].end_index = i == NUMBER_OF_THREADS - 1 ? array->size : (i+1) * n_array;

        pthread_create(&threads[i], NULL, parallel_radix_thread, &args[i]);
    }

    for (int i = 0; i < NUMBER_OF_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(args);
    pthread_mutex_destroy(&mutex);
    pthread_barrier_destroy(&barrier);
}

IntArray read_file(char filepath[]) {
    
    FILE *file;
    file = fopen(filepath, "r");

    if (file == NULL) {
        printf("Unable to open file '%s'", filepath);
        exit(EXIT_FAILURE);
    }

    IntArray array;
    array.data = NULL;
    array.size = 0;
    int current_data;
    while((fscanf(file, "%d ", &current_data)) != EOF) {
        if (array.size == 0) {
            array.data = (int*) malloc(sizeof(int) * ++array.size);
        } else {
            // Not the best performance. Some Java ArrayList sort of allocation might be more efficient
            array.data = (int*) realloc(array.data, sizeof(int) * ++array.size);
        }

        if (array.data == NULL) {
            printf("Allocation failed");
            exit(EXIT_FAILURE);
        }

        array.data[array.size - 1] = current_data;
    }
    fclose(file);
    return array;
}

IntArray copy_array(const IntArray* source) {
    IntArray copy;
    copy.size = source->size;
    copy.data = malloc(sizeof(int) * copy.size);
    if (copy.data == NULL) {
        printf("Memory allocation failed in copy_int_array\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < copy.size; i++) {
        copy.data[i] = source->data[i];
    }

    return copy;
}

int main(int argc ,char* argv[]) {
    struct timeval tvalBefore, tvalAfter;

    
    IntArray array;
    if (argc > 1) {
        int input = atoi(argv[1]);
        int n = pow(2, input);
        if (n > 0) {
            array.data = malloc(sizeof(int) * n);
            array.size = n;

            for(int i = 0; i < n; i++) {
                array.data[i] = rand();
            }

        } else {
            exit(1);
        }
    } else {
        char filepath[] = "numbers.txt";
        array = read_file(filepath);
    }

    IntArray sequential = copy_array(&array);
    
    gettimeofday(&tvalBefore , NULL);
    radix(&sequential);
    gettimeofday(&tvalAfter, NULL);

    // print_array(sequential, NULL);

    printf("Time in microseconds for radix-sort: %ld ms\n", ((tvalAfter.tv_sec - tvalBefore.tv_sec) * 1000000L + tvalAfter.tv_usec) - tvalBefore.tv_usec);

    IntArray parallel = copy_array(&array);
    
    gettimeofday(&tvalBefore , NULL);
    parallel_radix(&parallel);
    gettimeofday(&tvalAfter, NULL);

    // print_array(parallel, NULL);

    printf("Time in microseconds for radix-sort: %ld ms\n", ((tvalAfter.tv_sec - tvalBefore.tv_sec) * 1000000L + tvalAfter.tv_usec) - tvalBefore.tv_usec);

    free(array.data);
    free(sequential.data);
    free(parallel.data);

    return EXIT_SUCCESS;
}