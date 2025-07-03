#!/bin/bash

echo "installing git..."
sudo apt update
sudo apt install -y git

echo "configuring git global name and email..."
git config --global user.name "example"
git config --global user.email "example@example.example"

echo "git setup complete."
