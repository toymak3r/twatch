# Documentation Index

Complete index of all documentation files for the T-Watch S3 MAINFRAME firmware project.

## Main Documentation

### Core Documentation
- **[README.md](README.md)** — Documentation overview and quick navigation
- **[index.md](index.md)** — Full documentation index with build commands and file structure
- **[README_CN.MD](README_CN.MD)** — Chinese language documentation

## Configuration & Setup

- **[configuration/setup-guide.md](configuration/setup-guide.md)** — Development environment setup and build/flash commands

## Firmware Design References

- **[superpowers/specs/2026-06-04-twatch-mainframe-clock-design.md](superpowers/specs/2026-06-04-twatch-mainframe-clock-design.md)** — Authoritative MAINFRAME hardware & software design spec
- **[superpowers/plans/2026-06-04-twatch-mainframe-firmware.md](superpowers/plans/2026-06-04-twatch-mainframe-firmware.md)** — MAINFRAME implementation plan

## Guides

- **[guides/Quick_Start_Guide.md](guides/Quick_Start_Guide.md)** — Get up and running quickly

## API Reference

- **[api/LilyGoLib_API.md](api/LilyGoLib_API.md)** — Legacy LilyGoLib wrapper reference (kept for historical reference; wrapper has been removed)

## Battery Optimization

- **[BATTERY_OPTIMIZATION.md](BATTERY_OPTIMIZATION.md)** — General battery-saving strategies

## Troubleshooting & Support

- **[troubleshooting/troubleshooting.md](troubleshooting/troubleshooting.md)** — Comprehensive troubleshooting guide

## Hardware Documentation

### Shell Design
- **[shell/README.md](shell/README.md)** — Shell design documentation
- **[shell/BackCover.jpg](shell/BackCover.jpg)** — Back cover image
- **[shell/BackCover.stp](shell/BackCover.stp)** — Back cover 3D model
- **[shell/BackCover1.jpg](shell/BackCover1.jpg)** — Alternative back cover image
- **[shell/BackCover1.stp](shell/BackCover1.stp)** — Alternative back cover 3D model

### Schematics
- **[schematic/README.md](schematic/README.md)** — Hardware schematics documentation
- **[schematic/T_WATCH_S3.pdf](schematic/T_WATCH_S3.pdf)** — T-Watch S3 schematic
- **[schematic/T_WATCH-2020V01.pdf](schematic/T_WATCH-2020V01.pdf)** — T-Watch 2020 V01 schematic
- **[schematic/T_WATCH-2020V02.pdf](schematic/T_WATCH-2020V02.pdf)** — T-Watch 2020 V02 schematic
- **[schematic/T_WATCH-2020V03.pdf](schematic/T_WATCH-2020V03.pdf)** — T-Watch 2020 V03 schematic

### Hardware Reference
- **[hardware/TWatch_S3_Hardware.md](hardware/TWatch_S3_Hardware.md)** — Full hardware specs, pinouts, and electrical characteristics

### Images
- **[images/README.md](images/README.md)** — Documentation images guide
- **[images/BUTTON.jpg](images/BUTTON.jpg)** — Button location image
- **[images/InstallArduino.jpg](images/InstallArduino.jpg)** — Arduino installation guide image

## Getting Started

### For Beginners
1. Read [README.md](README.md) for project overview
2. Follow [configuration/setup-guide.md](configuration/setup-guide.md) for environment setup
3. Build and flash with: `python -m platformio run -e twatch-s3 -t upload --upload-port COM5`
4. Edit `src/config.h` to set WiFi credentials and timezone
5. Refer to [troubleshooting/troubleshooting.md](troubleshooting/troubleshooting.md) if needed

### For Developers
1. Read the [MAINFRAME design spec](superpowers/specs/2026-06-04-twatch-mainframe-clock-design.md)
2. Review the [implementation plan](superpowers/plans/2026-06-04-twatch-mainframe-firmware.md)
3. Check hardware documentation in [schematic/](schematic/) and [shell/](shell/)
4. Run host unit tests: `python -m platformio test -e native`

### For Hardware Modifications
1. Review schematics in [schematic/](schematic/)
2. Check shell designs in [shell/](shell/)
3. Reference images in [images/](images/)

## Quick Links

### Essential Reading
- [Setup Guide](configuration/setup-guide.md) — Get started quickly
- [Quick Start Guide](guides/Quick_Start_Guide.md) — 5-minute start
- [Troubleshooting](troubleshooting/troubleshooting.md) — Solve common issues

### Design & Architecture
- [MAINFRAME Spec](superpowers/specs/2026-06-04-twatch-mainframe-clock-design.md)
- [MAINFRAME Plan](superpowers/plans/2026-06-04-twatch-mainframe-firmware.md)

### Hardware Resources
- [Schematics](schematic/) — Hardware diagrams
- [Shell Design](shell/) — 3D case files
- [Hardware Reference](hardware/TWatch_S3_Hardware.md) — Full hardware specs

## Documentation Standards

### File Naming
- Use kebab-case for file names (e.g., `setup-guide.md`)
- Use descriptive names that indicate content
- Include language suffix for translations (e.g., `README_CN.MD`)

### Structure
- Each documentation file should have a clear title
- Include table of contents for longer documents
- Use consistent heading hierarchy
- Include code examples where appropriate

### Content Guidelines
- Write in British English (as per project preferences)
- Include practical examples
- Provide troubleshooting information
- Keep documentation up to date with code changes

## Contributing to Documentation

### Adding New Documentation
1. Create file in appropriate directory
2. Update this index file
3. Link from relevant existing documentation
4. Follow naming and structure conventions

### Updating Existing Documentation
1. Maintain existing structure where possible
2. Update links if files are moved
3. Ensure all examples still work
4. Test all code snippets

---

*Last updated: 2026-06-04*
*Documentation version: 3.0*
