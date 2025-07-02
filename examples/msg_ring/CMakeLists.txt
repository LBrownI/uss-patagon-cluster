cmake_minimum_required(VERSION 3.10)

project(MpiRing CXX)

find_package(MPI REQUIRED)

add_executable(ring mpi_ring.cpp)

target_link_libraries(mpi_ring PRIVATE MPI::MPI_CXX)