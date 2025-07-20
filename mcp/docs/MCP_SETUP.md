# GitHub MCP Server Configuration

This document explains how to configure the GitHub MCP Server in Cursor for the T-Watch project.

## What is MCP?

MCP (Model Context Protocol) is a protocol that allows Cursor to connect to external services, such as GitHub, to provide advanced integration capabilities.

## Configuration Files

### 1. `.cursorrules`
- Defines project rules and context for Cursor
- Includes specific development guidelines for T-Watch

### 2. `mcp_servers.json`
- GitHub MCP server configuration using Docker
- Defines how Cursor should connect to GitHub

### 3. `setup_github_mcp.sh`
- Automated script to configure the environment
- Downloads Docker image and guides through token configuration

## Quick Configuration

### Option 1: Automated Script (Recommended)

```bash
./setup_github_mcp.sh
```

### Option 2: Manual Configuration

1. **Install Docker** (if not already installed):
   ```bash
   # Check if already installed
   docker --version
   
   # If not installed, visit: https://docs.docker.com/get-docker/
   ```

2. **Download GitHub MCP Server image**:
   ```bash
   docker pull ghcr.io/github/github-mcp-server
   ```

3. **Create GitHub Personal Access Token**:
   - Go to: https://github.com/settings/tokens
   - Click on "Generate new token (classic)"
   - Select the following scopes:
     - `repo` (full access to repositories)
     - `read:org` (read organisation information)
     - `read:user` (read user information)
   - Copy the generated token

4. **Configure environment variable**:
   ```bash
   # Temporary (for this session only)
   export GITHUB_TOKEN='your_token_here'
   
   # Permanent (add to ~/.bashrc)
   echo "export GITHUB_TOKEN='your_token_here'" >> ~/.bashrc
   source ~/.bashrc
   ```

## Available Features

With the GitHub MCP Server configured, you will be able to:

- **Repository Management**:
  - List repositories
  - Create new repositories
  - Clone repositories

- **Issue Management**:
  - Create issues
  - Update issues
  - Close issues
  - Add comments

- **Pull Request Management**:
  - Create PRs
  - Review PRs
  - Merge PRs

- **Commits and Branches**:
  - Make commits
  - Create branches
  - Push/pull

- **Organisations**:
  - List organisations
  - Manage members

## Configuration Verification

To verify that everything is working:

1. **Restart Cursor**
2. **Test a simple command**:
   - Try listing your repositories
   - Check if you can access GitHub information

## Troubleshooting

### Error: "GitHub token not found"
- Check if the `GITHUB_TOKEN` variable is configured
- Run: `echo $GITHUB_TOKEN`

### Error: "Docker not found"
- Install Docker: https://docs.docker.com/get-docker/
- Check installation: `docker --version`

### Error: "Docker not running"
- Start Docker: `sudo systemctl start docker`
- Check status: `docker info`

### Error: "Permission denied"
- Run the script with: `chmod +x setup_github_mcp.sh`
- Or run the commands manually

## Additional Resources

- [Official MCP Documentation](https://modelcontextprotocol.io/)
- [Official GitHub MCP Server](https://github.com/github/github-mcp-server)
- [Cursor MCP Documentation](https://cursor.sh/docs/mcp)

## Support

If you encounter problems:
1. Check if all dependencies are installed
2. Confirm that the GitHub token has the correct permissions
3. Restart Cursor after configuration
4. Consult the official MCP documentation 