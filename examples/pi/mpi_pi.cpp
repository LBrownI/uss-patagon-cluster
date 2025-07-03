/**
 * @file mpi_pi.c
 * @brief Cálculo paralelo de PI usando el método del rectángulo e MPI.
 */

#include <stdio.h>
#include <mpi.h>

/**
 * @brief Función principal del programa.
 *
 * Calcula una aproximación del número PI utilizando el método del rectángulo
 * y paralelismo con MPI. Divide el intervalo [0,1] en subintervalos que son
 * repartidos entre los procesos, y luego reduce los resultados para obtener
 * la aproximación final.
 *
 * @param argc Número de argumentos de línea de comandos.
 * @param argv Vector de argumentos de línea de comandos.
 * @return int Código de salida del programa (0 si es exitoso).
 */
int main(int argc, char* argv[]) {
    int n = 10000000;           /**< Número total de subintervalos para la integral. */
    int rank, size, i;
    double h, x, sum = 0.0, total_sum = 0.0;
    double start_total, end_total, start_compute, end_compute;
    double compute_time, max_compute_time;
    double total_time, max_total_time;
    double pi = 0.0;

    // Inicializa el entorno MPI
    MPI_Init(&argc, &argv);

    // Obtiene el identificador (rank) del proceso actual
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Obtiene el número total de procesos
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Calcula el ancho de cada subintervalo
    h = 1.0 / (double)n;

    // Marca el inicio del tiempo total de ejecución
    start_total = MPI_Wtime();

    // Marca el inicio del tiempo de cómputo (solo el cálculo)
    start_compute = MPI_Wtime();

    /**
     * @brief Cada proceso calcula una parte de la integral.
     * 
     * El bucle está distribuido: cada proceso itera por índices separados por 'size'.
     */
    for (i = rank; i < n; i += size) {
        x = h * ((double)i + 0.5);        /**< Punto medio del subintervalo i */
        sum += 4.0 / (1.0 + x * x);       /**< Contribución a la suma parcial */
    }

    // Fin del tiempo de cómputo
    end_compute = MPI_Wtime();
    compute_time = end_compute - start_compute;

    // Reduce todas las sumas parciales a total_sum en el proceso 0
    MPI_Reduce(&sum, &total_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    // Marca el fin del tiempo total
    end_total = MPI_Wtime();
    total_time = end_total - start_total;

    // Obtiene el tiempo máximo de cómputo entre todos los procesos
    MPI_Reduce(&compute_time, &max_compute_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    // Obtiene el tiempo máximo total entre todos los procesos
    MPI_Reduce(&total_time, &max_total_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    // Solo el proceso 0 imprime los resultados finales
    if (rank == 0) {
        pi = h * total_sum;
        double error = pi - 3.1415926535897932;
        double points_per_sec = n / max_total_time;

        printf("Aproximacion de pi con n=%d: %.16f\n", n, pi);
        printf("Error: %.16f\n", error);
        printf("Tiempo total de ejecucion (walltime): %.6f segundos\n", max_total_time);
        printf("Tiempo maximo de computo por proceso: %.6f segundos\n", max_compute_time);
        printf("Velocidad: %.2f puntos/segundo\n", points_per_sec);
    }

    // Finaliza el entorno MPI
    MPI_Finalize();
    return 0;
}
