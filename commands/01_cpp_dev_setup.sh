#!/bin/bash

echo "installing c++ development tools..."
sudo apt update
sudo apt install -y build-essential gdb valgrind

echo "c++ environment ready."
