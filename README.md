[English Docs](README.md) | [中文文档](README_zh.md)   
# Solum OS

>*A General,Stable,Compatible,Next Generation Solum Operating System*

## Project Overview

Solum OS is an x64 operating system project based on the Multiboot2 specification, currently in early development stages. The project is written using NASM assembly and C language, booted via GRUB, and supports running in QEMU virtual machine.

## Build Instructions

### Dependencies
- NASM
- GCC
- LD
- GRUB
- QEMU

### Build Commands

```bash
make         # Build complete system
make debug_B # Build and run in BIOS in QEMU
make debug_U # Build and run in UEFI in QEMU
make clean   # Clean build files
```

## Technical Features
- Compatibility: Follows Multiboot2 standard, compatible with mainstream bootloaders

- Stability: Carefully designed memory layout and error handling

- Extensibility: Modular architecture for easy feature expansion

- Modern: Adopts modern operating system design concepts

## Contributing
Welcome to submit Issues and Pull Requests to help improve Solum OS!

## License
This project uses GPL 3.0 License.    
  
>*Solum OS - Building the Next Generation General Purpose Operating System*
