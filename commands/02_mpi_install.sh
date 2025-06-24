#!/bin/bash

echo "installing openmpi and openssh..."
sudo apt update
sudo apt install -y openmpi-bin libopenmpi-dev openssh-server openssh-client

echo "mpi and ssh installation complete."
