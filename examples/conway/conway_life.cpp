#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <ctime>

const int ROWS = 200;
const int COLS = 200;
const int GENERATIONS = 1000;

using Grid = std::vector<std::vector<int>>;

// Colores ANSI
const std::string GREEN = "\033[32m";
const std::string BLACK = "\033[30m";
const std::string RESET = "\033[0m";

// Imprime el tablero en consola con colores
void printGrid(const Grid& grid) {
    system("clear"); // Usa "cls" en Windows
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            if (grid[i][j] == 1) {
                std::cout << GREEN << "█" << RESET; // Vivo = verde
            } else {
                std::cout << BLACK << "█" << RESET; // Muerto = negro
            }
        }
        std::cout << '\n';
    }
}

// Cuenta vecinos vivos
int countAliveNeighbors(const Grid& grid, int x, int y) {
    int count = 0;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < ROWS && ny >= 0 && ny < COLS)
                count += grid[nx][ny];
        }
    }
    return count;
}

// Calcula la siguiente generación
Grid nextGeneration(const Grid& current) {
    Grid next = current;
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            int alive = countAliveNeighbors(current, i, j);
            if (current[i][j] == 1) {
                next[i][j] = (alive == 2 || alive == 3) ? 1 : 0;
            } else {
                next[i][j] = (alive == 3) ? 1 : 0;
            }
        }
    }
    return next;
}

// Inicializa una región aleatoria centrada
void initRandomCentered(Grid& grid, int size = 10, float density = 0.3f) {
    int startRow = ROWS / 2 - size / 2;
    int startCol = COLS / 2 - size / 2;

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            grid[startRow + i][startCol + j] = (static_cast<float>(rand()) / RAND_MAX < density) ? 1 : 0;
        }
    }
}

int main() {
    srand(time(nullptr)); // Semilla para aleatoriedad
    Grid grid(ROWS, std::vector<int>(COLS, 0));
    initRandomCentered(grid); // Iniciar con patrón aleatorio centrado

    for (int gen = 0; gen < GENERATIONS; ++gen) {
        printGrid(grid);
        std::cout << "Generación: " << gen << "\n";
        grid = nextGeneration(grid);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    return 0;
}
