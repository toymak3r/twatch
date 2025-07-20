# GitHub MCP Server Configuration

This directory contains all the configuration files, scripts, and documentation for setting up the GitHub MCP Server in Cursor for the T-Watch project.

## 📁 Directory Structure

```
mcp/
├── setup.sh                    # Main entry point
├── scripts/                    # Configuration scripts
│   ├── setup_mcp_choice.sh     # Main configuration script
│   ├── setup_github_mcp.sh     # Docker MCP setup
│   ├── setup_github_mcp_npm.sh # npm MCP setup
│   ├── setup_github_token.sh   # Token configuration
│   └── install_docker.sh       # Docker installation
├── config/                     # Configuration files
│   ├── mcp_servers.json        # Docker configuration
│   └── mcp_servers_npm.json    # npm configuration
├── docs/                       # Documentation
│   ├── MCP_SETUP.md           # Complete setup guide
│   ├── MCP_CHOICE_GUIDE.md    # Docker vs npm guide
│   └── GITHUB_MCP_SUMMARY.md  # Configuration summary
└── examples/                   # Usage examples
    └── mcp_usage_example.md   # T-Watch specific examples
```

## 🚀 Quick Start

### From Project Root
```bash
# Run the main setup script
./mcp/setup.sh
```

### From MCP Directory
```bash
# Navigate to mcp directory
cd mcp

# Run setup
./setup.sh
```

## 📋 Configuration Options

### Option 1: Docker (Recommended)
- **Official GitHub MCP Server**
- **More secure and isolated**
- **Requires Docker installation**

### Option 2: npm (Alternative)
- **Community version**
- **Simpler installation**
- **No Docker required**

## 🔧 Scripts

### Main Scripts
- **`setup.sh`** - Main entry point
- **`setup_mcp_choice.sh`** - Interactive configuration
- **`setup_github_mcp.sh`** - Docker MCP setup
- **`setup_github_mcp_npm.sh`** - npm MCP setup
- **`setup_github_token.sh`** - Token configuration
- **`install_docker.sh`** - Docker installation

## 📚 Documentation

- **`MCP_SETUP.md`** - Complete setup guide
- **`MCP_CHOICE_GUIDE.md`** - Docker vs npm comparison
- **`GITHUB_MCP_SUMMARY.md`** - Configuration summary
- **`mcp_usage_example.md`** - Usage examples

## ⚙️ Configuration Files

- **`mcp_servers.json`** - Docker MCP configuration
- **`mcp_servers_npm.json`** - npm MCP configuration

## 🎯 T-Watch Integration

This configuration is specifically designed for the T-Watch project, supporting:

- **LVGL to LovyanGFX migration**
- **ESP32-S3 development**
- **Hardware testing workflows**
- **Embedded development best practices**

## 🔗 Links

- [Official MCP Documentation](https://modelcontextprotocol.io/)
- [GitHub MCP Server](https://github.com/github/github-mcp-server)
- [Cursor MCP Guide](https://cursor.sh/docs/mcp)

## 🆘 Support

If you encounter issues:

1. Check the documentation in `docs/`
2. Verify your GitHub token permissions
3. Ensure Docker is running (if using Docker option)
4. Restart Cursor after configuration 