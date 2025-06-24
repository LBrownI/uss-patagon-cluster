#!/bin/bash

echo "installing git..."
sudo apt update
sudo apt install -y git

echo "configuring git global name and email..."
git config --global user.name "clusterpatagon"
git config --global user.email "clusterpatagon@gmail.com"

echo "git setup complete."
