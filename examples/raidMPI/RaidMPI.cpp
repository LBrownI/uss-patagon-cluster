#include <mpi.h>
#include <iostream>
#include <vector>

using namespace std;

const int BLOCK_SIZE = 4;

void xorBlocks(vector<int> &result, const vector<int> &a, const vector<int> &b) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        result[i] = a[i] ^ b[i];
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 4) {
        if (rank == 0)
            cout << "Este programa requiere exactamente 4 procesos (3 datos + 1 paridad)." << endl;
        MPI_Finalize();
        return 0;
    }

    vector<int> data(BLOCK_SIZE);

    if (rank == 0) {
        // Datos originales
        vector<int> d1 = {10, 20, 30, 40};
        vector<int> d2 = {50, 60, 70, 80};
        vector<int> d3 = {90,100,110,120};
        vector<int> parity(BLOCK_SIZE);

        // Calcular paridad: P = d1 ^ d2 ^ d3
        vector<int> temp(BLOCK_SIZE);
        xorBlocks(temp, d1, d2);
        xorBlocks(parity, temp, d3);

        // Enviar datos y paridad
        MPI_Send(d1.data(), BLOCK_SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(d2.data(), BLOCK_SIZE, MPI_INT, 2, 0, MPI_COMM_WORLD);
        MPI_Send(d3.data(), BLOCK_SIZE, MPI_INT, 3, 0, MPI_COMM_WORLD);
        MPI_Send(parity.data(), BLOCK_SIZE, MPI_INT, 3, 1, MPI_COMM_WORLD);
    }

    // Recibir datos en los nodos
    if (rank == 1 || rank == 2 || rank == 3) {
        MPI_Recv(data.data(), BLOCK_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cout << "Nodo " << rank << " recibió datos: ";
        for (int val : data) cout << val << " ";
        cout << endl;
    }

    // Simular recuperación en el nodo 0 si falla el nodo 2
    if (rank == 0) {
        cout << "\nSimulando falla del nodo 2 (rank 2)..." << endl;

        vector<int> d1(BLOCK_SIZE), d3(BLOCK_SIZE), parity(BLOCK_SIZE), recovered(BLOCK_SIZE);

        // Recibir de nodos 1 y 3
        MPI_Recv(d1.data(), BLOCK_SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(d3.data(), BLOCK_SIZE, MPI_INT, 3, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(parity.data(), BLOCK_SIZE, MPI_INT, 3, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Recuperar d2: d2 = parity ^ d1 ^ d3
        vector<int> temp(BLOCK_SIZE);
        xorBlocks(temp, parity, d1);
        xorBlocks(recovered, temp, d3);

        cout << "Datos recuperados del nodo 2: ";
        for (int val : recovered) cout << val << " ";
        cout << endl;
    } else {
        // Enviar bloques a nodo 0 para recuperación
        if (rank == 1)
            MPI_Send(data.data(), BLOCK_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD);
        if (rank == 3) {
            MPI_Send(data.data(), BLOCK_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD);
            vector<int> parity(BLOCK_SIZE);
            MPI_Recv(parity.data(), BLOCK_SIZE, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(parity.data(), BLOCK_SIZE, MPI_INT, 0, 1, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}
