/*
    a)
    We split the array in nprocs (about) equal parts.
    Each thread sorts his local subarray with radixsort.
    After all threads are done the results are merged as in mergesort.
    Analysis:
    Radixsort takes O(kn), for n = input size, k = amount of radices to look at
    The merge step takes O(tn), for t = number of threads
    Total time for parallelized Algorithm: O(k*n/t) + O(tn)
    If we use t = sqrt(k) threads: O(k * n / sqrt(k)) + O(sqrt(k) n) = O(sqrt(k)n)
    meaning the optimal speedup compared to the sequential algorithm is kn / sqrt(k)n = sqrt(k) = t
    So the optimal speedup is linear in the number of threads for t = sqrt(k) threads.
    Since k = 32 for unsigned integers and radix 2 we use 6 threads.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#ifndef NUM_THREADS
    #define NUM_THREADS 6
#endif

typedef struct args{
    unsigned int* list;
    unsigned int n;
}args_t;

const char USAGE[200] = "USAGE:\n./radix_parallel -n <int >= 1> | -f <path to file>\n\tIf -n is given a list of size 2^n is filled with 2^n integers\n\tIf -f (and not -n) is given: reads the numbers from the given file\n";
const int MAX_PRINT_SIZE = 20;

void rand_init_list(unsigned int* const list, const unsigned int n);
void read_list(char* file_name, unsigned int** list, unsigned int *n);
void print_list(const unsigned int* const list, const unsigned int n);
void radix_base(unsigned int* const list, const unsigned int n);
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

    struct timeval start, end;
    gettimeofday(&start, NULL);
    radix_base(list, n);
    gettimeofday(&end, NULL);

    if (n <= MAX_PRINT_SIZE)  {
        printf("sorted list:\n\t");
        print_list(list, n);
        printf("\n");
    }
    printf("Is the resulting list sorted: %s\n", check_sort(list, n) ? "yes" : "no");

    const long long elapsed_time_usec = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
    //long long elapsed_time_msec = elapsed_time_usec / 1000;
    printf("Time spent: %lld us\n", elapsed_time_usec);

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
void radix_base(unsigned int* const list, const unsigned int n) {
    unsigned int start_indices[NUM_THREADS];
    unsigned int end_indices[NUM_THREADS];

    unsigned int step = n / NUM_THREADS;
    for (int i = 0; i < NUM_THREADS; i++) {
        start_indices[i] = i * step;
        end_indices[i] = (i+1) * step + ((i == NUM_THREADS - 1) ? n % NUM_THREADS : 0);
    }

    pthread_t threads[NUM_THREADS];
    args_t args[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].list = list + start_indices[i];
        args[i].n = end_indices[i] - start_indices[i];
        pthread_create(&threads[i], NULL, radix_thread, &args[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // merge results
    unsigned int* temp_list = malloc(sizeof(unsigned int) * n);
    for (int i = 0; i < n; i++) {
        unsigned int min = 1 << 31;
        unsigned int index = 0;
        for (int thread = 0; thread < NUM_THREADS; thread++) {
            if (list[start_indices[thread]] < min && start_indices[thread] < end_indices[thread]) {
                min = list[start_indices[thread]];
                index = thread;
            }
        }
        start_indices[index]++;
        temp_list[i] = min;
    }

    memcpy(list, temp_list, sizeof(unsigned int) * n);
    free(temp_list);
}

void* radix_thread(void* _args) {
    args_t* args = (args_t*) _args;
    radix(args->list, args->n);
    return NULL;
}

// uses radix sort (with radix 2) to sort the n first elements of the list.
// the list must contain at least n elements
void radix(unsigned int* const list, const unsigned int n) {
    unsigned int* zeroes = malloc(sizeof(unsigned int) * n);
    unsigned int* ones = malloc(sizeof(unsigned int) * n);

    unsigned int number_bits = sizeof(unsigned int) * 8;
    for (int current_bit = 0; current_bit < number_bits; current_bit++) {
        unsigned int index_zeroes = 0;
        unsigned int index_ones = 0;

        // bucketing
        for (int i = 0; i < n; i++) {
            if ((list[i] >> current_bit) & 0x1) {
                ones[index_ones] = list[i];
                index_ones++;
            }
            else {
                zeroes[index_zeroes] = list[i];
                index_zeroes++;
            }
        }

        // collecting
        unsigned int index_list = 0;
        for (int i = 0; i < index_zeroes; i++) {
            list[index_list] = zeroes[i];
            index_list++;
        }
        for (int i = 0; i < index_ones; i++) {
            list[index_list] = ones[i];
            index_list++;
        }
    }

    free(zeroes);
    free(ones);
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