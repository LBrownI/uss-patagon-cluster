scp mpi_pi.cpp mpi@node02:~/uss-patagon-cluster/examples/pi
scp mpi_pi.cpp mpi@node03:~/uss-patagon-cluster/examples/pi
scp mpi_pi.cpp mpi@node04:~/uss-patagon-cluster/examples/pi

mpic++ mpi_pi.cpp -o mpi_pi 
echo "node01 ok"
ssh node02 mpic++ ~/uss-patagon-cluster/examples/pi/mpi_pi.cpp -o ~/uss-patagon-cluster/examples/pi/mpi_pi 
echo "node02 ok"
ssh node03 mpic++ ~/uss-patagon-cluster/examples/pi/mpi_pi.cpp -o ~/uss-patagon-cluster/examples/pi/mpi_pi 
echo "node03 ok"
ssh node04 mpic++ ~/uss-patagon-cluster/examples/pi/mpi_pi.cpp -o ~/uss-patagon-cluster/examples/pi/mpi_pi
echo "node04 ok"
