#include "util.h"
#include "vector.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief Sorts a given array with N elements using radixsort with the LSD method
 * @note 1.1 a) Sorting happens in these steps:
 * 1. For every digit 0 to 9, create a dynamic array for the numbers
 * 2. We will sort once by every digit, so we will iterate n_iteration times where n_iterations is
 * the number of digits of the largest number in the array
 * 3. Starting with the least significant digit (LSD, the first digit from the right), we iterate
 * the array and push each value to the corresponding dynamic array of the digit. This way we sort
 * by the current digit (counting phase)
 * 4. Iterate all dynamic arrays for digits 0 to 10 in order and write the values back to the
 * original array (collecting phase)
 * 5. Repeat steps 3 and 4 for all N digits to sort the entire array
 */
int *radix_sort(int *array, int N) {
    // Create the bins to put the numbers in to
    vector *vectors[10];
    for (int i = 0; i < 10; ++i)
        vectors[i] = vector_create();

    // Find the max value and make sure all values are positive
    int max_value = -1;
    for (int i = 0; i < N; ++i) {
        assert(array[i] > 0);
        if (array[i] > max_value)
            max_value = array[i];
    }

    // Execute steps 3 and 4
    int n_iterations = digit_count(max_value);
    for (int n = 0; n < n_iterations; ++n) {
        // counting phase
        for (int i = 0; i < N; ++i) {
            int digit = nth_digit(array[i], n);
            vector_push_back(vectors[digit], array[i]);
        }

        // collecting phase
        int current_index = 0;
        for (int i = 0; i < 10; ++i) {
            const int *data = vector_data(vectors[i]);
            const int size = vector_size(vectors[i]);

            for (int j = 0; j < size; ++j)
                array[current_index++] = data[j];

            vector_clear(vectors[i]);
        }
    }

    for (int i = 0; i < 10; ++i)
        vector_free(vectors[i]);

    return array;
}

void test_array_is_sorted() {
    const int N = 6;
    int sorted[] = {1, 2, 5, 7, 11, 1232};
    int not_sorted[] = {1, 5, 2, 7, 11, 1232};

    assert(array_is_sorted(sorted, N));
    assert(!array_is_sorted(not_sorted, N));
}

void test_nth_digit() {
    int number = 1234567890;
    int digits[] = {0, 9, 8, 7, 6, 5, 4, 3, 2, 1};

    for (int i = 0; i < 10; ++i)
        assert(nth_digit(number, i) == digits[i]);
}

void test_vector() {
    vector *vector = vector_create();
    for (int i = 0; i < 10; ++i) {
        vector_push_back(vector, i);
        assert(vector_size(vector) == i + 1);
    }

    for (int i = 0; i < vector_size(vector); ++i)
        assert(vector_data(vector)[i] == i);

    vector_free(vector);
}

void test_digit_count() {
    int values[] = {1, 0, 12, 123, 10000};
    int counts[] = {1, 1, 2, 3, 5};

    for (int i = 0; i < 5; ++i)
        assert(digit_count(values[i]) == counts[i]);
}

void test_read_numbers() {
    const char *path = "numbers.txt";
    vector *vector = read_numbers_from_file(path);

    assert(vector_size(vector) > 0);

    vector_free(vector);
}

void test_radix_sort() {
    const int N = 100;
    int *array = generate_random_array(N);
    radix_sort(array, N);
    assert(array_is_sorted(array, N));

    free(array);
}

void test() {
    printf("Starting tests\n");
    test_nth_digit();
    test_vector();
    test_digit_count();
    test_read_numbers();
    test_radix_sort();
    test_array_is_sorted();

    printf("All tests passed\n");
}

/**
 * @brief Reads in numbers.txt and measures the time radixsort takes to sort it
 */
void benchmark() {
    const char *path = "numbers.txt";
    vector *vector = read_numbers_from_file(path);
    int *array = vector_data(vector);

    // I prefer timespec because it supports ns precision
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    radix_sort(array, vector_size(vector));
    clock_gettime(CLOCK_MONOTONIC, &end);

    assert(array_is_sorted(array, vector_size(vector)));

    print_elapsed_time(start, end);

    vector_free(vector);
}

int main() {
    srand(time(NULL));

    test();
    benchmark();
}