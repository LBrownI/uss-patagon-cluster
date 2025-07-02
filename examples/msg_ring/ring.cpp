/**
 * @file msg_ring.cpp
 * @brief Cada nodo en un anillo MPI añade una palabra secuencialmente para reconstruir una frase.
 *
 * Compilación: mpic++ msg_ring.cpp -o ring
 * Ejecución  : mpirun -np <N> --hostfile <hosts> ./ring
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

static std::string serialize_vector(const std::vector<std::string>& vec) {
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        oss << vec[i] << (i == vec.size() - 1 ? "" : "\n");
    }
    return oss.str();
}

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

    // --- 1. OBTENER LOS NOMBRES DE TODOS LOS NODOS ---
    char proc_name_char[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_CHECK(MPI_Get_processor_name(proc_name_char, &name_len));
    
    // Buffer para recibir todos los nombres de host
    char all_names_buffer[size * MPI_MAX_PROCESSOR_NAME];

    // Cada proceso envía su nombre y recibe la lista completa de nombres
    MPI_CHECK(MPI_Allgather(proc_name_char, MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                            all_names_buffer, MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                            MPI_COMM_WORLD));

    // Convertir el buffer en una lista de strings para fácil acceso
    std::vector<std::string> all_hostnames;
    for (int i = 0; i < size; ++i) {
        all_hostnames.push_back(std::string(&all_names_buffer[i * MPI_MAX_PROCESSOR_NAME]));
    }
    std::string my_hostname = all_hostnames[rank];
    // --- FIN DEL CAMBIO 1 ---

    if (size < 1) {
        MPI_Finalize();
        return 1;
    }
    
    std::vector<std::string> all_words;
    int total_words = 0;

    if (rank == 0) {
        std::cout << "Escribe la frase:\n> ";
        std::string line;
        std::getline(std::cin, line);
        all_words = split_words(line);
        total_words = all_words.size();
    }

    MPI_CHECK(MPI_Bcast(&total_words, 1, MPI_INT, 0, MPI_COMM_WORLD));

    if (total_words > 0) {
        int serialized_len = 0;
        std::string serialized_words;
        if (rank == 0) {
            serialized_words = serialize_vector(all_words);
            serialized_len = serialized_words.length() + 1;
        }
        MPI_CHECK(MPI_Bcast(&serialized_len, 1, MPI_INT, 0, MPI_COMM_WORLD));
        char* word_buffer = new char[serialized_len];
        if (rank == 0) {
            snprintf(word_buffer, serialized_len, "%s", serialized_words.c_str());
        }
        MPI_CHECK(MPI_Bcast(word_buffer, serialized_len, MPI_CHAR, 0, MPI_COMM_WORLD));
        all_words = deserialize_string(std::string(word_buffer));
        delete[] word_buffer;
    }
    
    MPI_CHECK(MPI_Barrier(MPI_COMM_WORLD));
    double start_time = 0;
    if(rank == 0) start_time = MPI_Wtime();

    std::string current_phrase;
    const int TAG_PHRASE = 201;

    for (int i = 0; i < total_words; ++i) {
        int owner_rank = i % size;
        if (rank == owner_rank) {
            if (i > 0) {
                int source_rank = (i - 1) % size;
                current_phrase = recv_string(source_rank, TAG_PHRASE, MPI_COMM_WORLD);
            }

            std::string my_word = all_words[i];
            if (!current_phrase.empty()) {
                current_phrase += " ";
            }
            current_phrase += my_word;

            // --- 2. CORRECCIÓN DEL MENSAJE EN PANTALLA ---
            if (i < total_words - 1) {
                int next_owner_rank = (i + 1) % size;
                // Usar la lista de hostnames para mostrar el nombre del destinatario correcto
                std::cout << my_hostname << " añade '" << my_word << "' y envia '" << current_phrase << "' a " << all_hostnames[next_owner_rank] << std::endl;
                send_string(current_phrase, next_owner_rank, TAG_PHRASE);
            } else {
                // Para la última palabra, mostrar que se envía de vuelta al nodo raíz (rank 0)
                std::cout << my_hostname << " añade '" << my_word << "' y envia '" << current_phrase << "' a " << all_hostnames[0] << std::endl;
            }
            // --- FIN DEL CAMBIO 2 ---
        }
    }

    if (total_words > 0) {
        int final_owner_rank = (total_words - 1) % size;
        if (rank == final_owner_rank) {
            std::cout << "\nEl mensaje dio la vuelta completa, y terminó en " << my_hostname << ".\n";
            if (rank != 0) {
                send_string(current_phrase, 0, TAG_PHRASE);
            }
        }

        if (rank == 0) {
            std::string final_phrase = current_phrase;
            if (final_owner_rank != 0) {
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
    } else if (rank == 0) {
        std::cout << "\nNo se ingresaron palabras.\n";
    }

    MPI_CHECK(MPI_Finalize());
    return 0;
}
