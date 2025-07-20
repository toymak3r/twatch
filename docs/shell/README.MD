# Shell Design Documentation

## Overview
This directory contains 3D shell files and case design documentation for the LilyGO T-Watch S3.

## Files

### 3D Models (.stp files)
- **[BackCover.stp](BackCover.stp)** - Main back cover 3D model (1.5MB)
- **[BackCover1.stp](BackCover1.stp)** - Alternative back cover 3D model (751KB)

### Images (.jpg files)
- **[BackCover.jpg](BackCover.jpg)** - Main back cover render (39KB)
- **[BackCover1.jpg](BackCover1.jpg)** - Alternative back cover render (22KB)

## Usage

### 3D Printing
1. Open the `.stp` files in your preferred 3D modelling software
2. Scale and orient as needed for your printer
3. Export to STL format for slicing
4. Use appropriate print settings for your material

### Design Modifications
- The `.stp` files can be imported into CAD software for modifications
- Ensure compatibility with the T-Watch S3 PCB dimensions
- Consider button placement and USB port access

## Specifications

### Back Cover Dimensions
- Compatible with T-Watch S3 PCB
- Includes cutouts for:
  - USB-C connector
  - Crown button
  - Side buttons
  - Speaker grille
  - Sensor openings

### Material Recommendations
- **PLA/PETG**: Good for prototyping
- **TPU/Flexible**: Better fit and durability
- **ABS**: Higher temperature resistance
- **Resin**: Smooth finish for final versions

## Customisation

### Adding Custom Designs
1. Import the base `.stp` file
2. Add your custom text or logo
3. Ensure proper clearance from components
4. Export modified design

### Colour Options
- The shell can be printed in any colour
- Consider using different colours for top and bottom
- UV-resistant materials recommended for outdoor use

## Troubleshooting

### Print Issues
- **Poor fit**: Check printer calibration and scaling
- **Stringing**: Adjust retraction settings
- **Warping**: Use heated bed and proper adhesion
- **Layer separation**: Increase print temperature

### Design Issues
- **Button interference**: Check button clearance
- **USB port blocked**: Verify port opening dimensions
- **Sensor blocked**: Ensure sensor openings are clear

## Related Documentation
- [Schematics](../schematic/) - Hardware schematics
- [Images](../images/) - Reference images
- [Main Documentation](../README.md) - Complete documentation

## Contributing
If you create custom shell designs:
1. Share your modified files
2. Document any changes made
3. Test fitment thoroughly
4. Provide print settings used

---

*Last updated: [Current Date]*
*Shell design version: 1.0*

