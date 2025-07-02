/**
 * @file mpi_text_ring_verbose.cpp
 *
 * Demuestra:
 *   1. Distribución de palabras desde el root.
 *   2. Reconstrucción en anillo, mostrando cada hop:
 *        "Rank A → Rank B : '<frase_parcial>'"
 *
 * Compilación:  mpic++ mpi_text_ring_verbose.cpp -o text_ring_verbose
 * Ejecución   :  mpirun -np <N> --hostfile hostfile ./text_ring_verbose
 */

#include <mpi.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdio>

void check(int e, const char* f, int l) {
    if (e != MPI_SUCCESS) {
        char msg[MPI_MAX_ERROR_STRING]; int len;
        MPI_Error_string(e, msg, &len);
        std::fprintf(stderr, "MPI error @%s:%d  %s\n", f, l, msg);
        MPI_Abort(MPI_COMM_WORLD, e);
    }
}
#define MPI_CH(cmd) check(cmd, __FILE__, __LINE__)

// ---------------- Helpers ----------------
std::vector<std::string> split(const std::string& s) {
    std::istringstream iss(s); std::vector<std::string> v; std::string w;
    while (iss >> w) v.push_back(w);
    return v;
}
std::string join(const std::vector<std::string>& v, size_t b, size_t e) {
    std::ostringstream oss;
    for (size_t i=b; i<e; ++i) { if (i!=b) oss << ' '; oss << v[i]; }
    return oss.str();
}
void send_str(const std::string& s, int dst, int tag) {
    int len = static_cast<int>(s.size());
    MPI_CH(MPI_Send(&len, 1, MPI_INT, dst, tag, MPI_COMM_WORLD));
    if (len) MPI_CH(MPI_Send(s.data(), len, MPI_CHAR, dst, tag, MPI_COMM_WORLD));
}
std::string recv_str(int src, int tag) {
    MPI_Status st; int len;
    MPI_CH(MPI_Recv(&len, 1, MPI_INT, src, tag, MPI_COMM_WORLD, &st));
    std::string s(len, '\0');
    if (len) MPI_CH(MPI_Recv(s.data(), len, MPI_CHAR, src, tag, MPI_COMM_WORLD, &st));
    return s;
}

// ---------------- Programa ----------------
int main(int argc, char** argv) {
    MPI_CH(MPI_Init(&argc, &argv));
    int rank, size; MPI_CH(MPI_Comm_rank(MPI_COMM_WORLD,&rank));
    MPI_CH(MPI_Comm_size(MPI_COMM_WORLD,&size));
    if (size < 2){ if(rank==0) std::cerr<<"Se requieren ≥2 procesos\n"; MPI_Finalize(); return 1; }

    constexpr int TAG_DIST = 100, TAG_RING = 200;
    std::string seg;            // bloque propio
    if (rank==0) {
        std::cout<<"Escribe la frase:\n> "; std::string line; std::getline(std::cin,line);
        auto w = split(line);
        int workers = size-1;  size_t base=w.size()/workers, extra=w.size()%workers, idx=0;
        for(int r=1;r<=workers;++r){
            size_t cnt = base + (r<=extra?1:0);
            send_str(join(w,idx,idx+cnt), r, TAG_DIST);
            idx+=cnt;
        }
    } else {
        seg = recv_str(0,TAG_DIST);
    }

    MPI_CH(MPI_Barrier(MPI_COMM_WORLD));
    double t0 = (rank==0)?MPI_Wtime():0;

    int dest = (rank==size-1)?0:rank+1;
    int src  = (rank==1)?MPI_ANY_SOURCE:rank-1;

    if (rank==1) {                      // inicia anillo
        std::cout<<"Rank 1 inicia con: '"<<seg<<"' → Rank "<<dest<<std::endl; // [DEBUG]
        send_str(seg,dest,TAG_RING);
    }
    else if (rank!=0) {
        std::string frase = recv_str(src,TAG_RING);
        std::cout<<"Rank "<<rank<<" recibe de "<<src<<": '"<<frase<<"'\n";     // [DEBUG]

        if(!seg.empty()){ if(!frase.empty()) frase+=' '; frase+=seg; }
        std::cout<<"Rank "<<rank<<" envía  a "<<dest<<": '"<<frase<<"'\n";     // [DEBUG]
        send_str(frase,dest,TAG_RING);
    }
    else {                               // root recibe final
        std::string final = recv_str(size-1,TAG_RING);
        double total = MPI_Wtime()-t0;

        std::cout<<"\n--- Frase final ---\n"<<final<<"\n";
        std::cout<<"\n--- Métricas ---\nProcesos   : "<<size
                 <<"\nBytes      : "<<final.size()
                 <<"\nTiempo [s] : "<<total
                 <<"\nLatencia   : "<<total*1000<<" ms/vuelta"
                 <<"\nBW         : "<<(final.size()/total)/(1024*1024)<<" MB/s\n";
    }

    MPI_CH(MPI_Finalize());
    return 0;
}
