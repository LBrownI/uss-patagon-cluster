/******************************************************************
 * mpi_text_ring_sendrecv.cpp
 *
 * – Round-robin de palabras sobre todos los procesos.
 * – Cada vuelta usa MPI_Sendrecv  ➜  cero posibilidades de dead-lock.
 * – El root (rank 0) va ensamblando la frase y muestra métricas.
 ******************************************************************/
#include <mpi.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>

#define TAG_LEN   10     // para el intercambio de longitudes
#define TAG_DATA  20     // para el texto real
#define MAX_WORD  256    // long máx. de palabra individual
#define MAX_RING  4096   // buffer que viaja por el anillo

inline void die_if(int e, const char* f, int l){
    if(e!=MPI_SUCCESS){ char m[MPI_MAX_ERROR_STRING]; int n{};
        MPI_Error_string(e,m,&n); std::fprintf(stderr,"MPI %s:%d %s\n",f,l,m);
        MPI_Abort(MPI_COMM_WORLD,e); }
}
#define MPI_CH(x) die_if((x),__FILE__,__LINE__)

std::vector<std::string> split(const std::string& s){
    std::istringstream iss{s}; std::vector<std::string> v; std::string w;
    while(iss>>w) v.push_back(w); return v;
}

int main(int argc,char**argv){
    MPI_CH(MPI_Init(&argc,&argv));
    int rank,size; MPI_CH(MPI_Comm_rank(MPI_COMM_WORLD,&rank));
    MPI_CH(MPI_Comm_size(MPI_COMM_WORLD,&size));
    if(size<2){ if(rank==0) std::cerr<<"Se requieren ≥2 procesos\n";
                MPI_Finalize(); return 1; }

    /* ---------- Distribución round-robin ---------- */
    std::vector<std::string> mine;
    if(rank==0){
        std::cout<<"Ingresa la frase:\n> ";
        std::string line; std::getline(std::cin,line);
        auto w = split(line);

        for(int r=0;r<size;++r){
            std::ostringstream oss;
            for(std::size_t i=r;i<w.size();i+=size){
                if(i!=r) oss<<' ';
                oss<<w[i];
            }
            std::string payload = oss.str();
            if(r==0) mine = split(payload);
            else{
                int len = payload.size();
                MPI_CH(MPI_Send(&len,1,MPI_INT,r,TAG_LEN,MPI_COMM_WORLD));
                MPI_CH(MPI_Send(payload.data(),len,MPI_CHAR,r,TAG_DATA,MPI_COMM_WORLD));
            }
        }
    }else{
        int len;
        MPI_CH(MPI_Recv(&len,1,MPI_INT,0,TAG_LEN,MPI_COMM_WORLD,MPI_STATUS_IGNORE));
        std::string payload(len,'\0');
        MPI_CH(MPI_Recv(payload.data(),len,MPI_CHAR,0,TAG_DATA,MPI_COMM_WORLD,
                        MPI_STATUS_IGNORE));
        if(!payload.empty()) mine = split(payload);
    }

    /* ---------- Parámetros para el anillo ---------- */
    std::size_t local = mine.size(), total{};
    MPI_CH(MPI_Allreduce(&local,&total,1,MPI_UNSIGNED_LONG_LONG,MPI_SUM,
                         MPI_COMM_WORLD));
    std::size_t insert_cycles = (total + size - 1) / size; // ceil(total/size)
    std::size_t rounds        = insert_cycles + (size - 1);


    const int next = (rank+1)%size, prev = (rank-1+size)%size;
    char sendbuf[MAX_RING]{0}, recvbuf[MAX_RING]{0};
    std::size_t idx = 0;        // próxima palabra propia
    std::string assembled;      // solo root la usa

    MPI_CH(MPI_Barrier(MPI_COMM_WORLD));
    double t0 = (rank==0)?MPI_Wtime():0.0;

    /* ---------- Bucle principal ---------- */
    for(std::size_t r=0; r<rounds; ++r){
        std::string out = (idx < mine.size()) ? mine[idx++] : "";
        std::strncpy(sendbuf, out.c_str(), MAX_RING-1);
        int send_len = out.size(), recv_len = 0;

        /* Paso-1: todas las longitudes en paralelo */
        MPI_CH(MPI_Sendrecv(&send_len,1,MPI_INT,next,TAG_LEN,
                            &recv_len,1,MPI_INT,prev,TAG_LEN,
                            MPI_COMM_WORLD,MPI_STATUS_IGNORE));

        /* Paso-2: contenido real (si lo hay) */
        if(recv_len>MAX_RING) recv_len = MAX_RING;          // seguridad
        MPI_CH(MPI_Sendrecv(sendbuf,send_len?send_len:1,MPI_CHAR,next,TAG_DATA, // count≥1
                            recvbuf,recv_len?recv_len:1,MPI_CHAR,prev,TAG_DATA,
                            MPI_COMM_WORLD,MPI_STATUS_IGNORE));
        std::string in(recvbuf, recv_len);

        if(rank!=0){
            std::cout<<"Rank "<<rank<<" recibe <- "<<prev<<": '"<<in<<"'\n";
        }

        if(rank==0){
            if(!assembled.empty() && !in.empty()) assembled.push_back(' ');
            assembled += in;
        }

        if(rank!=0){
            if(idx < mine.size()){            // añadimos nuestra palabra
                if(!in.empty()) in.push_back(' ');
                in += mine[idx++];
            }
            std::strncpy(sendbuf, in.c_str(), MAX_RING-1);
            send_len = in.size();
        }else{
            /* El root, para la siguiente vuelta, enviará SU siguiente palabra
               (se prepara al comienzo de la iteración)            */
        }
    }

    MPI_CH(MPI_Barrier(MPI_COMM_WORLD));   // todos terminaron el anillo

    if(rank==0){
        double dt = MPI_Wtime() - t0;
        std::cout<<"\n--- Frase reconstruida ---\n"<<assembled<<"\n";
        std::cout<<"\n--- Métricas ---\nProcesos : "<<size
                 <<"\nPalabras  : "<<total
                 <<"\nTiempo    : "<<dt<<" s"
                 <<"\nLatencia  : "<<dt*1000<<" ms (completo)"
                 <<"\nBW aprox  : "<<(assembled.size()/dt)/(1024*1024)
                 <<" MB/s\n";
    }

    MPI_CH(MPI_Finalize());
    return 0;
}
