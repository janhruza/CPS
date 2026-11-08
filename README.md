# Cross-Platform Sandbox (CPS)

A collection of cross-platform C projects, experimental CLI utilities, graphical tools, and software sandboxes developed in modern C (C23).

---

## Projects Overview

| Project | Description | Language | Dependencies | Status |
|:---|:---|:---:|:---|:---:|
| [**NuclearPlant**](NuclearPlant) | Nuclear power plant control panel simulator and reactor state monitor with an interactive command-line interface. | C (C23) | Standard C library | Active |
| [**Plutus**](Plutus) | High-performance personal finance suite featuring financial calculators, an interactive terminal UI (TUI), and hardware-accelerated SDL2 graphical charts with zero runtime font dependencies. | C (C23) | SDL2 | Active |
| [**ProxyShell** (`proxsh`)](ProxyShell) | Lightweight command interceptor and proxy shell layer that hooks targeted commands with custom logic while transparently delegating all other execution to the host shell. | C (C23) | Standard C library | Active |

---

## Project Summaries

### 1. [NuclearPlant](NuclearPlant)
A command-line nuclear power plant simulator modeling core reactor metrics (temperatures, pressure, coolant levels, power outputs, and alarm states). Provides an interactive control terminal to manage reactors and inspect system telemetry.

- **Stack**: C23, CMake
- **Directory**: `NuclearPlant/`

### 2. [Plutus](Plutus)
A personal finance and wealth projection workspace. Includes TUI tools for mortgage planning, compound interest, and transaction tracking (`.plts` binary compact storage), plus 9 hardware-accelerated SDL2 chart visualizers using a built-in bitmap font engine.

- **Stack**: C23, CMake, SDL2
- **Directory**: `Plutus/`

### 3. [ProxyShell (`proxsh`)](ProxyShell)
An interactive intermediary shell utility that intercepts specific commands (such as line-numbered `cat`, ANSI-based `clear`, and session control) while forwarding all unhandled commands to the host environment shell via pass-through execution.

- **Stack**: C23, CMake, POSIX / standard shell APIs
- **Directory**: `ProxyShell/`

---

## Building the Projects

Each project includes its own independent `CMakeLists.txt` build configuration.

### Prerequisites
- Modern C compiler supporting C23 (`gcc` 13+, `clang` 16+, or MSVC with `/std:clatest`)
- CMake 3.25+
- SDL2 development libraries (for `Plutus`)

### General Build Steps
To build any individual project, navigate to its directory:

```bash
# Example: Building ProxyShell
cd ProxyShell
cmake -B build -S .
cmake --build build

# Run
./build/proxsh
```

For project-specific compilation instructions, command-line arguments, and prerequisites, refer to each project's respective `README.md`.

---

## License

This repository and its subprojects are licensed under the [MIT License](LICENSE) (or individual subproject licenses where specified).
