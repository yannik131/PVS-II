#ifndef VECTOR_H
#define VECTOR_H

/**
 * @brief Simple vector class for dynamic arrays
 */
typedef struct vector vector;

/**
 * @brief Creates a vector of size 0
 */
vector *vector_create();

/**
 * @brief Frees the memory of the vector
 */
void vector_free(vector *vector);

/**
 * @brief Appends a value to the back of the vector
 */
void vector_push_back(vector *vector, int number);

/**
 * @return Number of elements in the vector
 */
int vector_size(const vector *vector);

/**
 * @returns The underlying array used in the vector
 */
int *vector_data(vector *vector);

/**
 * @brief Resets the size of the vector to 0
 */
void vector_clear(vector *vector);

#endif /* VECTOR_H */