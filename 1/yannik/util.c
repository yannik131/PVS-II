#include "util.h"
#include "vector.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// We're only sorting signed ints, so we don't need values > 2^31
static const int powers_of_10[] = {1,      10,      100,      1000,      10000,
                                   100000, 1000000, 10000000, 100000000, 1000000000};

int rand_range(int min, int max) {
    return rand() % (max - min + 1) + min;
}

int *generate_random_array(int N) {
    int *array = (int *)malloc(N * sizeof(int));

    abort_on_failed_allocation(array);

    for (int i = 0; i < N; ++i)
        array[i] = rand_range(0, 100000);

    return array;
}

int nth_digit(int number, int n) {
    // https://stackoverflow.com/a/203877
    return ((number / powers_of_10[n]) % 10);
}

bool array_is_sorted(int *array, int N) {
    for (int i = 0; i < N - 1; ++i) {
        if (array[i] > array[i + 1])
            return false;
    }

    return true;
}

int digit_count(int number) {
    int count = 1;
    while ((number /= 10) > 0)
        ++count;

    return count;
}

vector *read_numbers_from_file(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Couldn't open file %s", path);
        exit(1);
    }

    vector *vector = vector_create();
    int number;
    while (fscanf(file, "%d", &number) == 1)
        vector_push_back(vector, number);

    fclose(file);

    printf("Read in %d numbers from %s\n", vector_size(vector), path);

    return vector;
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

const char *type_to_string(conversion_type type) {
    return type == LONG ? "long" : "float";
}

bool conversion_successfull(const char *str, char *end) {
    return errno != ERANGE && str != end && *end == '\0';
}

void convert_to_number(const char *str, void *result, conversion_type type) {
    if (str == NULL || result == NULL) {
        fprintf(stderr, "Can't convert a null pointer to %s\n", type_to_string(type));
        exit(1);
    }

    char *end;
    errno = 0; // reset after potential previous call

    if (type == LONG) {
        *(long *)result = strtol(str, &end, 10);
    } else if (type == FLOAT) {
        *(float *)result = strtof(str, &end);
    } else {
        fprintf(stderr, "Unknown type for conversion given\n");
        exit(1);
    }

    if (!conversion_successfull(str, end)) {
        fprintf(stderr, "Couldn't convert \"%s\" to %s\n", str, type_to_string(type));
        exit(1);
    }
}

bool is_palindrome(const char *str) {
    int len = strlen(str);

    for (int i = 0; i < len / 2; ++i) {
        if (str[i] != str[len - 1 - i])
            return false;
    }

    return true;
}

void abort_on_failed_allocation(void *ptr) {
    if (ptr == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        abort();
    }
}
