# ib-algo-trade

Interactive Brokers algorithmic trading — C++ core with Python verification.

## Setup

### 1. Prepare the ibuser home directory

The container mounts a host directory as `/home/ibuser`. Create it and set ownership to match the container UID/GID (1000:1000 by default):

```bash
sudo mkdir -p /mnt/ib-home
sudo chown -R 1000:1000 /mnt/ib-home
```

If you use a different path, update the `source` in `.devcontainer/devcontainer.json`:

```json
"source=/mnt/ib-home,target=/home/ibuser,type=bind"
```

### 2. Open in container

Open this repo in VS Code. When prompted "Reopen in Container", click yes — or run **Dev Containers: Reopen in Container** from the command palette.

### 3. Clone the repo

Once inside the container, clone the repo under `/home/ibuser`:

```bash
cd /home/ibuser
gh repo clone xiahualiu/ib-trader workspace
cp workspace/.env.example workspace/.env   # edit credentials if desired
```

## Build & test

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/ibalgotrade_tests              # C++ unit tests
PYTHONPATH=python pytest tests/python/  # Python verification tests
```

## `~/setup.sh`

After the container starts, run `~/setup.sh` to initialize and start all services:

- **DBus** — system bus for desktop IPC
- **PulseAudio** — audio server
- **NoMachine** — remote desktop (NX server)
- **PostgreSQL** — database + TimescaleDB
- **IBKR TWS** — Trader Workstation (auto-installs on first run)

The script creates `/home/ibuser/pgdata` and `/home/ibuser/pgsocket` on first run.

## Docs

- [PostgreSQL](doc/postgresql.md) — database setup and usage
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
