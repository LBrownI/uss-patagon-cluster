// ring_bw.cpp
#include <mpi.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <cstring>

// CRC32 tabla estática (polinomio 0xEDB88320)
static uint32_t crc_table[256];
void init_crc32() {
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t c = i;
        for (int j = 0; j < 8; ++j)
            c = (c & 1) ? 0xEDB88320U ^ (c >> 1) : (c >> 1);
        crc_table[i] = c;
    }
}
uint32_t crc32(const uint8_t* data, size_t len, uint32_t prev = 0xFFFFFFFFU) {
    uint32_t c = prev;
    for (size_t i = 0; i < len; ++i)
        c = crc_table[(c ^ data[i]) & 0xFFU] ^ (c >> 8);
    return c;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size; MPI_Comm_rank(MPI_COMM_WORLD, &rank); MPI_Comm_size(MPI_COMM_WORLD, &size);

    // ---- CLI mínima ------------------------------------------------------
    size_t msg_size = 1 << 20;   // 1 MiB
    int iters = 100;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--size") && i + 1 < argc)   msg_size = std::stoul(argv[++i]);
        else if (!strcmp(argv[i], "--iters") && i + 1 < argc) iters = std::stoi(argv[++i]);
        else if (!strcmp(argv[i], "--help")) {
            if (rank == 0)
                printf("Uso: mpirun -np <P> ./ring_bw [--size BYTES] [--iters N]\n");
            MPI_Finalize(); return 0;
        }
    }

    // ---- Preparar buffer -------------------------------------------------
    std::vector<uint8_t> send_buf(msg_size), recv_buf(msg_size);
    std::fill(send_buf.begin(), send_buf.end(), static_cast<uint8_t>(rank));

    int next = (rank + 1) % size;
    int prev = (rank - 1 + size) % size;

    init_crc32();
    uint32_t crc_local = crc32(send_buf.data(), msg_size);  // CRC de mi bloque original

    MPI_Barrier(MPI_COMM_WORLD);           // sincronizar antes de cronometrar
    double t0 = MPI_Wtime();

    for (int it = 0; it < iters; ++it) {
        MPI_Sendrecv(send_buf.data(), msg_size, MPI_BYTE, next, 0,
                     recv_buf.data(), msg_size, MPI_BYTE, prev, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Actualizar CRC con el bloque recibido
        crc_local = crc32(recv_buf.data(), msg_size, crc_local);

        // Copiar recv → send para la siguiente vuelta
        send_buf.swap(recv_buf);
    }

    double t1 = MPI_Wtime();
    double local_time = t1 - t0;

    // ---- Métrica global ---------------------------------------------------
    double t_max;
    MPI_Reduce(&local_time, &t_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    // Reducir CRC para validar (XOR entre ranks)
    uint32_t crc_global;
    MPI_Reduce(&crc_local, &crc_global, 1, MPI_UNSIGNED, MPI_BXOR, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double mb_sent = (double)msg_size * iters / 1e6;
        double bw = mb_sent / t_max;
        printf("=== Ring bandwidth test ===\n");
        printf("  Procesos      : %d\n", size);
        printf("  Tamaño mensaje: %.2f MB\n", msg_size / 1e6);
        printf("  Iteraciones   : %d\n", iters);
        printf("  Tiempo (peor) : %.4f s\n", t_max);
        printf("  BW efectivo   : %.2f MB/s\n", bw);
        printf("  CRC global    : 0x%08X\n", crc_global);
    }

    MPI_Finalize();
    return 0;
}
