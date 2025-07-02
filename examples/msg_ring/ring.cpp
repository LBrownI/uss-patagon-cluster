/**
 * @file mpi_text_roundrobin.cpp
 *
 *  • Divide las palabras en round-robin entre *todos* los procesos.
 *  • Reconstruye la frase en el mismo orden original, dando tantas
 *    vueltas al anillo como sea necesario.
 *
 * Compilación:  mpic++ mpi_text_roundrobin.cpp -o text_rr
 * Ejecución   :  mpirun -np <N> --hostfile hostfile ./text_rr
 */

#include <mpi.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdio>

// ---------- Utilidades ----------
void chk(int e,const char*f,int l){
    if(e!=MPI_SUCCESS){char m[MPI_MAX_ERROR_STRING];int n;
        MPI_Error_string(e,m,&n);std::fprintf(stderr,"MPI %s:%d %s\n",f,l,m);
        MPI_Abort(MPI_COMM_WORLD,e);}
}
#define MPI_CH(cmd) chk(cmd,__FILE__,__LINE__)

std::vector<std::string> split(const std::string& s){
    std::istringstream iss(s);std::vector<std::string> v;std::string w;
    while(iss>>w) v.push_back(w);return v;
}
std::string join(const std::vector<std::string>& v,const char*sep=" "){
    std::ostringstream oss;for(size_t i=0;i<v.size();++i){
        if(i) oss<<sep;oss<<v[i];}return oss.str();
}
void send_str(const std::string&s,int dst,int tag){
    int len=s.size();MPI_CH(MPI_Send(&len,1,MPI_INT,dst,tag,MPI_COMM_WORLD));
    if(len) MPI_CH(MPI_Send(s.data(),len,MPI_CHAR,dst,tag,MPI_COMM_WORLD));
}
std::string recv_str(int src,int tag){
    MPI_Status st;int len;
    MPI_CH(MPI_Recv(&len,1,MPI_INT,src,tag,MPI_COMM_WORLD,&st));
    std::string s(len,'\0');
    if(len) MPI_CH(MPI_Recv(s.data(),len,MPI_CHAR,src,tag,MPI_COMM_WORLD,&st));
    return s;
}
// ----------------------------------

int main(int argc,char**argv){
    MPI_CH(MPI_Init(&argc,&argv));
    int rank,size;MPI_CH(MPI_Comm_rank(MPI_COMM_WORLD,&rank));
    MPI_CH(MPI_Comm_size(MPI_COMM_WORLD,&size));
    if(size<2){if(rank==0)std::cerr<<"Se requieren ≥2 procesos\n";
        MPI_Finalize();return 1;}

    constexpr int TAG_DIST=100,TAG_RING=200;
    std::vector<std::string> my_words;

    // ---- 1. Root lee frase y reparte en round-robin ----
    if(rank==0){
        std::cout<<"Ingresa la frase:\n> ";
        std::string line;std::getline(std::cin,line);
        auto words=split(line);

        for(int r=0;r<size;++r){
            std::vector<std::string> bucket;
            for(size_t i=r;i<words.size();i+=size) bucket.push_back(words[i]);
            std::string msg=join(bucket);
            if(r==0) my_words=bucket;
            else     send_str(msg,r,TAG_DIST);
        }
    }else{
        std::string msg=recv_str(0,TAG_DIST);
        if(!msg.empty()) my_words=split(msg);
    }

    // ---- 2. Preparación para reconstrucción ----
    size_t next_idx=0;                         // índice de la próxima palabra propia
    const int dest=(rank+1)%size;
    const int src =(rank-1+size)%size;
    MPI_CH(MPI_Barrier(MPI_COMM_WORLD));
    double t0=(rank==0)?MPI_Wtime():0;

    size_t total_words;
    if(rank==0){
        // root conoce el total
        total_words=0; for(auto &w:my_words) total_words+=w.size(); // dummy
    }
    // broadcast para saber cuántas vueltas se necesitan
    size_t my_count=my_words.size();
    size_t global_count;
    MPI_CH(MPI_Reduce(&my_count,&global_count,1,MPI_UNSIGNED_LONG_LONG,
                      MPI_SUM,0,MPI_COMM_WORLD));
    if(rank==0){
        total_words=global_count;
        MPI_CH(MPI_Bcast(&total_words,1,MPI_UNSIGNED_LONG_LONG,0,MPI_COMM_WORLD));
    }else{
        MPI_CH(MPI_Bcast(&total_words,1,MPI_UNSIGNED_LONG_LONG,0,MPI_COMM_WORLD));
    }
    size_t cycles=(total_words+size-1)/size;   // nº de vueltas

    std::string phrase;                        // solo root la usa al final
    for(size_t c=0;c<cycles;++c){
        std::string payload;                   // mensaje que circula
        if(rank==0){
            // root coloca su palabra de esta vuelta (si existe)
            if(next_idx<my_words.size()) payload=my_words[next_idx++];
            send_str(payload,dest,TAG_RING);
            payload=recv_str(src,TAG_RING);
            if(c==cycles-1) phrase=payload;   // frase completa llegará al final
        }else{
            payload=recv_str(src,TAG_RING);
            std::cout<<"Rank "<<rank<<" recibe de "<<src
                     <<" : '"<<payload<<"'\n";
            if(next_idx<my_words.size()){
                if(!payload.empty()) payload+=' ';
                payload+=my_words[next_idx++];
            }
            std::cout<<"Rank "<<rank<<" envía  a "<<dest
                     <<" : '"<<payload<<"'\n";
            send_str(payload,dest,TAG_RING);
        }
    }

    // ---- 3. Métricas y salida final ----
    if(rank==0){
        double total=MPI_Wtime()-t0;
        std::cout<<"\n--- Frase reconstruida ---\n"<<phrase<<"\n";
        std::cout<<"\n--- Métricas ---\nProcesos   : "<<size
                 <<"\nPalabras   : "<<total_words
                 <<"\nTiempo [s] : "<<total
                 <<"\nLatencia   : "<<total*1000<<" ms (todas las vueltas)"
                 <<"\nBW (aprox) : "<<(phrase.size()/total)/(1024*1024)
                 <<" MB/s\n";
    }

    MPI_CH(MPI_Finalize());
    return 0;
}
