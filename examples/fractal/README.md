# Fractal

This code will save an image to disk of a fern fractal. The image is generated based on a certains mathematics functions that repeats the times that it´s definated in the code as 'iterations'. It uses mpi by giving each CPU a portion of the image.

on `~/uss-patagon-cluster/examples/fractal`, run the following command:
```bash
mpirun -np 4 --rankfile ~/uss-patagon-cluster/examples/rankfile --hostfile ~/uss-patagon-cluster/examples/hostfile ./main
```

## Script
This script will copy the main.cpp to the other nodes, and compile them. To execute it, just
```bash
./script.sh
```
