/**
 * @file mpi_word_ring.cpp
 * @brief Each node in an MPI ring sequentially adds a word to reconstruct a sentence.
 *
 * This program demonstrates a token-ring communication pattern where a sentence is built
 * word by word. Each process, in turn, receives the current state of the sentence,
 * adds its assigned word, and passes it to the next process.
 *
 * @version 2.0
 * @date 2025-07-03
 *
 * @par Compilation
 * @code
 * mpic++ mpi_word_ring.cpp -o word_ring
 * @endcode
 *
 * @par Execution
 * @code
 * mpirun -np <N> --hostfile <hosts> ./word_ring
 * @endcode
 */

#include <mpi.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdio>

// --- Function Prototypes ---

void check_mpi_error(int err, const char* file, int line);
static std::vector<std::string> split_words(const std::string& txt);
static std::string serialize_vector(const std::vector<std::string>& vec);
static std::vector<std::string> deserialize_string(const std::string& s);
static void send_string(const std::string& s, int dest, int tag);
static std::string recv_string(int src, int tag, MPI_Comm comm);

// --- MPI Error Utility ---

/**
 * @def MPI_CHECK(cmd)
 * @brief A macro to wrap MPI calls for automatic error checking.
 */
#define MPI_CHECK(cmd) check_mpi_error(cmd, __FILE__, __LINE__)

/**
 * @brief Checks the return value of an MPI function and aborts if it's not MPI_SUCCESS.
 * @param err The error code returned by the MPI function.
 * @param file The source file where the error occurred.
 * @param line The line number where the error occurred.
 */
void check_mpi_error(int err, const char* file, int line) {
    if (err != MPI_SUCCESS) {
        char msg[MPI_MAX_ERROR_STRING];
        int len;
        MPI_Error_string(err, msg, &len);
        std::fprintf(stderr, "MPI error at %s:%d - %s\n", file, line, msg);
        MPI_Abort(MPI_COMM_WORLD, err);
    }
}

// --- String and Vector Helpers ---

/**
 * @brief Splits a string into a vector of words.
 * @param txt The input string to split.
 * @return A std::vector<std::string> containing the words.
 */
static std::vector<std::string> split_words(const std::string& txt) {
    std::istringstream iss(txt);
    std::vector<std::string> words;
    std::string w;
    while (iss >> w) words.push_back(w);
    return words;
}

/**
 * @brief Serializes a vector of strings into a single newline-delimited string.
 * @param vec The vector of strings to serialize.
 * @return A single string containing all vector elements separated by '\\n'.
 */
static std::string serialize_vector(const std::vector<std::string>& vec) {
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        oss << vec[i] << (i == vec.size() - 1 ? "" : "\n");
    }
    return oss.str();
}

/**
 * @brief Deserializes a newline-delimited string into a vector of strings.
 * @param s The serialized string.
 * @return A vector containing the separated strings.
 */
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

// --- MPI Communication Wrappers ---

/**
 * @brief Sends a std::string to a destination process.
 *
 * First sends the length of the string, then the string data itself.
 * @param s The string to send.
 * @param dest The rank of the destination process.
 * @param tag The message tag.
 */
static void send_string(const std::string& s, int dest, int tag) {
    int len = static_cast<int>(s.size());
    MPI_CHECK(MPI_Send(&len, 1, MPI_INT, dest, tag, MPI_COMM_WORLD));
    if (len > 0) {
        MPI_CHECK(MPI_Send(s.c_str(), len, MPI_CHAR, dest, tag, MPI_COMM_WORLD));
    }
}

/**
 * @brief Receives a std::string from a source process.
 *
 * First receives the length of the string, then the string data.
 * @param src The rank of the source process.
 * @param tag The message tag.
 * @param comm The MPI communicator.
 * @return The received string.
 */
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


// --- Main Program ---

/**
 * @brief The main entry point of the program.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return 0 on success, 1 on error.
 */
int main(int argc, char** argv) {
    MPI_CHECK(MPI_Init(&argc, &argv));
    int rank, size;
    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &size));

    // --- 1. GATHER HOSTNAMES FROM ALL NODES ---
    char proc_name_char[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_CHECK(MPI_Get_processor_name(proc_name_char, &name_len));
    
    // Buffer to receive all hostnames
    char all_names_buffer[size * MPI_MAX_PROCESSOR_NAME];

    // Each process sends its name and receives the full list of names
    MPI_CHECK(MPI_Allgather(proc_name_char, MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                            all_names_buffer, MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                            MPI_COMM_WORLD));

    // Convert the buffer into a list of strings for easy access
    std::vector<std::string> all_hostnames;
    for (int i = 0; i < size; ++i) {
        all_hostnames.push_back(std::string(&all_names_buffer[i * MPI_MAX_PROCESSOR_NAME]));
    }
    std::string my_hostname = all_hostnames[rank];

    if (size < 1) {
        MPI_Finalize();
        return 1;
    }
    
    std::vector<std::string> all_words;
    int total_words = 0;

    // The root process (rank 0) gets the sentence from the user
    if (rank == 0) {
        std::cout << "Enter the sentence:\n> ";
        std::string line;
        std::getline(std::cin, line);
        all_words = split_words(line);
        total_words = all_words.size();
    }

    // Broadcast the total number of words to all processes
    MPI_CHECK(MPI_Bcast(&total_words, 1, MPI_INT, 0, MPI_COMM_WORLD));

    // Broadcast the list of words to all processes
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
    
    // Synchronize all processes before starting the timer
    MPI_CHECK(MPI_Barrier(MPI_COMM_WORLD));
    double start_time = 0;
    if(rank == 0) start_time = MPI_Wtime();

    // --- Sequential Word-by-Word Ring Assembly ---
    std::string current_phrase;
    const int TAG_PHRASE = 201;

    for (int i = 0; i < total_words; ++i) {
        int owner_rank = i % size;
        if (rank == owner_rank) {
            // Receive the phrase from the previous process (unless this is the first word)
            if (i > 0) {
                int source_rank = (i - 1) % size;
                current_phrase = recv_string(source_rank, TAG_PHRASE, MPI_COMM_WORLD);
            }

            // Add the next word to the phrase
            std::string my_word = all_words[i];
            if (!current_phrase.empty()) {
                current_phrase += " ";
            }
            current_phrase += my_word;
            
            // Log the action and send the updated phrase to the next process
            if (i < total_words - 1) {
                int next_owner_rank = (i + 1) % size;
                std::cout << my_hostname << " adds '" << my_word << "' and sends '" << current_phrase << "' to " << all_hostnames[next_owner_rank] << std::endl;
                send_string(current_phrase, next_owner_rank, TAG_PHRASE);
            } else {
                // For the last word, log that it's being sent back to the root node (rank 0)
                std::cout << my_hostname << " adds '" << my_word << "' and sends '" << current_phrase << "' to " << all_hostnames[0] << std::endl;
            }
        }
    }

    // --- Finalization and Metrics Reporting ---
    if (total_words > 0) {
        int final_owner_rank = (total_words - 1) % size;
        if (rank == final_owner_rank) {
            // The process that added the last word reports that the ring is complete
            std::cout << "\nThe message has completed the ring and was finalized at " << my_hostname << ".\n";
            if (rank != 0) {
                // If the final owner is not the root, send the final phrase to the root
                send_string(current_phrase, 0, TAG_PHRASE);
            }
        }

        // The root process gathers the final results and prints the metrics
        if (rank == 0) {
            std::string final_phrase = current_phrase;
            // If the root was not the final owner, it must receive the final phrase
            if (final_owner_rank != 0) {
                final_phrase = recv_string(final_owner_rank, TAG_PHRASE, MPI_COMM_WORLD);
            }
            
            double total_time = MPI_Wtime() - start_time;
            
            std::cout << "\n--- Reconstructed Sentence ---\n" << final_phrase << "\n";
            std::cout << "\n--- Metrics ---\n"
                    << "Processes      : " << size << '\n'
                    << "Words          : " << total_words << '\n'
                    << "Final size     : " << final_phrase.size() << " bytes\n"
                    << "Total time     : " << total_time << " s\n"
                    << "Latency (total): " << total_time * 1000.0 << " ms\n"
                    << "Bandwidth      : " << (total_words > 0 && total_time > 0 ? (final_phrase.size() / total_time) / (1024.0 * 1024.0) : 0) << " MB/s\n";
        }
    } else if (rank == 0) {
        // Handle the case where no words were entered
        std::cout << "\nNo words were entered.\n";
    }

    MPI_CHECK(MPI_Finalize());
    return 0;
}
