# Dev Container — Build, Configuration & Customization

How the `ib-algo-trade` dev container is built, configured, and customized.

## Overview

The container provides a C++ development environment with:

- **Clang 20** toolchain (clang, clangd, lld, lldb, clang-tidy, clang-format)
- **CMake + Ninja + ccache** build system
- **Python 3.12** with venv and common packages

Everything is defined in `.devcontainer/`:

| File | Purpose |
|---|---|
| `Dockerfile` | Image layers — OS, toolchain, user |
| `devcontainer.json` | VS Code integration — mounts, ports, extensions |

External services must be set up separately and their ports/sockets must be accessible from within the container by the `ubuntu` user:

- [PostgreSQL](postgresql.md) — TimescaleDB extension and socket path requirements
- [TWS / IB Gateway](tws.md) — API port configuration

## Prerequisites

### Host

- **Docker** — install from the [official Docker repository](https://docs.docker.com/engine/install/ubuntu/), not the Ubuntu apt package
- **VS Code** with the **Dev Containers** extension (`ms-vscode-remote.remote-containers`)
- A directory on the host to persist the user's home data (SSD/NVMe recommended for PG data)

## First-run workflow

### Step 1: Prepare host directory

The container bind-mounts a host directory as `/home/ubuntu`. Create it with ownership matching the default ubuntu user (1000:1000):

```bash
sudo mkdir -p /mnt/ib-home
sudo chown -R 1000:1000 /mnt/ib-home
```

Update `source` in `.devcontainer/devcontainer.json` → `mounts` if using a different path.

### Step 2: Set up `.env`

```bash
cp .env.example .env
```

Fill in `GH_TOKEN` and `ANTHROPIC_AUTH_TOKEN`.

### Step 3: Open in VS Code

**Dev Containers: Reopen in Container** from the command palette.

### Step 4: Clone the repo

Inside the container:

```bash
cd /home/ubuntu
gh repo clone xiahualiu/ib-trader workspace
```

### Step 5: Build the project

```bash
cd /home/ubuntu/workspace
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Step 6: Create Python venv

```bash
cd /home/ubuntu/workspace
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## Customization

### Adding APT packages

Add packages to the relevant `RUN apt-get install` layer in the Dockerfile. Group related packages in the same layer to avoid creating unnecessary image layers.

### Adding Python packages

Create a `requirements.txt` and add a `COPY` + `RUN pip install` layer in the Dockerfile, or install manually inside the container with `pip install --user`.

### Changing the home mount path

Update the `source` field in `devcontainer.json` → `mounts`. The target must remain `/home/ubuntu` (matching `workspaceFolder` and `remoteUser`).
