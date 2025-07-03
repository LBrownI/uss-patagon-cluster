#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <thread>

using Grid = std::vector<std::vector<int>>;

const std::string PURPLE = "\033[35m";
const std::string WHITE = "\033[37m";
const std::string RESET = "\033[0m";

// Inicializa aleatoriamente la subgrilla local
void initGrid(Grid &grid, int local_rows, int cols) {
    for (int i = 1; i <= local_rows; ++i)
        for (int j = 0; j < cols; ++j)
            grid[i][j] = rand() % 2;
}

// Cuenta vecinos vivos (considerando filas fantasmas)
int countAliveNeighbors(const Grid &grid, int x, int y, int rows, int cols) {
    int count = 0;
    for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = (y + dy + cols) % cols;
            if (nx >= 0 && nx < rows + 2 && grid[nx][ny] == 1)
                ++count;
        }
    return count;
}

// Aplica las reglas de Conway
void updateGrid(const Grid &current, Grid &next, int local_rows, int cols) {
    for (int i = 1; i <= local_rows; ++i)
        for (int j = 0; j < cols; ++j) {
            int alive = countAliveNeighbors(current, i, j, local_rows, cols);
            next[i][j] = (current[i][j] == 1) ?
                ((alive == 2 || alive == 3) ? 1 : 0) :
                ((alive == 3) ? 1 : 0);
        }
}

// Imprime toda la grilla (proceso 0)
void printFullGrid(const Grid &local_grid, int local_rows, int cols, int rank, int size, MPI_Comm comm) {
    if (rank == 0) {
        Grid full_grid(size * local_rows, std::vector<int>(cols));
        for (int i = 0; i < local_rows; ++i)
            full_grid[i] = local_grid[i + 1];
        for (int src = 1; src < size; ++src)
            for (int i = 0; i < local_rows; ++i)
                MPI_Recv(&full_grid[src * local_rows + i][0], cols, MPI_INT, src, 0, comm, MPI_STATUS_IGNORE);

        system("clear");
        for (const auto &row : full_grid)
            for (int j = 0; j < cols; ++j)
                std::cout << (row[j] ? PURPLE + "█" + RESET : WHITE + " " + RESET), j == cols - 1 ? std::cout << "\n" : std::cout << "";
    } else {
        for (int i = 1; i <= local_rows; ++i)
            MPI_Send(&local_grid[i][0], cols, MPI_INT, 0, 0, comm);
    }
}

int main(int argc, char** argv) {
    int cols = 40, rows = 40, gens = 10;
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Leer parámetros si se pasan
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-c" && i + 1 < argc)
            cols = std::atoi(argv[++i]);
        else if (std::string(argv[i]) == "-f" && i + 1 < argc)
            rows = std::atoi(argv[++i]);
        else if (std::string(argv[i]) == "-g" && i + 1 < argc)
            gens = std::atoi(argv[++i]);
    }

    if (rows % size != 0) {
        if (rank == 0)
            std::cerr << "[!] Error: filas no divisible por cantidad de procesos.\n";
        MPI_Finalize();
        return 1;
    }

    int local_rows = rows / size;
    Grid current(local_rows + 2, std::vector<int>(cols));
    Grid next(local_rows + 2, std::vector<int>(cols));
    srand(time(NULL) + rank * 100);

    initGrid(current, local_rows, cols);

    for (int gen = 0; gen < gens; ++gen) {
        // Comunicación de filas frontera
        int up = (rank == 0) ? MPI_PROC_NULL : rank - 1;
        int down = (rank == size - 1) ? MPI_PROC_NULL : rank + 1;

        MPI_Sendrecv(&current[1][0], cols, MPI_INT, up, 0,
                     &current[local_rows + 1][0], cols, MPI_INT, down, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Sendrecv(&current[local_rows][0], cols, MPI_INT, down, 1,
                     &current[0][0], cols, MPI_INT, up, 1,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        updateGrid(current, next, local_rows, cols);
        current.swap(next);

        printFullGrid(current, local_rows, cols, rank, size, MPI_COMM_WORLD);
        if (rank == 0)
            std::cout << "\nGeneración: " << gen << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    MPI_Finalize();
    return 0;
}
