scp ring.cpp mpi@node02:~/uss-patagon-cluster/examples/msg_ring
scp ring.cpp mpi@node03:~/uss-patagon-cluster/examples/msg_ring
scp ring.cpp mpi@node04:~/uss-patagon-cluster/examples/msg_ring

mpic++ ring.cpp -o ring 
echo "node01 ok"
ssh node02 mpic++ ~/uss-patagon-cluster/examples/msg_ring/ring.cpp -o ~/uss-patagon-cluster/examples/msg_ring/ring 
echo "node02 ok"
ssh node03 mpic++ ~/uss-patagon-cluster/examples/msg_ring/ring.cpp -o ~/uss-patagon-cluster/examples/msg_ring/ring 
echo "node03 ok"
ssh node04 mpic++ ~/uss-patagon-cluster/examples/msg_ring/ring.cpp -o ~/uss-patagon-cluster/examples/msg_ring/ring
echo "node04 ok"
