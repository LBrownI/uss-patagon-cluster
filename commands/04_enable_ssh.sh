#!/bin/bash

echo "🔐 Enabling and starting SSH service..."

sudo systemctl enable ssh
sudo systemctl start ssh

echo "✅ SSH service enabled and running."
