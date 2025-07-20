#!/bin/bash

# GitHub MCP Server Setup - Configuration Choice
echo "🎯 GitHub MCP Server Configuration for T-Watch"
echo "=============================================="
echo ""

echo "📋 Two options available:"
echo ""
echo "🐳 1. Docker (Recommended - Official)"
echo "   ✅ Official GitHub version"
echo "   ✅ More secure and isolated"
echo "   ❌ Requires Docker installation"
echo ""
echo "📦 2. npm (Alternative)"
echo "   ✅ Simpler to install"
echo "   ✅ No Docker required"
echo "   ❌ Community version"
echo ""

# Check if Docker is available
docker_available=false
if command -v docker &> /dev/null; then
    if docker info &> /dev/null; then
        docker_available=true
        echo "✅ Docker found and running"
    else
        echo "⚠️  Docker installed but not running"
    fi
else
    echo "❌ Docker not found"
fi

# Check if npm is available
npm_available=false
if command -v npm &> /dev/null; then
    npm_available=true
    echo "✅ npm found"
else
    echo "❌ npm not found"
fi

echo ""
echo "🤔 Which option do you prefer?"
echo ""

if [ "$docker_available" = true ]; then
    echo "1. Docker (Recommended)"
fi

if [ "$npm_available" = true ]; then
    echo "2. npm (Alternative)"
fi

echo "3. View detailed guide"
echo "4. Exit"
echo ""

read -p "Choose an option (1-4): " choice

case $choice in
    1)
        if [ "$docker_available" = true ]; then
            echo ""
            echo "🐳 Configuring Docker MCP Server..."
            ./scripts/setup_github_mcp.sh
        else
            echo ""
            echo "❌ Docker is not available"
            echo "   Run: ./install_docker.sh"
            exit 1
        fi
        ;;
    2)
        if [ "$npm_available" = true ]; then
            echo ""
            echo "📦 Configuring npm MCP Server..."
            ./scripts/setup_github_mcp_npm.sh
            echo ""
            echo "📋 Copying npm configuration..."
            ./scripts/copy_config.sh npm
        else
            echo ""
            echo "❌ npm is not available"
            echo "   Install Node.js and npm first"
            exit 1
        fi
        ;;
    3)
        echo ""
        echo "📖 Opening detailed guide..."
        cat ../docs/MCP_CHOICE_GUIDE.md
        echo ""
        echo "🔄 Run this script again to make your choice"
        ;;
    4)
        echo ""
        echo "👋 Configuration cancelled"
        exit 0
        ;;
    *)
        echo ""
        echo "❌ Invalid option"
        exit 1
        ;;
esac

echo ""
echo "🔑 Configuring GitHub token..."
./scripts/setup_github_token.sh

echo ""
echo "🎉 Configuration completed!"
echo ""
echo "📋 Next steps:"
echo "1. Restart Cursor"
echo "2. Test MCP functionalities"
echo "3. Check examples in mcp/examples/mcp_usage_example.md"
echo ""
echo "🔗 Documentation: mcp/docs/MCP_SETUP.md" 