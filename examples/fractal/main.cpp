#include <iostream>
#include <vector>
#include <cstdlib>
#include <climits>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Point2f f1(Point2f p) {
    Point2f retval;
    retval.x = 0;
    retval.y = 0.16*p.y;

    return retval;
}

Point2f f2(Point2f p) {
    Point2f retval;
    retval.x = 0.2*p.x - 0.26*p.y;
    retval.y = 0.23*p.x + 0.22*p.y + 1.6;

    return retval;
}

Point2f f3(Point2f p) {
    Point2f retval;
    retval.x = -0.15*p.x + 0.28*p.y;
    retval.y = 0.26*p.x + 0.24*p.y + 0.44;

    return retval;
}

Point2f f4(Point2f p) {
    Point2f retval;
    retval.x = 0.85*p.x + 0.04*p.y;
    retval.y = -0.04*p.x + 0.85*p.y + 1.6;

    return retval;
}
// Point2f f1(Point2f p) {
//     Point2f retval;
//     retval.x = 0.5f*p.x;
//     retval.y = 0.5f*p.y;

//     return retval;
// }

// Point2f f2(Point2f p) {
//     Point2f retval;
//     retval.x = 0.5f*p.x + 0.5f;
//     retval.y = 0.5f*p.y;

//     return retval;
// }

// Point2f f3(Point2f p) {
//     Point2f retval;
//     retval.x = 0.5f*p.x + 0.25f;
//     retval.y = 0.5f*p.y + sqrt(3.f)/4;

//     return retval;
// }

// Point2i determineWindowSize(vector<Point2f> points) {
//     Point2i maximum = Point2i(INT_MIN, INT_MIN);
//     for (int i = 0; i < points.size(); ++i) {
//         int x = points.at(i).x;
//         int y = points.at(i).y;

//         maximum.x = x > maximum.x ? x : maximum.x;
//         maximum.y = x > maximum.x ? x : maximum.x;
//     }
//     return maximum;
// }

// void drawFractal(float xRule, float yRule, vector<Point2f> points, int iterations = 50000) {
//     Point2f lastPosition = Point2f(0, 0);
//     const int dieWalls = points.size();
//     Point2i upperRightCorner = determineWindowSize(points);

//     Mat image = Mat(upperRightCorner.x, upperRightCorner.y, CV_8UC1, Scalar::all(0));

//     for (int i = 0; i < iterations; ++i) {
//         int rnd = rand()%dieWalls;
//         Point2f pointTo = points.at(rnd);
//         Point2f diff = pointTo - lastPosition;
//         lastPosition.x += xRule*diff.x;
//         lastPosition.y += yRule*diff.y;

//         image.at<uchar>(lastPosition.y, lastPosition.x) = 255;
//     }
//     imshow("Fractal", image);
//     waitKey();
// }

// void drawSierpinskiIFS(int iterations = 50000) {
//     Point2f lastPosition = Point2f(0, 0);
//     const int dieWalls = 3;

//     Mat image = Mat(1080, 1920, CV_8UC1, Scalar::all(0));

//     for (int i = 0; i < iterations; ++i) {
//         int rnd = rand()%dieWalls;

//         if (rnd==0) {
//             lastPosition = f1(lastPosition);
//         }
//         if (rnd==1) {
//             lastPosition = f2(lastPosition);
//         }
//         if (rnd==2) {
//             lastPosition = f3(lastPosition);
//         }

//         image.at<uchar>(lastPosition.y*1000, lastPosition.x*1000) = 255;
//     }
//     imshow("Sierpinski IFS", image);
//     waitKey();
// }

void drawBarnsleayFernIFS(int iterations) {
    Point2f lastPosition = Point2f(0, 0);
    const int dieWalls = 100;

    Mat image = Mat(1080, 1920, CV_8UC1, Scalar::all(0));

    for (int i = 0; i < iterations; ++i) {
        int rnd = rand()%dieWalls;

        if (rnd<=1) {
            lastPosition = f1(lastPosition);
        }
        else if (rnd<=8) {
            lastPosition = f2(lastPosition);
        }
        else if (rnd<=15) {
            lastPosition = f3(lastPosition);
        }
        else {
            lastPosition = f4(lastPosition);
        }

        image.at<uchar>((lastPosition.x+3)*150, lastPosition.y*150) = 255;
    }
    imshow("Barnsley Fern IFS", image);
    waitKey();
}

int main() {

    // vector<Point2f> points;
    // points.push_back(Point2f(0, 0));
    // points.push_back(Point2f(1000, 0));
    // points.push_back(Point2f(500, 866));

    // drawFractal(0.5, 0.5, points);
    drawBarnsleayFernIFS(1000000);
    return 0;
}
