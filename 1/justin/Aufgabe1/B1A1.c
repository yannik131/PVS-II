#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>

const char USAGE[200] = "USAGE:\n./B1A1 <path to input file>\nIf no input file is given a random list with 10 elements (0-99) is generated\n";
const int MAX_PRINT_SIZE = 20;

void rand_init_list(unsigned int* const list, const unsigned int n);
void read_list(char* file_name, unsigned int** list, unsigned int *n);
void print_list(const unsigned int* const list, const unsigned int n);
void radix(unsigned int* const list, const unsigned int n);
bool check_sort(const unsigned int* const list, const unsigned int n);

int main(int argc, char** argv) {
    // list containing the elements to sort
    unsigned int* list = NULL;
    // length of the list
    unsigned int n = 10;

    if (argc == 1) {
        srand(time(NULL));
        list = malloc(sizeof(unsigned int) * n);
        rand_init_list(list, n);
    }
    else if (argc == 2) {
        read_list(argv[1], &list, &n);
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
    radix(list, n);
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
        list[i] = (float) rand() / (float) RAND_MAX * 100;
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
        printf("%d, ", list[i]);
    }
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

        // collectiing
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