#!/bin/bash

# Script to install Docker on Steam Deck
echo "🐳 Installing Docker on Steam Deck..."

# Check if already installed
if command -v docker &> /dev/null; then
    echo "✅ Docker already installed!"
    docker --version
    exit 0
fi

echo "📦 Updating packages..."
sudo pacman -Syu --noconfirm

echo "📦 Installing Docker..."
sudo pacman -S docker --noconfirm

echo "🔧 Configuring Docker..."
sudo systemctl enable docker
sudo systemctl start docker

# Add user to docker group
echo "👤 Adding user to docker group..."
sudo usermod -aG docker $USER

echo ""
echo "✅ Docker installed successfully!"
echo ""
echo "🔄 To apply group changes, you need to:"
echo "   1. Log out and log in again, OR"
echo "   2. Run: newgrp docker"
echo ""
echo "🧪 To test the installation:"
echo "   docker --version"
echo "   docker run hello-world"
echo ""
echo "📋 Next steps:"
echo "   1. Log out and log in again"
echo "   2. Run: ./setup_github_mcp.sh"
echo "   3. Configure GitHub token"
echo "   4. Restart Cursor" 