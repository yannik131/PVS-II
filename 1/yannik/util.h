#ifndef UTIL_HPP
#define UTIL_HPP

#include "vector.h"

#include <stdbool.h>
#include <time.h>

/**
 * @brief Generates a random integer between min and max
 */
int rand_range(int min, int max);

/**
 * @brief Generates a random array of length N with random values larger than 0
 */
int *generate_random_array(int N);

/**
 * @returns n-th digit of the given number (from the right)
 */
int nth_digit(int number, int n);

/**
 * @returns Number of digits of the given number
 */
int digit_count(int number);

/**
 * @returns true if the array is sorted in ascending order
 */
bool array_is_sorted(int *array, int N);

/**
 * @brief Reads in space separated numbers from a file
 */
vector *read_numbers_from_file(const char *path);

/**
 * @brief Prints the elapsed time between 2 time points in a reasonable unit
 * @param start First time point
 * @param end Second time point
 */
void print_elapsed_time(struct timespec start, struct timespec end);

/**
 * Types required by the convert_to_number function
 */
typedef enum { LONG, FLOAT } conversion_type;

/**
 * Helper function for error messages
 */
const char *type_to_string(conversion_type type);

/**
 * Checks if the result of a conversion using strtol or strtof was successfull
 * @param str String containing a number (or not)
 * @param end This is the convention:
 * successful conversion - end points to null terminator
 * partial conversion - end points to first invalid character
 * no conversion - end and str are the same
 */
bool conversion_successfull(const char *str, char *end);

/**
 * Converts a given string to a float or long
 * @param str String containing a number (or not)
 * @param result Either a long or float (please)
 * @param type Desired conversion type
 */
void convert_to_number(const char *str, void *result, conversion_type type);

/**
 * @returns true if the given string is a palindrome
 */
bool is_palindrome(const char *str);

/**
 * @brief Calls abort() if the given pointer is NULL and prints a message indicating that memory
 * allocation failed
 */
void abort_on_failed_allocation(void *ptr);

#endif /* UTIL_HPP */
