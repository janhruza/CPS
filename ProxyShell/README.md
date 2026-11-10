# ProxyShell (`proxsh`)

> A lightweight proxy shell that intercepts targeted commands to provide custom behaviors while transparently forwarding everything else to your underlying system shell.

---

## Overview

**ProxyShell** (`proxsh`) acts as an intermediary shell layer. When running an interactive session, ProxyShell checks each command against an internal registry of intercepted commands:

- **Custom Handlers**: If a matching command is registered, ProxyShell handles it internally via custom implementations (e.g., customized formatting, error management, or built-in actions).
- **Transparent Fallback**: Any command not intercepted by ProxyShell is passed directly to the host operating system's shell via `system()`.

---

## Features & Command Interception

ProxyShell currently defines and handles the following commands:

| Command        | Status in Shell         | Behavior                                                                                          |
|:---------------|:------------------------|:--------------------------------------------------------------------------------------------------|
| `cat <file>`   | **Active**              | Displays file contents prefixed with formatted line numbers (`%-4d `).                            |
| `clear`        | **Active**              | Clears the visible screen and scrollback history via ANSI sequences (`\033[H\033[2J\033[3J`).    |
| `date`         | **Active**              | Displays the current local date in `DD/MM/YYYY` format.                                           |
| `exit`         | **Active**              | Terminates the interactive ProxyShell session immediately.                                        |
| `ls [path]`    | **Active**              | Enumerates directory contents with item indices (`%-3d`), filtering hidden files (`.`). Defaults to current directory (`.`). |
| `open <target>`| **Active**              | Opens a file, directory, or URL using the system's default handler (`xdg-open`).                   |
| `time`         | **Active**              | Displays the current local time in `HH:MM:SS` format.                                             |
| `touch <file>` | **Active**              | Binary file creation helper; implemented and ready to be bound in the registry.                   |
| *(all other)*  | **Pass-through**        | Forwarded directly to the underlying system shell.                                                |

---

## How It Works

```
                         +------------------------+
                         |  User Input (proxsh:)  |
                         +-----------+------------+
                                     |
                                     v
                       +----------------------------+
                       | Matched known command?     |
                       +-------------+--------------+
                              /             \
                        Yes  /               \  No
                            v                 v
                +-----------------------+   +-----------------------+
                | Internal C Handler    |   | System Shell          |
                | (cat, clear, date,    |   | (system(input))       |
                |  exit, ls, open, time)|   |                       |
                +-----------------------+   +-----------------------+
```

1. Starts an interactive prompt (`proxsh: `) displaying product and version information.
2. Reads command lines via `fgets` and tokenizes the command name.
3. Checks against `commands[]`. If found, executes the respective handler with structured error reporting.
4. If unrecognized, delegates execution transparently to the environment shell.

---

## Building and Running

### Prerequisites

- **C Compiler** supporting the **C23** standard (e.g., modern GCC or Clang)
- **CMake** 3.25+ / 4.x
- **Linux** or **macOS** (POSIX-compatible environment)

### Build Steps

Using CMake:

```bash
# Generate build configuration
cmake -B build -S .

# Compile target binary
cmake --build build
```

This produces the executable binary named **`proxsh`** (located in the `build/` directory).

### Usage

Run the compiled executable directly without arguments:

```bash
./build/proxsh
```

Example session:
```text
ProxyShell, v0.1.0
(c) 2026 @jendahruza

proxsh: cat sample.txt
1    Hello from ProxyShell!
2    Line numbers are added automatically.
proxsh: ls
1  sample.txt
2  CMakeLists.txt
proxsh: time
14:15:30
proxsh: date
10/11/2026
proxsh: open sample.txt
proxsh: clear
... (terminal cleared) ...
proxsh: exit
```

> **Note**: Command-line arguments to the `proxsh` executable itself are disabled by design (`Arguments are disabled.`).

---

## Project Structure

```
ProxyShell/
├── CMakeLists.txt        # CMake build configuration
├── main.c                # Interactive REPL loop & command dispatcher
├── inc/
│   ├── command.h         # Command struct definition & error reporting interface
│   ├── commands.h        # Handler function prototypes (CmdCat, CmdTouch, etc.)
│   ├── macros.h          # Project constants, versioning, and helper macros
│   └── types.h           # Standard type definitions and error codes
├── src/
│   ├── command.c         # Error lookup table and helper implementations
│   └── commands.c        # Custom command handler implementations
├── LICENSE.txt           # MIT License
└── README.md             # Project documentation
```

---

## License

ProxyShell is part of the CPS software suite and is licensed under the [MIT License](LICENSE.md).
