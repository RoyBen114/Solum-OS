[English Docs](README.md) | [中文文档](README_zh.md)   
# Solum OS

>*通用·稳定·兼容——新世代通用操作系统*

## 项目概述

Solum OS 是基于 Multiboot2 规范的 x64 操作系统项目，目前处于早期开发阶段。项目采用 NASM 汇编与 C 语言编写，通过 GRUB 引导启动，支持在 QEMU 虚拟机中运行。

## 构建指南

### 环境依赖
- NASM
- GCC
- LD
- GRUB
- QEMU

### 构建命令

```bash
make         # 完整系统构建
make debug_B # 构建并使用BIOS启动 QEMU
make debug_U # 构建并使用UEFI启动 QEMU
make clean   # 清理构建文件
```

## 技术特性
- 兼容性：遵循 Multiboot2 标准，兼容主流引导程序

- 稳定性：精心规划的内存布局与错误处理机制

- 扩展性：模块化架构设计，便于功能扩展

- 现代性：采用现代操作系统设计理念

## 助项目一臂之力
欢迎提交 Issue 和 Pull Request 共同完善 Solum OS！

## 开源协议
本项目采用 GPL 3.0 开源协议。    
  
>*Solum OS——筑就新世代通用操作系统*
