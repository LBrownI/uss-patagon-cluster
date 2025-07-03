#!/bin/bash

echo "starting full setup..."

bash ./commands/00_git_setup.sh
bash ./commands/01_cpp_dev_setup.sh
bash ./commands/02_mpi_install.sh
bash ./commands/03_cmake_install.sh
bash ./commands/04_enable_ssh.sh

echo "all tools installed and ready for c++ and mpi development."
