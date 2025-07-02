/******************************************************************
 * mpi_text_roundrobin_safe.cpp
 *
 * • Distribuye las palabras en round-robin entre TODOS los procesos.
 * • Da las vueltas necesarias para reconstruir la frase completa.
 * • Muestra, paso a paso, quién recibe y quién envía.
 *
 *  Compilación:
 *      mpic++ -O2 -std=c++17 mpi_text_roundrobin_safe.cpp -o text_rr
 *  Ejecución:
 *      mpirun -np 4 --hostfile hostfile ./text_rr
 ******************************************************************/
#include <mpi.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdio>

/* ---------- Utilidades de comprobación ---------- */
void chk(int e,const char*f,int l){
    if(e!=MPI_SUCCESS){
        char msg[MPI_MAX_ERROR_STRING]; int n{};
        MPI_Error_string(e,msg,&n);
        std::fprintf(stderr,"MPI-ERR %s:%d -> %s\n",f,l,msg);
        MPI_Abort(MPI_COMM_WORLD,e);
    }
}
#define MPI_CH(cmd) chk(cmd,__FILE__,__LINE__)

/* ---------- String helpers ---------- */
std::vector<std::string> split(const std::string& s){
    std::istringstream iss{s}; std::vector<std::string> v; std::string w;
    while(iss>>w) v.push_back(w); return v;
}
void send_str(const std::string& s,int dst,int tag){
    int len = static_cast<int>(s.size());
    MPI_CH(MPI_Send(&len,1,MPI_INT,dst,tag,MPI_COMM_WORLD));
    if(len>0)
        MPI_CH(MPI_Send(s.data(),len,MPI_CHAR,dst,tag,MPI_COMM_WORLD));
}
std::string recv_str(int src,int tag){
    MPI_Status st; int len{};
    MPI_CH(MPI_Recv(&len,1,MPI_INT,src,tag,MPI_COMM_WORLD,&st));
    std::string s(len, '\0');
    if(len>0)
        MPI_CH(MPI_Recv(s.data(),len,MPI_CHAR,src,tag,MPI_COMM_WORLD,&st));
    return s;
}

/* ================================================= */
int main(int argc,char**argv){
    MPI_CH(MPI_Init(&argc,&argv));
    int rank,size; MPI_CH(MPI_Comm_rank(MPI_COMM_WORLD,&rank));
    MPI_CH(MPI_Comm_size(MPI_COMM_WORLD,&size));
    if(size<2){ if(rank==0) std::cerr<<"Se requieren ≥2 procesos\n";
                MPI_Finalize(); return 1; }

    constexpr int TAG_DIST = 100, TAG_RING = 200;
    std::vector<std::string> my_words;

    /* 1. Root lee la frase y reparte en round-robin */
    if(rank==0){
        std::cout<<"Ingresa la frase:\n> ";
        std::string frase; std::getline(std::cin,frase);
        auto words = split(frase);

        for(int r=0;r<size;++r){
            std::string bucket;
            for(std::size_t i=r;i<words.size();i+=size){
                if(!bucket.empty()) bucket.push_back(' ');
                bucket += words[i];
            }
            if(r==0) my_words = split(bucket);
            else     send_str(bucket,r,TAG_DIST);
        }
    }else{
        auto bucket = recv_str(0,TAG_DIST);
        if(!bucket.empty()) my_words = split(bucket);
    }

    /* 2. Todo el mundo conoce cuántas palabras hay en total */
    std::size_t local_cnt = my_words.size();
    std::size_t total_words{};
    MPI_CH(MPI_Allreduce(&local_cnt,&total_words,1,
                         MPI_UNSIGNED_LONG_LONG,MPI_SUM,MPI_COMM_WORLD));
    std::size_t cycles = (total_words + size - 1) / size;

    /* 3. Preparativos para el anillo */
    std::size_t next_idx = 0;                       // Próxima palabra propia
    const int dest = (rank + 1) % size;
    const int src  = (rank - 1 + size) % size;

    MPI_CH(MPI_Barrier(MPI_COMM_WORLD));
    double t0 = (rank==0) ? MPI_Wtime() : 0.0;

    std::string assembled;                          // root acumula aquí

    for(std::size_t c=0; c<cycles; ++c){
        std::string token;
        if(rank==0){
            if(next_idx < my_words.size()) token = my_words[next_idx++];
            send_str(token,dest,TAG_RING);
            token = recv_str(src,TAG_RING);

            /* guardamos la parte recibida en ensamblado global */
            if(!assembled.empty() && !token.empty()) assembled.push_back(' ');
            assembled += token;
        }
        else{
            token = recv_str(src,TAG_RING);
            std::cout<<"Rank "<<rank<<" recibe de "<<src
                     <<" : '"<<token<<"'\n";

            if(next_idx < my_words.size()){
                if(!token.empty()) token.push_back(' ');
                token += my_words[next_idx++];
            }

            std::cout<<"Rank "<<rank<<" envía  a "<<dest
                     <<" : '"<<token<<"'\n";
            send_str(token,dest,TAG_RING);
        }
    }

        /* 4. Resultados en root */
    if(rank==0){
        double elapsed = MPI_Wtime() - t0;
        std::cout<<"\n--- Frase reconstruida ---\n"<<assembled<<"\n";
        std::cout<<"\n--- Métricas ---\nProcesos  : "<<size
                 <<"\nPalabras   : "<<total_words
                 <<"\nTiempo [s] : "<<elapsed
                 <<"\nLatencia   : "<<elapsed*1000<<" ms (todas las vueltas)"
                 <<"\nBW aprox   : "
                 << (assembled.size()/elapsed)/(1024.0*1024.0) << " MB/s\n";
    }

    /* 5. Cierre de MPI y salida limpia */
    MPI_CH(MPI_Finalize());
    return 0;
}
