#include <float.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **args) {
    int grid_size = (32 * 1024 * 1024);
    int num_time_steps = 300;
    double delta_t = 0.02;
    double conductivity_constant = 0.1;

    // Current temperature
    double *T_k = malloc(sizeof(double) * grid_size);
    // Next temperature
    double *T_kn = malloc(sizeof(double) * grid_size);

    // Setting the initial values
    T_k[0] = 1. / 2.;
    for (int i = 1; i < grid_size; i++)
        T_k[i] = 3.59 * T_k[i - 1] * (1 - T_k[i - 1]);

    // Time loop
    for (int k = 0; k < num_time_steps; k++) {
        // System loop
        // Computes the next temperature for each grid cell
        for (int i = 0; i < grid_size; i++) {
            // Computing the temporal derivate of the ith grid cell
            double dTdt_i = conductivity_constant * (-2 * T_k[i] + T_k[i != 0 ? i - 1 : 1] +
                                                     T_k[i != grid_size - 1 ? i + 1 : i - 1]);

            // Using explicit Euler method to compute the next temperature
            // of the ith grid cell
            T_kn[i] = T_k[i] + delta_t * dTdt_i;
        }

        // Swapping T_kn and T_k
        double *temp = T_kn;
        T_kn = T_k;
        T_k = temp;
    }

    // Computing statistics of the final temperature of the grid

    double T_max = DBL_MIN;
    double T_min = DBL_MAX;
    double T_average = 0;

    for (int i = 0; i < grid_size; i++) {
        T_max = T_max > T_k[i] ? T_max : T_k[i];
        T_min = T_min < T_k[i] ? T_min : T_k[i];
    }

    for (int i = 0; i < grid_size; i++)
        T_average += T_k[i];

    T_average = T_average / grid_size;

    printf("T_max: %f, T_min: %f, T_average: %f", T_max, T_min, T_average);
    return 0;
}