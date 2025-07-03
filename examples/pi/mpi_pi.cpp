#include <stdio.h>
#include <mpi.h>

int main(int argc, char* argv[]) {
    int n = 10000000; // Número total de subintervalos para aproximar pi
    int rank, size, i;
    double h, x, sum = 0.0, total_sum = 0.0;
    double start_total, end_total, start_compute, end_compute;
    double compute_time, max_compute_time;
    double total_time, max_total_time;
    double pi = 0.0;

    // Inicializa el entorno MPI
    MPI_Init(&argc, &argv);

    // Obtiene el "rank" (identificador) de este proceso
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Obtiene el número total de procesos que están participando
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Calcula el ancho de cada subintervalo
    h = 1.0 / (double)n;

    // Marca el inicio del tiempo total de ejecución
    start_total = MPI_Wtime();

    // Marca el inicio del tiempo de cómputo (solo la parte del cálculo)
    start_compute = MPI_Wtime();

    // Cada proceso calcula una parte de la suma
    // El for está distribuido: cada proceso toma un valor de i distinto, saltando de size en size
    for (i = rank; i < n; i += size) {
        x = h * ((double)i + 0.5);        // Calcula el punto medio del subintervalo
        sum += 4.0 / (1.0 + x * x);       // Suma parcial para este proceso
    }

    // Marca el fin del tiempo de cómputo
    end_compute = MPI_Wtime();
    compute_time = end_compute - start_compute; // Tiempo que este proceso tardó en calcular su parte

    // Reduce (suma) todas las sumas parciales en total_sum en el proceso 0
    MPI_Reduce(&sum, &total_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    // Marca el fin del tiempo total de ejecución para este proceso
    end_total = MPI_Wtime();
    total_time = end_total - start_total;

    // Reduce (busca el máximo) del tiempo de cómputo entre todos los procesos y lo guarda en el proceso 0
    MPI_Reduce(&compute_time, &max_compute_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    // Reduce (busca el máximo) del tiempo total entre todos los procesos y lo guarda en el proceso 0
    MPI_Reduce(&total_time, &max_total_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    // Solo el proceso 0 muestra los resultados finales
    if (rank == 0) {
        pi = h * total_sum; // Calcula la aproximación de pi
        double error = pi - 3.1415926535897932; // Calcula el error
        double points_per_sec = n / max_total_time; // Velocidad de cómputo

        printf("Aproximacion de pi con n=%d: %.16f\n", n, pi);
        printf("Error: %.16f\n", error);
        printf("Tiempo total de ejecucion (walltime): %.6f segundos\n", max_total_time);
        printf("Tiempo maximo de computo por proceso: %.6f segundos\n", max_compute_time);
        printf("Velocidad: %.2f puntos/segundo\n", points_per_sec);

        // Sugerencia sobre cómo calcular el speedup usando un solo proceso
        printf("\nPara calcular speedup, ejecuta también con 1 proceso y compara los tiempos.\n");
        printf("Speedup = Tiempo_secuencial / Tiempo_paralelo\n");
    }

    // Finaliza el entorno MPI
    MPI_Finalize();
    return 0;
}
