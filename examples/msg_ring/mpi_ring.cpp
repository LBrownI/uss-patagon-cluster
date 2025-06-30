/**
 * @file mpi_ring_metrics.cpp
 * @brief A program to test MPI point-to-point communication in a ring topology.
 *
 * @details
 * This program demonstrates and measures the performance of MPI communication by
 * passing a message around a ring of processes. Each process receives a message
 * from its left neighbor (rank - 1) and sends it to its right neighbor (rank + 1).
 *
 * MPI Functionality Demonstrated:
 * - MPI_Init, MPI_Finalize: Standard MPI environment management.
 * - MPI_Comm_rank, MPI_Comm_size: Obtaining process rank and total size.
 * - MPI_Send, MPI_Recv: Core point-to-point blocking communication.
 * - MPI_Bcast: Collective communication to broadcast settings from the root.
 * - MPI_Wtime: A portable function to measure wall-clock time for performance metrics.
 * - MPI_Barrier: A synchronization point for all processes.
 *
 * The program accepts command-line arguments to control the number of rounds the
 * message circulates and the size of the message, allowing for latency and
 * bandwidth measurements.
 *
 * @author Your Name
 * @date 2025-06-30
 */

#include <iostream>
#include <vector>
#include <string>
#include <mpi.h>

/**
 * @brief Checks the return code of an MPI function and aborts on error.
 * @param error_code The integer return code from an MPI function.
 * @param file The name of the file where the error occurred (__FILE__).
 * @param line The line number where the error occurred (__LINE__).
 */
void check_mpi_error(int error_code, const char* file, int line) {
    if (error_code != MPI_SUCCESS) {
        char error_string[MPI_MAX_ERROR_STRING];
        int length_of_error_string;
        MPI_Error_string(error_code, error_string, &length_of_error_string);
        fprintf(stderr, "MPI Error at %s:%d: %s\n", file, line, error_string);
        MPI_Abort(MPI_COMM_WORLD, error_code);
    }
}

// Macro to easily wrap MPI calls for error checking
#define MPI_CHECK(cmd) check_mpi_error(cmd, __FILE__, __LINE__)

/**
 * @brief Main function to execute the MPI ring communication test.
 *
 * @param argc Argument count.
 * @param argv Argument vector. Expects CLI options for rounds and message size.
 * -r, --rounds   : Number of times the message circulates the ring.
 * -s, --size     : Size of the message in bytes.
 * @return int Exit code.
 */
int main(int argc, char** argv) {
    MPI_CHECK(MPI_Init(&argc, &argv));

    int rank, size;
    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &size));

    if (size < 2) {
        if (rank == 0) {
            std::cerr << "Error: This program requires at least 2 processes." << std::endl;
        }
        MPI_Finalize();
        return 1;
    }
    
    // --- CLI Parameter Parsing (on root) and Broadcasting ---
    int num_rounds = 10;
    int message_size = 1; // Default to 1 byte

    if (rank == 0) {
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if ((arg == "-r" || arg == "--rounds") && i + 1 < argc) {
                num_rounds = std::stoi(argv[++i]);
            } else if ((arg == "-s" || arg == "--size") && i + 1 < argc) {
                message_size = std::stoi(argv[++i]);
            }
        }
    }

    // Broadcast the settings from root to all other processes
    MPI_CHECK(MPI_Bcast(&num_rounds, 1, MPI_INT, 0, MPI_COMM_WORLD));
    MPI_CHECK(MPI_Bcast(&message_size, 1, MPI_INT, 0, MPI_COMM_WORLD));
    
    // --- Ring Communication Logic ---
    int dest = (rank + 1) % size;
    int source = (rank - 1 + size) % size;
    std::vector<char> message(message_size, 'A');
    double start_time, end_time, total_time;

    MPI_CHECK(MPI_Barrier(MPI_COMM_WORLD)); // Synchronize before starting timer

    if (rank == 0) {
        start_time = MPI_Wtime();
    }

    for (int i = 0; i < num_rounds; ++i) {
        if (rank == 0) {
            MPI_CHECK(MPI_Send(message.data(), message_size, MPI_CHAR, dest, 0, MPI_COMM_WORLD));
            MPI_CHECK(MPI_Recv(message.data(), message_size, MPI_CHAR, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE));
        } else {
            MPI_CHECK(MPI_Recv(message.data(), message_size, MPI_CHAR, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE));
            MPI_CHECK(MPI_Send(message.data(), message_size, MPI_CHAR, dest, 0, MPI_COMM_WORLD));
        }
    }

    if (rank == 0) {
        end_time = MPI_Wtime();
        total_time = end_time - start_time;

        // --- Performance Metrics Reporting ---
        double total_bytes = static_cast<double>(num_rounds) * message_size * size;
        double bandwidth_mbps = (total_bytes / total_time) / (1024.0 * 1024.0);
        double latency_ms_per_round = (total_time / num_rounds) * 1000.0;

        std::cout << "\n--- Ring Communication Performance ---" << std::endl;
        std::cout << "Processes:        " << size << std::endl;
        std::cout << "Rounds:           " << num_rounds << std::endl;
        std::cout << "Message Size:     " << message_size << " bytes" << std::endl;
        std::cout << "------------------------------------" << std::endl;
        std::cout << "Total Time:       " << total_time << " seconds" << std::endl;
        std::cout << "Avg. Latency/Round: " << latency_ms_per_round << " ms" << std::endl;
        std::cout << "Bandwidth:        " << bandwidth_mbps << " MB/s" << std::endl;
        std::cout << "------------------------------------" << std::endl;
    }

    MPI_CHECK(MPI_Finalize());
    return 0;
}