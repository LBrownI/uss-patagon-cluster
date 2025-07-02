#include <mpi.h>
#include <iostream>
#include <vector>
#include <unistd.h>

using namespace std;

const int BLOCK_SIZE = 4;

void xorBlocks(vector<int>& result, const vector<int>& a, const vector<int>& b) {
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        result[i] = a[i] ^ b[i];
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    vector<int> data(BLOCK_SIZE);

    if (rank == 0) {
        // Crear bloques de datos para cada nodo (excepto el nodo 0 que es maestro)
        int dataNodes = size - 1;
        vector<vector<int>> blocks(dataNodes, vector<int>(BLOCK_SIZE));

        // Asignar datos arbitrarios
        for (int i = 0; i < dataNodes; ++i) {
            for (int j = 0; j < BLOCK_SIZE; ++j) {
                blocks[i][j] = (i + 1) * 10 + j;
            }
        }

        // Calcular la paridad con XOR de todos los bloques
        vector<int> parity(BLOCK_SIZE, 0);
        for (const auto& block : blocks) {
            xorBlocks(parity, parity, block);
        }

        // Enviar cada bloque a su nodo correspondiente
        for (int i = 0; i < dataNodes; ++i) {
            MPI_Send(blocks[i].data(), BLOCK_SIZE, MPI_INT, i + 1, 0, MPI_COMM_WORLD);
        }

        // Enviar paridad al último nodo
        MPI_Send(parity.data(), BLOCK_SIZE, MPI_INT, size - 1, 1, MPI_COMM_WORLD);
    }

    // Todos los nodos diferentes del maestro (rank != 0)
    if (rank > 0) {
        MPI_Recv(data.data(), BLOCK_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cout << "Nodo " << hostname << " (rank " << rank << ") recibió datos: ";
        for (int val : data) cout << val << " ";
        cout << endl;
    }

    // Simular recuperación en nodo 0 si falla un nodo arbitrario (por ejemplo, rank 2)
    int failed_rank = 2;

    if (rank == 0) {
        cout << "\nSimulando falla del nodo " << failed_rank << "..." << endl;

        vector<int> recovered(BLOCK_SIZE, 0);
        vector<int> parity(BLOCK_SIZE, 0);

        // Recibir la paridad del último nodo
        MPI_Recv(parity.data(), BLOCK_SIZE, MPI_INT, size - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Recuperar datos haciendo XOR de todos menos el fallado
        for (int i = 1; i < size; ++i) {
            if (i == failed_rank) continue;

            vector<int> temp(BLOCK_SIZE);
            MPI_Recv(temp.data(), BLOCK_SIZE, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            xorBlocks(recovered, recovered, temp);
        }

        xorBlocks(recovered, recovered, parity);

        cout << "Datos recuperados del nodo " << failed_rank << ": ";
        for (int val : recovered) cout << val << " ";
        cout << endl;
    } else if (rank != failed_rank) {
        // Enviar datos a nodo 0 para recuperación
        MPI_Send(data.data(), BLOCK_SIZE, MPI_INT, 0, 2, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
