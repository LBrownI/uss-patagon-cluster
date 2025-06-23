
cmake_minimum_required(VERSION 3.10)

project(MpiHelloWorld CXX)

find_package(MPI REQUIRED)

add_executable(mpi_hello mpi_hello_world.cpp)

target_link_libraries(mpi_hello PRIVATE MPI::MPI_CXX)