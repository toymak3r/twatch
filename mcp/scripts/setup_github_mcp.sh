#!/bin/bash

# GitHub MCP Server Setup Script for T-Watch Project
echo "🔧 Configuring GitHub MCP Server for T-Watch project..."

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "❌ Docker is not installed. Please install Docker first."
    echo "   Visit: https://docs.docker.com/get-docker/"
    exit 1
fi

# Check if Docker is running
if ! docker info &> /dev/null; then
    echo "❌ Docker is not running. Please start Docker first."
    exit 1
fi

echo "✅ Docker found and running"

# Pull GitHub MCP server image
echo "📦 Downloading GitHub MCP Server (Docker image)..."
docker pull ghcr.io/github/github-mcp-server

# Check if GitHub token is already set
if [ -z "$GITHUB_TOKEN" ]; then
    echo ""
    echo "🔑 To use the GitHub MCP Server, you need a Personal Access Token."
    echo ""
    echo "📋 How to create a token:"
    echo "1. Go to https://github.com/settings/tokens"
    echo "2. Click on 'Generate new token (classic)'"
    echo "3. Select the following scopes:"
    echo "   - repo (full access to repositories)"
    echo "   - read:org (read organisation information)"
    echo "   - read:user (read user information)"
    echo "4. Copy the generated token"
    echo ""
    echo "💡 To configure the token permanently, add to your ~/.bashrc:"
    echo "   export GITHUB_TOKEN='your_token_here'"
    echo ""
    echo "🔧 To configure temporarily for this session:"
    echo "   export GITHUB_TOKEN='your_token_here'"
    echo ""
    read -p "Do you want to configure the token now? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        read -p "Enter your GitHub Personal Access Token: " token
        export GITHUB_TOKEN="$token"
        echo "✅ Token configured for this session"
        echo "💡 Remember to add to ~/.bashrc for persistence"
    fi
else
    echo "✅ GitHub token already configured"
fi

echo ""
echo "🎉 GitHub MCP Server configuration completed!"
echo ""
echo "📋 Next steps:"
echo "1. Restart Cursor"
echo "2. The GitHub MCP Server will be available automatically"
echo "3. You can use commands such as:"
echo "   - List repositories"
echo "   - Create issues"
echo "   - Make commits"
echo "   - Manage pull requests"
echo ""
echo "🔗 Documentation: https://github.com/github/github-mcp-server" 