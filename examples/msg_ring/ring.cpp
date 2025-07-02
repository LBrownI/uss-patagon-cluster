/**
 * @file mpi_text_ring.cpp
 * @brief Distribuye palabras entre nodos y reconstruye la frase en un anillo MPI.
 *
 * Compilación:  mpic++ mpi_text_ring.cpp -o text_ring
 * Ejecución   :  mpirun -np <N> --hostfile <hosts> ./text_ring
 * (N ≥ 2; el proceso 0 actúa como coordinador).
 */

#include <mpi.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cctype>

// ---------- Utilidad de chequeo ----------
void check_mpi_error(int err, const char* file, int line) {
    if (err != MPI_SUCCESS) {
        char msg[MPI_MAX_ERROR_STRING];
        int len;
        MPI_Error_string(err, msg, &len);
        std::fprintf(stderr, "MPI error at %s:%d - %s\n", file, line, msg);
        MPI_Abort(MPI_COMM_WORLD, err);
    }
}
#define MPI_CHECK(cmd) check_mpi_error(cmd, __FILE__, __LINE__)

// ---------- Helpers ----------
static std::vector<std::string> split_words(const std::string& txt) {
    std::istringstream iss(txt);
    std::vector<std::string> words;
    std::string w;
    while (iss >> w) words.push_back(w);
    return words;
}
static std::string join(const std::vector<std::string>& v, size_t beg, size_t end) {
    std::ostringstream oss;
    for (size_t i = beg; i < end; ++i) {
        if (i != beg) oss << ' ';
        oss << v[i];
    }
    return oss.str();
}
static void send_string(const std::string& s, int dest, int tag) {
    int len = static_cast<int>(s.size());
    MPI_CHECK(MPI_Send(&len, 1, MPI_INT, dest, tag, MPI_COMM_WORLD));
    if (len)
        MPI_CHECK(MPI_Send(s.data(), len, MPI_CHAR, dest, tag, MPI_COMM_WORLD));
}
static std::string recv_string(int src, int tag) {
    MPI_Status st;
    int len;
    MPI_CHECK(MPI_Recv(&len, 1, MPI_INT, src, tag, MPI_COMM_WORLD, &st));
    std::string buf(len, '\0');
    if (len)
        MPI_CHECK(MPI_Recv(buf.data(), len, MPI_CHAR, src, tag, MPI_COMM_WORLD, &st));
    return buf;
}

// ---------- Programa principal ----------
int main(int argc, char** argv) {
    MPI_CHECK(MPI_Init(&argc, &argv));
    int rank, size;
    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &size));

    if (size < 2) {
        if (rank == 0)
            std::cerr << "Se requieren al menos 2 procesos.\n";
        MPI_Finalize();
        return 1;
    }

    std::string my_segment;
    const int TAG_DIST = 100;
    const int TAG_RING = 200;

    // ---------- 1. Root pide texto y reparte ----------
    if (rank == 0) {
        std::cout << "Ingresa la frase a distribuir:\n> ";
        std::string line;
        std::getline(std::cin, line);
        auto words = split_words(line);

        int workers = size - 1;
        size_t base = words.size() / workers;
        size_t extra = words.size() % workers;
        size_t idx = 0;

        for (int r = 1; r <= workers; ++r) {
            size_t count = base + (r <= static_cast<int>(extra) ? 1 : 0);
            std::string seg = (count ? join(words, idx, idx + count) : "");
            idx += count;
            send_string(seg, r, TAG_DIST);
        }
    } else {
        my_segment = recv_string(0, TAG_DIST);
    }

    // ---------- 2. Sincronización antes del anillo ----------
    MPI_CHECK(MPI_Barrier(MPI_COMM_WORLD));
    double start = 0.0;
    if (rank == 0) start = MPI_Wtime();

    // ---------- 3. Anillo de reconstrucción ----------
    int dest = (rank == size - 1) ? 0 : rank + 1;
    int source = (rank == 1) ? MPI_ANY_SOURCE : rank - 1;

    if (rank == 1) {
        // Inicia el anillo
        send_string(my_segment, dest, TAG_RING);
    } else if (rank != 0) {
        std::string frase = recv_string(source, TAG_RING);
        if (!my_segment.empty()) {
            if (!frase.empty()) frase += ' ';
            frase += my_segment;
        }
        send_string(frase, dest, TAG_RING);
    } else {
        // Root espera la frase final desde el último proceso
        std::string final_phrase = recv_string(size - 1, TAG_RING);
        double total = MPI_Wtime() - start;
        size_t bytes = final_phrase.size();

        std::cout << "\n--- Frase reconstruida ---\n" << final_phrase << "\n";
        std::cout << "\n--- Métricas ---\n"
                  << "Procesos          : " << size << '\n'
                  << "Tamaño mensaje    : " << bytes << " bytes\n"
                  << "Tiempo total      : " << total << " s\n"
                  << "Latencia (1 vuelta): " << total * 1000.0 << " ms\n"
                  << "Ancho de banda    : "
                  << (bytes / total) / (1024.0 * 1024.0) << " MB/s\n";
    }

    MPI_CHECK(MPI_Finalize());
    return 0;
}
