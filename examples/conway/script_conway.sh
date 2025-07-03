scp mpi_life.cpp mpi@node02:~/uss-patagon-cluster/examples/conway
scp mpi_life.cpp mpi@node03:~/uss-patagon-cluster/examples/conway
scp mpi_life.cpp mpi@node04:~/uss-patagon-cluster/examples/conway

mpic++ mpi_life.cpp -o mpi_life
echo "node01 ok"

ssh node02 mpic++ ~/uss-patagon-cluster/examples/conway/mpi_life.cpp -o ~/uss-patagon-cluster/examples/conway/mpi_life
echo "node02 ok"

ssh node03 mpic++ ~/uss-patagon-cluster/examples/conway/mpi_life.cpp -o ~/uss-patagon-cluster/examples/conway/mpi_life
echo "node03 ok"

ssh node04 mpic++ ~/uss-patagon-cluster/examples/conway/mpi_life.cpp -o ~/uss-patagon-cluster/examples/conway/mpi_life
echo "node04 ok"
