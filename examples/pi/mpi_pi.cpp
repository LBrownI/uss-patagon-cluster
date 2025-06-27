#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    long num_steps = 100000000; // Total number of rectangles
    double step_width = 1.0 / static_cast<double>(num_steps);
    double sum = 0.0;
    
    double start_time, end_time;

    // Synchronize all processes before starting the timer
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        start_time = MPI_Wtime();
    }

    // Each process calculates a partial sum
    // Distribute the work  
    for (long i = rank; i < num_steps; i += size) {
        double x = (static_cast<double>(i) + 0.5) * step_width;
        sum += 4.0 / (1.0 + x * x);
    }

    double partial_pi = sum * step_width;
    double global_pi = 0.0;

    // Reduce all the partial sums into a single global sum on the root process (rank 0)
    MPI_Reduce(&partial_pi, &global_pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        end_time = MPI_Wtime();
        std::cout << "Pi calculation with " << size << " processes." << std::endl;
        std::cout << "Estimated Pi: " << std::fixed << std::setprecision(12) << global_pi << std::endl;
        std::cout << "Known Pi:     " << std::fixed << std::setprecision(12) << M_PI << std::endl;
        std::cout << "Time elapsed: " << std::fixed << std::setprecision(6) << end_time - start_time << " seconds" << std::endl;
    }

    MPI_Finalize();
    return 0;
}