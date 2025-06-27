
cmake_minimum_required(VERSION 3.10)

project(MpiPi CXX)

find_package(MPI REQUIRED)

add_executable(mpi_pi mpi_pi.cpp)

target_link_libraries(mpi_pi PRIVATE MPI::MPI_CXX)