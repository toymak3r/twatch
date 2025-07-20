#!/bin/bash

# GitHub MCP Server Setup for T-Watch Project
# Main entry point for configuration

echo "🎯 GitHub MCP Server Setup for T-Watch Project"
echo "=============================================="
echo ""

# Check if we're in the right directory
if [ ! -f "scripts/setup_mcp_choice.sh" ]; then
    echo "❌ Please run this script from the project root directory"
    echo "   cd /path/to/twatch && ./mcp/setup.sh"
    exit 1
fi

# Make all scripts executable
echo "🔧 Making scripts executable..."
chmod +x scripts/*.sh

# Run the main configuration script
echo "🚀 Starting configuration..."
./scripts/setup_mcp_choice.sh 