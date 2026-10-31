# Plutus
> *"God of wealth."* — Greek Mythology

Plutus is a high-performance personal finance suite written in modern C (C23). It combines an accurate mathematical financial engine, an interactive cross-platform terminal user interface (TUI), and modern hardware-accelerated SDL2 graphical charts with interactive inspection tools.

All graphical components use a custom embedded Cascadia Code bitmap typography engine and a dedicated chart widget library (`SDLWidgets`), requiring **zero runtime font or asset dependencies**.

> [!WARNING]
> **Disclaimer:** Plutus is a personal financial modeling project. It is not audited financial software and does not constitute financial advice. Always verify important numbers, such as mortgage payments and loan amortisation tables, against official banking documentation.

---

## Features & Micro-Applications

### Terminal Financial Tools (TUI)
- **Mortgage Calculator**: Computes down payments (age-dependent down payment rules), monthly payments, loan principals, and total interest across custom amortisation terms.
- **Investment Calculator**: Simulates compound investment growth with initial capital, recurring monthly contributions, and expected annual return rates.
- **Expense Tracker**: In-memory and persistent transaction journal tracking income and expenses by category with unique collision-free ID generation, date formatting, and `.plts` binary compact serialization.

### Interactive Graphical Charts (SDL2)
Every chart window supports real-time hover crosshair inspection, auto-scaling grids, stat cards, and dynamic tooltips:

1. **Investment Growth Chart**: Dual-curve visualization comparing cumulative out-of-pocket deposits against compound exponential balance over time.
2. **Mortgage Amortisation Schedule**: Monthly stacked area curve tracking remaining loan balance, cumulative principal paid, cumulative interest paid, and a cost-split donut chart.
3. **Loan Comparison Chart**: Grouped multi-scenario bar chart comparing monthly payment amounts, total interest costs, and total payments across up to 4 loan configurations.
4. **Expense Report Chart**: Income vs. expense bar comparison with a net balance line and an interactive 8-category expense distribution donut chart.
5. **Net Worth & Wealth Timeline**: Long-term wealth accumulation projection featuring optimistic (+2.5%) and conservative (-2.5%) market confidence bands, plus a debt-free crossover marker ($0).
6. **Savings Goal Accumulator**: Stacked contribution vs. investment gains tracker displaying trajectory toward a target savings milestone.
7. **Monthly Budget Allocation Wheel**: Donut chart displaying budget allocation across 8 categories, central surplus/deficit gauge, savings rate, and proportional progress bars.
8. **Accelerated Loan Payoff Race**: Horizontal comparison tracks comparing standard loan amortisation against extra monthly principal payment strategies, highlighting months and interest saved.
9. **Purchasing Power Inflation Erosion**: Nominal face value vs. real purchasing power decay curve over time with a 50% value-lost half-life threshold marker.

---

## Architecture Highlights

- **Standard C23**: Built with modern C standards (`-std=gnu2x` / `/std:clatest`).
- **Portable Terminal Navigation**: ANSI escape codes on Linux/macOS with raw mode (`termios`); Windows Virtual Terminal Processing (`SetConsoleMode`) and UTF-8 console output (`SetConsoleOutputCP`) with `_getch()` on Windows.
- **Zero-Dependency Typography**: Header-only multi-size bitmap font engine (`inc/Font.h`) rendered from Cascadia Code at 4 sizes (`FONT_SM`, `FONT_MD`, `FONT_LG`, `FONT_MONO`).
- **Reusable Chart Engine (`SDLWidgets`)**: Reusable dark-mode palette (`WC_BG`, `WC_SURFACE`, `WC_BORDER`, `WC_GRID`), rounded containers, dashed lines, pie/donut arcs, crosshairs, tooltips, and progress bars.
- **Binary Storage (`PlutusFile`)**: Magic-byte verified (`PLTSFILE`) compact serialization that stores active transaction records without saving empty padding.

---

## Prerequisites

### Linux
- **C Compiler**: GCC 13+ or Clang 16+ (supporting C23)
- **Build System**: CMake 3.25+ and Ninja (or Make)
- **Libraries**: SDL2 development libraries

**Debian / Ubuntu / Linux Mint:**
```bash
sudo apt update
sudo apt install build-essential cmake ninja-build libsdl2-dev
```

**Fedora / RHEL:**
```bash
sudo dnf install gcc cmake ninja-build SDL2-devel
```

**Arch Linux / Manjaro:**
```bash
sudo pacman -S base-devel cmake ninja sdl2
```

---

### Windows
- **Compiler & Tools**: Visual Studio 2022 (with **Desktop development with C++** workload) or MinGW-w64 (GCC 13+)
- **Build System**: CMake 3.25+
- **Libraries**: SDL2 (via [vcpkg](https://vcpkg.io/) or official development release)

**Using vcpkg (Recommended for MSVC):**
```cmd
git clone https://github.com/microsoft/vcpkg.git %USERPROFILE%\vcpkg
cd %USERPROFILE%\vcpkg
bootstrap-vcpkg.bat
vcpkg install sdl2:x64-windows
```

---

## Building and Running

### Linux

1. **Clone the repository:**
   ```bash
   git clone https://github.com/janhruza/Plutus.git
   cd Plutus
   ```

2. **Configure with CMake (using Ninja):**
   ```bash
   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
   ```
   *(Or omit `-G Ninja` to use standard Makefiles)*

3. **Compile:**
   ```bash
   cmake --build build
   ```

4. **Run Plutus:**
   ```bash
   ./build/Plutus
   ```
   *Direct shortcut to launch Investment Growth Chart:*
   ```bash
   ./build/Plutus --chart
   ```

---

### Windows

#### Option A: Visual Studio / MSVC with vcpkg

1. Open **x64 Native Tools Command Prompt for VS 2022**.
2. Navigate to your project directory:
   ```cmd
   cd C:\path\to\Plutus
   ```
3. Configure using the vcpkg toolchain file:
   ```cmd
   cmake -B build -DCMAKE_TOOLCHAIN_FILE=%USERPROFILE%\vcpkg\scripts\buildsystems\vcpkg.cmake
   ```
4. Build:
   ```cmd
   cmake --build build --config Release
   ```
5. Run:
   ```cmd
   build\Release\Plutus.exe
   ```

#### Option B: MinGW-w64

1. Ensure `gcc`, `cmake`, and `ninja` are in your `PATH`.
2. Provide SDL2 via `CMAKE_PREFIX_PATH`:
   ```cmd
   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\SDL2
   cmake --build build
   build\Plutus.exe
   ```

---

## Navigation & Controls

### Terminal Menu
| Key | Action |
|---|---|
| `↑` / `k` / `w` | Move cursor up |
| `↓` / `j` / `s` | Move cursor down |
| `Enter` / `Space` | Select menu option |
| `q` / `ESC` | Exit program / return to menu |

### SDL Chart Windows
| Control | Action |
|---|---|
| `Mouse Hover` | Inspect timeline data point with crosshair and dynamic tooltip |
| `Window Resize` | Automatically recomputes responsive chart layout and grids |
| `ESC` / `q` | Close chart window and return to terminal menu |

---

## Project Structure

```
Plutus/
├── CMakeLists.txt              # CMake build configuration
├── README.md                   # Project documentation
├── LICENSE.md                  # MIT license
├── main.c                      # Application entry point & TUI menu loop
├── inc/                        # Header declarations
│   ├── Ansi.h                  # ANSI terminal formatting macros
│   ├── Config.h                # App configuration file structures
│   ├── Core.h                  # Terminal inputs, progress bars, pauses
│   ├── Font.h                  # Embedded Cascadia Code bitmap typography (4 sizes)
│   ├── Menu.h                  # Portable cross-platform TUI menu component
│   ├── MicroApps.h             # High-level calculator & chart declarations
│   ├── SDLWidgets.h            # Reusable dark-mode SDL2 chart primitives
│   └── Engine/                 # Core financial engine modules
│       ├── ExpenseTracker.h    # Transaction journal structures & balance math
│       ├── Investment.h        # Compound investment & interest calculations
│       ├── Loan.h              # Amortisation schedule & payment engine
│       ├── Mortgage.h          # Down payment & mortgage model
│       ├── PlutusFile.h        # Binary compact file storage
│       └── Transaction.h       # Single financial record definitions & CSV export
└── src/                        # Implementations
    ├── Config.c
    ├── Core.c
    ├── Menu.c                  # Windows (conio) & Linux (termios) input driver
    ├── MicroApps.c             # TUI applications & SDL graphical chart tests
    ├── SDLWidgets.c            # SDL chart rendering engine & widget primitives
    └── Engine/
        ├── ExpenseTracker.c
        ├── Investment.c
        ├── Loan.c
        ├── Mortgage.c
        ├── PlutusFile.c
        └── Transaction.c
```

---

## License

This project is licensed under the MIT License — see the [LICENSE.md](LICENSE.md) file for details.
