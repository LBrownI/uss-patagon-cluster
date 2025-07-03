#include <mpi.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <climits>

using namespace std;
using namespace cv;

const int WIDTH = 1080;
const int HEIGHT = 1920;
const int SCALE = 150;
const int TOTAL_ITER = 1000000;

/**
 * @brief Applies the first affine transformation used in the Barnsley fern.
 * 
 * @param p Input point.
 * @return Transformed point.
 */
Point2f f1(Point2f p) { return Point2f(0, 0.16f * p.y); }

/**
 * @brief Applies the second affine transformation used in the Barnsley fern.
 * 
 * @param p Input point.
 * @return Transformed point.
 */
Point2f f2(Point2f p) {
    return Point2f(0.2f * p.x - 0.26f * p.y,
                   0.23f * p.x + 0.22f * p.y + 1.6f);
}

/**
 * @brief Applies the third affine transformation used in the Barnsley fern.
 * 
 * @param p Input point.
 * @return Transformed point.
 */
Point2f f3(Point2f p) {
    return Point2f(-0.15f * p.x + 0.28f * p.y,
                   0.26f * p.x + 0.24f * p.y + 0.44f);
}

/**
 * @brief Applies the fourth affine transformation used in the Barnsley fern.
 * 
 * @param p Input point.
 * @return Transformed point.
 */
Point2f f4(Point2f p) {
    return Point2f(0.85f * p.x + 0.04f * p.y,
                   -0.04f * p.x + 0.85f * p.y + 1.6f);
}

/**
 * @brief Generates the local portion of a Barnsley fern image using a stochastic IFS method.
 * 
 * @param image Output OpenCV matrix to write the fern pixels into.
 * @param iterations Number of iterations to perform.
 * @param seed Random seed for generating different point sets.
 */
void generateFern(Mat& image, int iterations, int seed) {
    Point2f pos(0, 0);
    const int dieWalls = 100;
    srand(seed);

    for (int i = 0; i < iterations; ++i) {
        int rnd = rand() % dieWalls;
        if (rnd < 1)
            pos = f1(pos);
        else if (rnd < 8)
            pos = f2(pos);
        else if (rnd < 15)
            pos = f3(pos);
        else
            pos = f4(pos);

        int x = static_cast<int>(pos.x * SCALE + WIDTH / 2);
        int y = static_cast<int>(HEIGHT - pos.y * SCALE);

        if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
            image.at<uchar>(y, x) = 255;
        }
    }
}

/**
 * @brief Entry point. Initializes MPI, generates partial images on each rank, and reduces them into a final image.
 * 
 * @param argc Argument count.
 * @param argv Argument values.
 * @return int Exit status.
 */
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int local_iter = TOTAL_ITER / size;

    double start_time = MPI_Wtime();

    // Imagen local de cada nodo
    Mat local_image = Mat::zeros(HEIGHT, WIDTH, CV_8UC1);
    generateFern(local_image, local_iter, 1234 + rank);

    double end_time = MPI_Wtime();
    cout << "Rank " << rank << " completed in " << (end_time - start_time) << " seconds." << endl;

    // Imagen final solo en el proceso 0
    Mat global_image;
    if (rank == 0) {
        global_image = Mat::zeros(HEIGHT, WIDTH, CV_8UC1);
    }

    // Reunir las imágenes usando reducción por máximo (para binario)
    MPI_Reduce(local_image.data,
                (rank == 0 ? global_image.data : nullptr),
                WIDTH * HEIGHT, MPI_UNSIGNED_CHAR,
                MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        rotate(global_image, global_image, ROTATE_90_COUNTERCLOCKWISE);
        imwrite("Fern.png", global_image);
    }

    MPI_Finalize();
    return 0;
}
