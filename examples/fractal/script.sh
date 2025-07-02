scp main.cpp mpi@node02:~/uss-patagon-cluster/examples/fractal
scp main.cpp mpi@node03:~/uss-patagon-cluster/examples/fractal
scp main.cpp mpi@node04:~/uss-patagon-cluster/examples/fractal

mpic++ main.cpp -o main `pkg-config --cflags --libs opencv4`
echo "node01 ok"
ssh node02 mpic++ ~/uss-patagon-cluster/examples/fractal/main.cpp -o ~/uss-patagon-cluster/examples/fractal/main `pkg-config --cflags --libs opencv4`
echo "node02 ok"
ssh node03 mpic++ ~/uss-patagon-cluster/examples/fractal/main.cpp -o ~/uss-patagon-cluster/examples/fractal/main `pkg-config --cflags --libs opencv4`
echo "node03 ok"
ssh node04 mpic++ ~/uss-patagon-cluster/examples/fractal/main.cpp -o ~/uss-patagon-cluster/examples/fractal/main `pkg-config --cflags --libs opencv4`
echo "node04 ok"
