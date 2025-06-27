#include <iostream>
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if(rank == 0) std::cerr << "This program requires at least 2 processes to form a ring." << std::endl;
        MPI_Finalize();
        return 1;
    }

    int token;

    // Determine the source and destination for the ring communication
    int dest = (rank + 1) % size;
    int source = (rank - 1 + size) % size;

    if (rank == 0) {
        // Process 0 starts the token
        token = 100;
        std::cout << "Process " << rank << " starting with token " << token << std::endl;
        MPI_Send(&token, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
        
        // Now it waits to receive the token back from the last process
        MPI_Recv(&token, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Process " << rank << " received the final token: " << token << std::endl;
    } else {
        // All other processes wait to receive, modify, and then send
        MPI_Recv(&token, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Process " << rank << " received token " << token << " from " << source << std::endl;
        
        // Modify the token
        token += rank;
        
        MPI_Send(&token, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
        std::cout << "Process " << rank << " sent token " << token << " to " << dest << std::endl;
    }
    
    // Barrier to ensure all output is printed before finalizing
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == 0) {
        std::cout << "\nRing communication complete." << std::endl;
    }


    MPI_Finalize();
    return 0;
}