/**
 * @file mpi_word_ring.cpp
 * @brief Cada nodo en un anillo MPI añade una palabra secuencialmente para reconstruir una frase.
 *
 * Compilación: mpic++ mpi_word_ring.cpp -o word_ring
 * Ejecución  : mpirun -np <N> --hostfile <hosts> ./word_ring
 */

#include <mpi.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdio>

// ---------- Utilidad de chequeo de errores MPI ----------
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

// ---------- Helpers para manejo de strings y vectores ----------
static std::vector<std::string> split_words(const std::string& txt) {
    std::istringstream iss(txt);
    std::vector<std::string> words;
    std::string w;
    while (iss >> w) words.push_back(w);
    return words;
}

// Serializa un vector de strings a un solo string con saltos de línea
static std::string serialize_vector(const std::vector<std::string>& vec) {
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        oss << vec[i] << (i == vec.size() - 1 ? "" : "\n");
    }
    return oss.str();
}

// Deserializa un string a un vector de strings
static std::vector<std::string> deserialize_string(const std::string& s) {
    std::vector<std::string> vec;
    if (s.empty()) return vec;
    std::istringstream iss(s);
    std::string word;
    while (std::getline(iss, word)) {
        vec.push_back(word);
    }
    return vec;
}

static void send_string(const std::string& s, int dest, int tag) {
    int len = static_cast<int>(s.size());
    MPI_CHECK(MPI_Send(&len, 1, MPI_INT, dest, tag, MPI_COMM_WORLD));
    if (len > 0) {
        MPI_CHECK(MPI_Send(s.c_str(), len, MPI_CHAR, dest, tag, MPI_COMM_WORLD));
    }
}

static std::string recv_string(int src, int tag, MPI_Comm comm) {
    MPI_Status st;
    int len;
    MPI_CHECK(MPI_Recv(&len, 1, MPI_INT, src, tag, comm, &st));
    std::string buf;
    if (len > 0) {
        buf.resize(len);
        MPI_CHECK(MPI_Recv(&buf[0], len, MPI_CHAR, src, tag, comm, &st));
    }
    return buf;
}


// ---------- Programa Principal ----------
int main(int argc, char** argv) {
    MPI_CHECK(MPI_Init(&argc, &argv));
    int rank, size;
    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &size));

    char proc_name_char[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_CHECK(MPI_Get_processor_name(proc_name_char, &name_len));
    std::string my_hostname(proc_name_char, name_len);

    if (size < 1) {
        MPI_Finalize();
        return 1;
    }
    
    std::vector<std::string> all_words;
    int total_words = 0;

    // --- 1. Root lee la frase y la transmite a todos los nodos ---
    if (rank == 0) {
        std::cout << "Escribe la frase:\n> ";
        std::string line;
        std::getline(std::cin, line);
        all_words = split_words(line);
        total_words = all_words.size();
    }

    // Transmitir el número total de palabras a todos
    MPI_CHECK(MPI_Bcast(&total_words, 1, MPI_INT, 0, MPI_COMM_WORLD));

    if (total_words > 0) {
        std::string serialized_words;
        if (rank == 0) {
            serialized_words = serialize_vector(all_words);
        }
        // Transmitir las palabras serializadas
        send_string(serialized_words, 0, 99); // Rank 0 -> Rank 0 (para simplificar)
        if(rank == 0) {
            serialized_words = recv_string(0, 99, MPI_COMM_SELF);
            for(int i = 1; i < size; ++i) {
                send_string(serialized_words, i, 99);
            }
        } else {
            serialized_words = recv_string(0, 99, MPI_COMM_WORLD);
        }
        all_words = deserialize_string(serialized_words);
    }
    
    MPI_CHECK(MPI_Barrier(MPI_COMM_WORLD));
    double start_time = 0;
    if(rank == 0) start_time = MPI_Wtime();

    // --- 2. Anillo secuencial palabra por palabra ---
    std::string current_phrase;
    const int TAG_PHRASE = 201;

    for (int i = 0; i < total_words; ++i) {
        int owner_rank = i % size;

        if (rank == owner_rank) {
            // Es mi turno. Primero, recibo la frase del anterior (si no soy el primero).
            if (i > 0) {
                int source_rank = (i - 1) % size;
                current_phrase = recv_string(source_rank, TAG_PHRASE, MPI_COMM_WORLD);
            }

            // Añado mi palabra
            std::string my_word = all_words[i];
            if (!current_phrase.empty()) {
                current_phrase += " ";
            }
            current_phrase += my_word;

            // Muestro el log y la envío al siguiente
            int next_rank_in_ring = (rank + 1) % size;
            std::cout << my_hostname << " añade '" << my_word << "' y envia '" << current_phrase << "' a node" << next_rank_in_ring << std::endl;
            
            // Si no es la última palabra, la paso al siguiente nodo del anillo
            if (i < total_words - 1) {
                int next_owner_rank = (i + 1) % size;
                send_string(current_phrase, next_owner_rank, TAG_PHRASE);
            }
        }
    }

    // --- 3. Finalización y recolección de métricas ---
    int final_owner_rank = (total_words - 1) % size;
    if (total_words > 0 && rank == final_owner_rank) {
         std::cout << "\nEl mensaje dio la vuelta completa, y terminó en " << my_hostname << ".\n";
         // Si el dueño final no es el root, debe enviarle el resultado
         if (rank != 0) {
             send_string(current_phrase, 0, TAG_PHRASE);
         }
    }

    if (rank == 0) {
        std::string final_phrase = current_phrase;
        if (total_words > 0 && final_owner_rank != 0) {
             final_phrase = recv_string(final_owner_rank, TAG_PHRASE, MPI_COMM_WORLD);
        }
        
        double total_time = MPI_Wtime() - start_time;
        
        std::cout << "\n--- Frase reconstruida ---\n" << final_phrase << "\n";
        std::cout << "\n--- Métricas ---\n"
                  << "Procesos       : " << size << '\n'
                  << "Palabras       : " << total_words << '\n'
                  << "Tamaño final   : " << final_phrase.size() << " bytes\n"
                  << "Tiempo total   : " << total_time << " s\n"
                  << "Latencia (total): " << total_time * 1000.0 << " ms\n"
                  << "Ancho de banda : " << (total_words > 0 && total_time > 0 ? (final_phrase.size() / total_time) / (1024.0 * 1024.0) : 0) << " MB/s\n";
    }

    MPI_CHECK(MPI_Finalize());
    return 0;
}
