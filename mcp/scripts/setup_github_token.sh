#!/bin/bash

# Script to configure GitHub token permanently
echo "🔑 Configuring GitHub Token Permanently..."

# Check if already exists in .bashrc
if grep -q "GITHUB_TOKEN" ~/.bashrc; then
    echo "⚠️  Token already configured in .bashrc"
    echo "   To update, edit ~/.bashrc manually"
    exit 0
fi

echo ""
echo "📋 To configure the token permanently:"
echo "1. Go to https://github.com/settings/tokens"
echo "2. Click on 'Generate new token (classic)'"
echo "3. Select the scopes: repo, read:org, read:user"
echo "4. Copy the generated token"
echo ""

read -p "Enter your GitHub Personal Access Token: " token

if [ -n "$token" ]; then
    # Add to .bashrc
    echo "" >> ~/.bashrc
    echo "# GitHub MCP Server Token" >> ~/.bashrc
    echo "export GITHUB_TOKEN='$token'" >> ~/.bashrc
    
    # Configure for this session as well
    export GITHUB_TOKEN="$token"
    
    echo "✅ Token configured permanently!"
    echo "🔄 Reloading .bashrc..."
    source ~/.bashrc
    
    echo ""
    echo "🎉 Configuration completed!"
    echo "   The token will be loaded automatically in new sessions"
    echo "   To use now, restart the terminal or run: source ~/.bashrc"
else
    echo "❌ Token not provided. Configuration cancelled."
fi 