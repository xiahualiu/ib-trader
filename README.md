# ib-algo-trade

Interactive Brokers algorithmic trading — C++ core with Python verification.

## Overview

The container provides a full desktop development environment with:

- **Xfce** desktop via **NoMachine** remote access
- **Clang 20** toolchain (clang, clangd, lld, lldb, clang-tidy, clang-format)
- **CMake + Ninja + ccache** build system
- **Python 3.12** with venv and common packages
- **PostgreSQL 18 + TimescaleDB** for time-series market data
- **IBKR TWS** for broker connectivity


## Toolchain

| Layer | Tools |
|---|---|
| **C++** | Clang 20, lld, lldb, clangd, clang-tidy, clang-format |
| **Build** | CMake, Ninja, ccache |
| **Python** | Python 3.12, ib_insync, pandas, numpy, matplotlib, pytest |

## Docs

- [Dev Container Build & Setup](doc/build-dev-container.md)
- [PostgreSQL](doc/postgresql.md)
- [Project Structure](doc/project-structure.md)
- [Remote Desktop](doc/remote-desktop.md)

## License

MIT — see [LICENSE](LICENSE).
