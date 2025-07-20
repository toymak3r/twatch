#!/bin/bash

# Copy MCP Configuration Script
# This script copies the appropriate MCP configuration to the project root

echo "📋 Copying MCP Configuration..."

# Check if we're in the right directory
if [ ! -f "../config/mcp_servers.json" ]; then
    echo "❌ Configuration files not found"
    echo "   Please run this script from the mcp/scripts directory"
    exit 1
fi

# Get the configuration type from argument
CONFIG_TYPE=${1:-"docker"}

case $CONFIG_TYPE in
    "docker"|"d")
        echo "🐳 Copying Docker configuration..."
        cp ../config/mcp_servers.json ../../mcp_servers.json
        echo "✅ Docker configuration copied to project root"
        ;;
    "npm"|"n")
        echo "📦 Copying npm configuration..."
        cp ../config/mcp_servers_npm.json ../../mcp_servers.json
        echo "✅ npm configuration copied to project root"
        ;;
    *)
        echo "❌ Invalid configuration type: $CONFIG_TYPE"
        echo "   Use: docker, d, npm, or n"
        exit 1
        ;;
esac

echo ""
echo "📋 Next steps:"
echo "1. Restart Cursor"
echo "2. Test MCP functionality"
echo "3. Check configuration in mcp_servers.json" 