Raspberry Pi Setup Scripts
==========================

This folder contains scripts to install all tools needed for C++ and MPI development.

Included tools:
---------------
- Git
- C++ compiler (g++, gdb, valgrind)
- OpenMPI + SSH
- CMake
- SSH enabled and started

How to use:
-----------
1. Open the file: commands/00_git_setup.sh  
   → Replace "example" with your real Git name and email.

2. Give execution permissions:
   chmod +x commands/*.sh
   chmod +x 05_all_in_one.sh

3. Run the full installer:
   ./05_all_in_one.sh

This will install everything and enable SSH access.

Script list:
------------
- 00_git_setup.sh → Install Git and set user info
- 01_cpp_dev_setup.sh → Install C++ tools
- 02_mpi_install.sh → Install MPI and SSH
- 03_cmake_install.sh → Install CMake
- 04_enable_ssh.sh → Enable SSH
- 05_all_in_one.sh → Run everything
