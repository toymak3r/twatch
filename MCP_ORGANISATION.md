# MCP Configuration Organisation

## 📁 New Directory Structure

The GitHub MCP Server configuration has been reorganised for better structure and maintainability:

```
twatch/
├── setup_mcp.sh                    # Quick setup from project root
├── mcp_servers.json                # Active MCP configuration (auto-generated)
├── mcp/                           # MCP configuration directory
│   ├── setup.sh                   # Main entry point
│   ├── README.md                  # MCP directory documentation
│   ├── scripts/                   # Configuration scripts
│   │   ├── setup_mcp_choice.sh    # Interactive configuration
│   │   ├── setup_github_mcp.sh    # Docker MCP setup
│   │   ├── setup_github_mcp_npm.sh # npm MCP setup
│   │   ├── setup_github_token.sh  # Token configuration
│   │   ├── install_docker.sh      # Docker installation
│   │   └── copy_config.sh         # Configuration file copy utility
│   ├── config/                    # Configuration templates
│   │   ├── mcp_servers.json       # Docker configuration template
│   │   └── mcp_servers_npm.json   # npm configuration template
│   ├── docs/                      # Documentation
│   │   ├── MCP_SETUP.md          # Complete setup guide
│   │   ├── MCP_CHOICE_GUIDE.md   # Docker vs npm comparison
│   │   └── GITHUB_MCP_SUMMARY.md # Configuration summary
│   └── examples/                  # Usage examples
│       └── mcp_usage_example.md  # T-Watch specific examples
└── .github/workflows/             # GitHub Actions
    └── mcp-test.yml              # MCP integration test
```

## 🚀 Quick Start

### Option 1: From Project Root (Recommended)
```bash
./setup_mcp.sh
```

### Option 2: From MCP Directory
```bash
cd mcp
./setup.sh
```

### Option 3: Manual Configuration
```bash
# Choose your configuration type
cd mcp/scripts
./copy_config.sh docker  # or npm
```

## 📋 Key Improvements

### 1. **Organised Structure**
- All MCP-related files in dedicated `mcp/` directory
- Clear separation of scripts, configs, docs, and examples
- Easy to maintain and update

### 2. **Multiple Entry Points**
- `setup_mcp.sh` - Quick setup from project root
- `mcp/setup.sh` - Main setup from MCP directory
- Individual scripts for specific tasks

### 3. **Configuration Management**
- Template configurations in `mcp/config/`
- Automatic copying to project root
- Easy switching between Docker and npm

### 4. **Comprehensive Documentation**
- Complete setup guides
- Usage examples
- Troubleshooting information
- T-Watch specific workflows

## 🔧 Script Functions

### Main Scripts
- **`setup_mcp.sh`** - Quick entry point from project root
- **`mcp/setup.sh`** - Main setup with directory checks
- **`setup_mcp_choice.sh`** - Interactive configuration choice
- **`copy_config.sh`** - Copy configuration templates

### Setup Scripts
- **`setup_github_mcp.sh`** - Docker MCP server setup
- **`setup_github_mcp_npm.sh`** - npm MCP server setup
- **`setup_github_token.sh`** - GitHub token configuration
- **`install_docker.sh`** - Docker installation for Steam Deck

## 📚 Documentation

### Setup Guides
- **`MCP_SETUP.md`** - Complete setup instructions
- **`MCP_CHOICE_GUIDE.md`** - Docker vs npm comparison
- **`GITHUB_MCP_SUMMARY.md`** - Configuration summary

### Examples
- **`mcp_usage_example.md`** - T-Watch specific usage examples
- GitHub Actions workflow for testing

## ⚙️ Configuration Files

### Templates
- **`mcp/config/mcp_servers.json`** - Docker configuration template
- **`mcp/config/mcp_servers_npm.json`** - npm configuration template

### Active Configuration
- **`mcp_servers.json`** - Active configuration in project root (auto-generated)

## 🎯 T-Watch Integration

This organisation supports:

- **LVGL to LovyanGFX migration workflows**
- **ESP32-S3 development best practices**
- **Hardware testing integration**
- **Embedded development workflows**

## 🔄 Migration from Old Structure

If you had the old configuration:

1. **Old files are automatically moved** to the new structure
2. **Configuration is preserved** during reorganisation
3. **New entry points** provide the same functionality
4. **Enhanced documentation** for better user experience

## 🆘 Support

### Quick Help
```bash
# View setup guide
cat mcp/docs/MCP_SETUP.md

# View choice guide
cat mcp/docs/MCP_CHOICE_GUIDE.md

# View examples
cat mcp/examples/mcp_usage_example.md
```

### Manual Configuration
```bash
# Copy Docker configuration
cd mcp/scripts && ./copy_config.sh docker

# Copy npm configuration
cd mcp/scripts && ./copy_config.sh npm
```

---

**✅ Organisation complete! The MCP configuration is now properly structured and ready for use.** 