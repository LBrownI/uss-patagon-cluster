scp main.cpp mpi@node02:~/uss-patagon-cluster/examples/msg_ring
scp main.cpp mpi@node03:~/uss-patagon-cluster/examples/msg_ring
scp main.cpp mpi@node04:~/uss-patagon-cluster/examples/msg_ring

mpic++ main.cpp -o main 
echo "node01 ok"
ssh node02 mpic++ ~/uss-patagon-cluster/examples/msg_ring/ring_2.cpp -o ~/uss-patagon-cluster/examples/msg_ring/ring2
echo "node02 ok"
ssh node03 mpic++ ~/uss-patagon-cluster/examples/msg_ring/ring_2.cpp -o ~/uss-patagon-cluster/examples/msg_ring/ring2
echo "node03 ok"
ssh node04 mpic++ ~/uss-patagon-cluster/examples/msg_ring/ring_2.cpp -o ~/uss-patagon-cluster/examples/msg_ring/ring2
echo "node04 ok"
