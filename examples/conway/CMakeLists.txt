cmake_minimum_required(VERSION 3.10)
project(conway_mpi)

find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

add_executable(conway_mpi mpi_life.cpp)
target_link_libraries(conway_mpi ${MPI_LIBRARIES})
