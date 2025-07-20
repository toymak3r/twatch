# GitHub MCP Usage Examples for T-Watch Project

This document shows how to use GitHub MCP Server features during T-Watch project development.

## Useful MCP Commands

### 1. Issue Management

#### Create a new issue for a bug
```
Create an issue for the LovyanGFX display bug with title "Fix display rendering issue" and detailed description of the problem
```

#### Update issue status
```
Mark issue #123 as resolved and add a comment explaining the solution
```

#### List open issues
```
List all open issues related to LovyanGFX
```

### 2. Pull Request Management

#### Create a PR for a new feature
```
Create a pull request for the LVGL to LovyanGFX migration feature with title "Migrate from LVGL to LovyanGFX"
```

#### Review a PR
```
Review pull request #45 and add comments about the BMA423 sensor implementation
```

### 3. Commits and Branches

#### Make commit of a completed feature
```
Make commit of LVGL to LovyanGFX migration changes with message "Complete LVGL to LovyanGFX migration"
```

#### Create a new branch
```
Create a new branch called "feature/weather-app" to develop the weather application
```

### 4. Repository Management

#### List repositories
```
List all my repositories related to ESP32 or T-Watch
```

#### Check repository status
```
Check the current status of the T-Watch repository and list the last 5 issues
```

## Workflow with MCP

### New Feature Development

1. **Create branch**:
   ```
   Create a feature/new-functionality branch
   ```

2. **Develop and test**:
   - Work on the code
   - Test on T-Watch hardware
   - Document changes

3. **Create issue for tracking**:
   ```
   Create an issue for the new functionality with title "Implement new feature" and detailed description
   ```

4. **Make commit**:
   ```
   Make commit of changes with message "Add new feature implementation"
   ```

5. **Create PR**:
   ```
   Create a pull request to merge the feature with the main branch
   ```

### Bug Fixes

1. **Identify the bug**:
   ```
   Create an issue for the bug with title "Fix bug description" and detailed description
   ```

2. **Develop fix**:
   - Implement the fix
   - Test on hardware

3. **Reference the issue**:
   ```
   Make commit of the fix referencing issue #123
   ```

4. **Close the issue**:
   ```
   Close issue #123 with comment explaining the fix
   ```

## Cursor Integration

### Direct Commands in Chat

You can use direct commands in Cursor chat:

- "Create an issue for the WiFi problem"
- "List the last 5 open issues"
- "Make commit of LVGL migration changes"
- "Create a PR for the new sensor feature"

### Workflow Automation

MCP can automate various tasks:

- **Automatic commits** after completing features
- **Issue creation** for bugs found
- **Status updates** of issues
- **Progress reports** generation

## T-Watch Specific Examples

### LVGL → LovyanGFX Migration

```
Create an issue for the complete LVGL to LovyanGFX migration with:
- Title: "Complete LVGL to LovyanGFX migration"
- Description: "Migrate all display code from LVGL to LovyanGFX for better performance and compatibility"
- Labels: ["migration", "display", "high-priority"]
```

### App Development

```
Create a branch for developing the music app:
- Name: feature/music-app
- Base: develop
- Description: "Music player app for T-Watch using LovyanGFX"
```

### Sensor Bug Fix

```
Create an issue for the BMA423 sensor bug:
- Title: "Fix BMA423 accelerometer readings"
- Description: "Accelerometer readings are inconsistent, need to investigate and fix"
- Labels: ["bug", "sensor", "hardware"]
```

## Usage Tips

1. **Be specific**: Use clear titles and descriptions
2. **Use labels**: Organise issues with appropriate labels
3. **Reference issues**: Always reference issues in commits
4. **Document changes**: Keep documentation updated
5. **Test on hardware**: Always test on T-Watch before committing

## Additional Resources

- [MCP Documentation](https://modelcontextprotocol.io/)
- [GitHub MCP Server](https://github.com/modelcontextprotocol/server-github)
- [Cursor MCP Guide](https://cursor.sh/docs/mcp) 