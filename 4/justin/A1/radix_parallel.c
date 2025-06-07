/*
    a)
    We split the array in nprocs (about) equal parts.
    Each thread creates its local ones and zeroes array.
    Once all threads are done for the step, two local arrays with the positions of the ones and zeroes for every thread are updated.
    The main thread calculates the presum, then the worker threads update the shared array.
    Since each thread operates on a different part of the shared memory, we don't need to use mutexes.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

typedef struct args {
    unsigned int start, end;
    unsigned int *list;
    unsigned int *zero_index, *ones_index;
    pthread_barrier_t *barrier_split, *barrier_presum, *barrier_done;
}args_t;

const char USAGE[200] = "USAGE:\n./radix_parallel -n <int >= 1> | -f <path to file>\n\tIf -n is given a list of size 2^n is filled with 2^n integers\n\tIf -f (and not -n) is given: reads the numbers from the given file\n";
const int MAX_PRINT_SIZE = 40;

void rand_init_list(unsigned int* const list, const unsigned int n);
void read_list(char* file_name, unsigned int** list, unsigned int *n);
void print_list(const unsigned int* const list, const unsigned int n);
void radix_base(unsigned int* const list, const unsigned int n, const unsigned int num_threads);
void* radix_thread(void* _args);
void radix(unsigned int* const list, const unsigned int n);
bool check_sort(const unsigned int* const list, const unsigned int n);

int main(int argc, char** argv) {
    // list containing the elements to sort
    unsigned int* list = NULL;
    // length of the list
    unsigned int n = 0;

    char* file = NULL;
    int c = 0;
    while ((c = getopt(argc, argv, "n:f:")) != -1) {
        switch(c) {
            case 'n':
                n = atoi(optarg);
                break;
            case 'f':
                file = optarg;
                break;
            default:
                printf("%s", USAGE);
                exit(1);
        }
    }

    if (n > 0) {
        unsigned int temp = 1 << n;
        n = temp;
        list = malloc(sizeof(unsigned int) * n);
        rand_init_list(list, n);
    }
    else if (file) {
        read_list(file, &list, &n);
    }
    else {
        printf("%s", USAGE);
        exit(1);
    }

    if (n <= MAX_PRINT_SIZE)  {
        printf("sorting list:\n\t");
        print_list(list, n);
        printf("\n");
    }
    else {
        printf("sorting list of length %d\n", n);
    }

    long num_threads = sysconf(_SC_NPROCESSORS_ONLN);

    struct timeval start, end;
    gettimeofday(&start, NULL);
    radix_base(list, n, num_threads);
    gettimeofday(&end, NULL);

    if (n <= MAX_PRINT_SIZE)  {
        printf("sorted list:\n\t");
        print_list(list, n);
        printf("\n");
    }
    printf("Is the resulting list sorted: %s\n", check_sort(list, n) ? "yes" : "no");

    const long long elapsed_time_usec = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
    const long long elapsed_time_msec = elapsed_time_usec / 1000;
    const long long elapsed_time_sec = elapsed_time_msec / 1000;
    printf("Time spent: %lld s, %lld ms, %lld us\n", elapsed_time_sec, elapsed_time_msec % 1000, elapsed_time_usec % 1000);

    free(list);
}

// list has to be allocated and have at least size n
void rand_init_list(unsigned int* const list, const unsigned int n) {
    for (int i = 0; i < n; i++) {
        list[i] = rand();
    }
}

// *list will be allocated and has to be freed afterwards
// file_name has to be the path to a valid file containing space seperated integers
void read_list(char* file_name, unsigned int** list, unsigned int *n) {
    FILE* file = fopen(file_name, "r");

    // check the amout of numbers in the file
    (*n) = 0;
    while (!feof(file)) {
        unsigned int _ignored;
        fscanf(file, "%u", &_ignored);
        (*n)++;
    }
    printf("There are %u numbers in the file '%s'.\n", *n, file_name);
    rewind(file);

    *list = malloc(sizeof(unsigned int) * (*n));
    int i = 0;
    while (!feof(file)) {
        fscanf(file, "%u", &((*list)[i]));
        i++;
    }

    fclose(file);
}

// prints the given list
// the list must contain at least n elements
void print_list(const unsigned int* const list, const unsigned int n) {
    for (int i = 0; i < n; i++) {
        printf("%d%s", list[i], (i == n - 1) ? "\n" : ", ");
    }
}

// starts NUM_THREADS threads
void radix_base(unsigned int* const list, const unsigned int n, const unsigned int num_threads) {
    pthread_t* threads = malloc(sizeof(pthread_t) * num_threads);
    args_t* args = malloc(sizeof(args_t) * num_threads);
    unsigned int* zero_indices = malloc(sizeof(unsigned int) * num_threads);
    unsigned int* ones_indices = malloc(sizeof(unsigned int) * num_threads);
    pthread_barrier_t barrier_split, barrier_presum, barrier_done;
    pthread_barrier_init(&barrier_split, NULL, num_threads + 1);
    pthread_barrier_init(&barrier_presum, NULL, num_threads + 1);
    pthread_barrier_init(&barrier_done, NULL, num_threads);
    unsigned int step = n / num_threads;
    for (int t = 0; t < num_threads; t++) {
        args[t].start = t * step;
        args[t].end = (t + 1) * step + ((t == num_threads-1) ? n % num_threads : 0);
        args[t].list = list;
        args[t].zero_index = &zero_indices[t];
        args[t].ones_index = &ones_indices[t];
        args[t].barrier_split = &barrier_split;
        args[t].barrier_presum = &barrier_presum;
        args[t].barrier_done = &barrier_done;
        pthread_create(&threads[t], NULL, radix_thread, &args[t]);
    }

    unsigned int number_bits = sizeof(unsigned int) * 8;
    for (int current_bit = 0; current_bit < number_bits; current_bit++) {
        pthread_barrier_wait(&barrier_split);
        for (int t = 1; t < num_threads; t++) {
            zero_indices[t] += zero_indices[t - 1];
        }
        ones_indices[0] += zero_indices[num_threads - 1];
        for (int t = 1; t < num_threads; t++) {
            ones_indices[t] += ones_indices[t - 1];
        }
        pthread_barrier_wait(&barrier_presum);
    }

    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }
}

void* radix_thread(void* _args) {
    args_t* args = (args_t*) _args;

    unsigned int n = args->end - args->start;
    unsigned int* my_list = malloc(sizeof(unsigned int) * n);
    unsigned int* zeroes = malloc(sizeof(unsigned int) * n);
    unsigned int* ones = malloc(sizeof(unsigned int) * n);

    unsigned int number_bits = sizeof(unsigned int) * 8;
    for (int current_bit = 0; current_bit < number_bits; current_bit++) {
        memcpy(my_list, args->list + args->start, (sizeof(unsigned int) * n));

        unsigned int num_ones = 0, num_zeroes = 0;
        for (int i = 0; i < n; i++) {
            if ((my_list[i] >> current_bit) & 0x1) {
                ones[num_ones] = my_list[i];
                num_ones++;
            }
            else {
                zeroes[num_zeroes] = my_list[i];
                num_zeroes++;
            }
        }

        // update global information
        *(args->zero_index) = num_zeroes;
        *(args->ones_index) = num_ones;

        pthread_barrier_wait(args->barrier_split);
        pthread_barrier_wait(args->barrier_presum);

        // collecting
        memcpy(args->list + (*(args->zero_index) - num_zeroes), zeroes, sizeof(unsigned int) * num_zeroes);
        memcpy(args->list + (*(args->ones_index) - num_ones), ones, sizeof(unsigned int) * num_ones);

        pthread_barrier_wait(args->barrier_done);
    }

    free(zeroes);
    free(ones);
    free(my_list);

    return NULL;
}

// returns true if the given list is sorted for the first n elements
// the list must contain at least n elements
bool check_sort(const unsigned int* const list, const unsigned int n) {
    for (int i = 0; i < n - 1; i++) {
        if (list[i] > list[i+1]) {
            return false;
        }
    }
    return true;
}