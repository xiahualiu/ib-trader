# ib-algo-trade

Interactive Brokers algorithmic trading — C++ core with Python verification.

## Setup

```bash
cp .env.example .env          # edit credentials if desired
```

Open this repo in VS Code. When prompted "Reopen in Container", click yes — or run **Dev Containers: Reopen in Container** from the command palette.

## Build & test

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/ibalgotrade_tests              # C++ unit tests
PYTHONPATH=python pytest tests/python/  # Python verification tests
```

Shell aliases are pre-configured: `configure`, `dbg`, `build`.

## Docs

- [Project structure](doc/project-structure.md) — layout, workflow, design decisions
- [Remote desktop](doc/remote-desktop.md) — NoMachine + Xfce setup and connection

## Toolchain

Toolchain is pre-installed in the Dev Container.

| Layer | Tools |
|---|---|
| **C++** | Clang 20, lld, lldb, clangd, clang-tidy, clang-format |
| **Build** | CMake, Ninja, ccache |
| **Python** | Python 3.12, ib_insync, pandas, numpy, matplotlib, pytest |

## License

MIT — see [LICENSE](LICENSE).
