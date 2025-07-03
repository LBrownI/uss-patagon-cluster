/**
 * @file raidMPI.cpp
 * @brief Programa MPI que simula distribución y recuperación de bloques de datos con paridad XOR.
 *
 * Este programa implementa un sistema simple de codificación por bloques con paridad XOR
 * distribuido entre múltiples nodos usando MPI. El nodo maestro (rank 0) reparte bloques de datos 
 * y una paridad calculada al resto de nodos. Luego, simula la recuperación de un bloque fallado.
 */

#include <mpi.h>
#include <iostream>
#include <vector>
#include <unistd.h>

using namespace std;

/// Tamaño fijo del bloque de datos
const int BLOCK_SIZE = 4;

/**
 * @brief Realiza la operación XOR entre dos bloques y guarda el resultado.
 * 
 * @param result Vector donde se guarda el resultado del XOR.
 * @param a Primer bloque de datos.
 * @param b Segundo bloque de datos.
 */
void xorBlocks(vector<int>& result, const vector<int>& a, const vector<int>& b) {
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        result[i] = a[i] ^ b[i];
    }
}

/**
 * @brief Función principal del programa. Controla la inicialización, distribución, recepción y recuperación de datos.
 * 
 * @param argc Número de argumentos de línea de comandos.
 * @param argv Argumentos de línea de comandos.
 * @return int Código de retorno del programa.
 */
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); ///< Identificador del proceso
    MPI_Comm_size(MPI_COMM_WORLD, &size); ///< Número total de procesos

    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    vector<int> data(BLOCK_SIZE);

    if (rank == 0) {
        // --- NODO MAESTRO: Genera bloques de datos y calcula paridad ---

        int dataNodes = size - 1; ///< Número de nodos de datos (excluyendo el maestro)
        vector<vector<int>> blocks(dataNodes, vector<int>(BLOCK_SIZE));

        // Inicializa los bloques con valores arbitrarios
        for (int i = 0; i < dataNodes; ++i) {
            for (int j = 0; j < BLOCK_SIZE; ++j) {
                blocks[i][j] = (i + 1) * 10 + j;
            }
        }

        // Calcula la paridad XOR de todos los bloques
        vector<int> parity(BLOCK_SIZE, 0);
        for (const auto& block : blocks) {
            xorBlocks(parity, parity, block);
        }

        // Envía cada bloque a su nodo correspondiente
        for (int i = 0; i < dataNodes; ++i) {
            MPI_Send(blocks[i].data(), BLOCK_SIZE, MPI_INT, i + 1, 0, MPI_COMM_WORLD);
        }

        // Envía la paridad al último nodo
        MPI_Send(parity.data(), BLOCK_SIZE, MPI_INT, size - 1, 1, MPI_COMM_WORLD);
    }

    // --- NODOS TRABAJADORES: Reciben sus bloques de datos ---
    if (rank > 0) {
        MPI_Recv(data.data(), BLOCK_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        cout << "Nodo " << hostname << " (rank " << rank << ") recibió datos: ";
        for (int val : data) cout << val << " ";
        cout << endl;

        // El último nodo almacena la paridad y la reenvía luego
        if (rank == size - 1) {
            vector<int> parity(BLOCK_SIZE);
            MPI_Recv(parity.data(), BLOCK_SIZE, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(parity.data(), BLOCK_SIZE, MPI_INT, 0, 1, MPI_COMM_WORLD);
        }
    }

    // Sincroniza todos los procesos
    MPI_Barrier(MPI_COMM_WORLD);

    // --- SIMULACIÓN DE FALLA Y RECUPERACIÓN ---
    int failed_rank = 2; ///< Nodo fallado simulado

    if (rank == 0) {
        cout << "\nSimulando falla del nodo " << failed_rank << "..." << endl;

        vector<int> recovered(BLOCK_SIZE, 0);
        vector<int> parity(BLOCK_SIZE, 0);

        // Recibe la paridad del último nodo
        MPI_Recv(parity.data(), BLOCK_SIZE, MPI_INT, size - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Recibe todos los bloques (excepto el fallado) y aplica XOR
        for (int i = 1; i < size; ++i) {
            if (i == failed_rank) continue;

            vector<int> temp(BLOCK_SIZE);
            MPI_Recv(temp.data(), BLOCK_SIZE, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            xorBlocks(recovered, recovered, temp);
        }

        // Aplica XOR con la paridad para recuperar los datos faltantes
        xorBlocks(recovered, recovered, parity);

        cout << "Datos recuperados del nodo " << failed_rank << ": ";
        for (int val : recovered) cout << val << " ";
        cout << endl;
    } 
    else if (rank != failed_rank) {
        // Nodos válidos envían sus datos al maestro para recuperación
        MPI_Send(data.data(), BLOCK_SIZE, MPI_INT, 0, 2, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
