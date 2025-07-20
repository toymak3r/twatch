#!/bin/bash

# Quick MCP Setup for T-Watch Project
# This script runs the main MCP configuration

echo "🎯 Quick MCP Setup for T-Watch Project"
echo "======================================"
echo ""

# Check if mcp directory exists
if [ ! -d "mcp" ]; then
    echo "❌ MCP directory not found"
    echo "   Please ensure you're in the T-Watch project root"
    exit 1
fi

# Run the main setup script
echo "🚀 Starting MCP configuration..."
./mcp/setup.sh 