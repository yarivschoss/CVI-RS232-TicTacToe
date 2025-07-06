# 🎮 RS‑232 Tic‑Tac‑Toe – LabWindows/CVI

[![Toolchain](https://img.shields.io/badge/LabWindows--CVI-2020%2B-blue)](#source-code-build)
[![Language](https://img.shields.io/badge/C-ANSI-green)](#source-code-build)

> **Course:** CVI Final Project – Serial GUI & Threads (Afeka College, 2025)  
> **Author:** Yariv Shossberger

---

## 📑 Table of Contents
1. [Project Overview](#project-overview)  
2. [Key Features](#key-features)  
3. [Install & Play (No CVI Needed)](#install--play-no-cvi-needed)  
4. [Running with Virtual COM Ports](#running-with-virtual-com-ports)  
5. [Source‑Code Build](#source-code-build)  
6. [Repository Layout](#repository-layout)  

---

## Project Overview
A full‑featured **multiplayer Tic‑Tac‑Toe** game that runs over an **RS‑232 serial link**.  
Play locally (two apps on one PC bridged via virtual COM ports) or across two PCs connected with a null‑modem cable.

Built entirely in **LabWindows/CVI (C)** using separate threads for UI, serial I/O, game logic, and logging.

---

## Key Features
* **Real‑time two‑player Tic‑Tac‑Toe** with graphical board on a canvas  
* **Serial transport abstraction** – works with local virtual ports *or* physical COM cable  
* **Polling vs. interrupt** thread for incoming moves, selectable at runtime  
* **Game save/load** (binary file list)  
* **Animated instruction screen** and sound effects  
* **Comprehensive logging** to file via `FileLog.*`  
* **Modular DLL (`LogicLib`)** – core game & packet framing reusable in other apps  

---

## Install & Play (No CVI Needed)
1. Download the installer **`TicTacToeSetup.exe`** from the repo’s *Releases* page or `install/` folder.  
2. Run the setup (Windows 7 ×86/64 or newer). The wizard will:  
   * Copy the game files  
   * Deploy the **NI LabWindows/CVI 20.0 Run‑Time Engine** automatically  
3. Launch **RS‑232 Tic‑Tac‑Toe** from the Start‑menu shortcut (or `bin\TicTacToe.exe`).  
4. On first run choose **Communication Type → Local** (virtual bridge) or **Cable** (two PCs).  
5. Click **New Game** and play!

> **Note**: No NI development tools are required on the target machine.

---

## Running with Virtual COM Ports
Create a local bridge between **COM1** and **COM2** (e.g., with Virtual Serial Port Tools).  
In‑game, set **Port A = COM1** and **Port B = COM2** (defaults).

---

## Source‑Code Build
1. Open workspace **`FinalWork.cws`** in LabWindows/CVI.  
2. Build → **Rebuild All**.  
3. Run (**Ctrl + F5**) or create a new distribution.

---

## Repository Layout
```
RS232-TicTacToe-CVI/
├── src/
│   ├─ Logic.c|h
│   ├─ MenuLoop.c|h
│   ├─ TicTacToe.c|h
│   ├─ ComThread.c|h
│   ├─ Audio.c|h
├── ui/
│   └─ TicTacToe.uir
├── dll/
│   ├─ LogicLib.prj
│   ├─ LogicLib.dll
├── install/
│   └─ TicTacToeSetup.exe
├── docs/
│   └─ README.rtf
└── README.md
```
