scp ring2.cpp mpi@node02:~/uss-patagon-cluster/examples/msg_ring
scp ring2.cpp mpi@node03:~/uss-patagon-cluster/examples/msg_ring
scp ring2.cpp mpi@node04:~/uss-patagon-cluster/examples/msg_ring

mpic++ ring2.cpp -o ring2 
echo "node01 ok"
ssh node02 mpic++ ~/uss-patagon-cluster/examples/msg_ring/ring_2.cpp -o ~/uss-patagon-cluster/examples/msg_ring/ring2
echo "node02 ok"
ssh node03 mpic++ ~/uss-patagon-cluster/examples/msg_ring/ring_2.cpp -o ~/uss-patagon-cluster/examples/msg_ring/ring2
echo "node03 ok"
ssh node04 mpic++ ~/uss-patagon-cluster/examples/msg_ring/ring_2.cpp -o ~/uss-patagon-cluster/examples/msg_ring/ring2
echo "node04 ok"
