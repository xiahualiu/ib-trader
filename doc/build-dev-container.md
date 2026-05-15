# Dev Container — Build, Configuration & Customization

How the `ib-algo-trade` dev container is built, configured, and customized.

## Overview

The container provides a full desktop development environment with:

- **Xfce** desktop via **NoMachine** remote access
- **Clang 20** toolchain (clang, clangd, lld, lldb, clang-tidy, clang-format)
- **CMake + Ninja + ccache** build system
- **Python 3.12** with venv and common packages
- **PostgreSQL 18 + TimescaleDB** for time-series market data
- **IBKR TWS** for broker connectivity

Everything is defined in `.devcontainer/`:

| File | Purpose |
|---|---|
| `Dockerfile` | Image layers — OS, toolchain, user, services |
| `devcontainer.json` | VS Code integration — mounts, ports, extensions |
| `ansible/setup.yml` | Ansible playbook for runtime services (DBus, PulseAudio, NoMachine, PG, TWS) |


## Prerequisites

### Host

- **Docker** with GPU support ([`nvidia-container-toolkit`](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html))
- **VS Code** with the **Dev Containers** extension (`ms-vscode-remote.remote-containers`)
- A directory on the host to persist the user's home data (SSD/NVMe recommended for PG data)

## First-run workflow

### Step 1: Prepare host directory

The container bind-mounts a host directory as `/home/ibuser`. Create it with ownership matching the container UID/GID (1000:1000 by default):

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
cd /home/ibuser
gh repo clone xiahualiu/ib-trader workspace
```

### Step 5: Run setup

```bash
ansible-playbook /opt/ansible/setup.yml
```

Initializes and starts DBus, PulseAudio, PostgreSQL (+TimescaleDB), NoMachine NX server, and IBKR TWS (auto-downloads on first run). The playbook is idempotent — safe to re-run at any time.

### Step 6: Build the project

```bash
cd /home/ibuser/workspace
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Step 7: Create Python venv

```bash
cd /home/ibuser/workspace
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## Connecting via NoMachine

After the playbook has started the NX server:

1. Install the [NoMachine client](https://www.nomachine.com/download) on your local machine
2. Connect to `<container-host-ip>:4000`
3. Login with `CONTAINER_USER` / `CONTAINER_PASSWORD` (default: `ibuser` / `ibpass`)
4. You should see the Xfce desktop

See [Remote desktop](remote-desktop.md) for more details.

## Start TWS

TWS is installed at `/home/ibuser/tws` by the playbook on first run. To launch:

```bash
/home/ibuser/tws/tws &
```

The IB Gateway / TWS must be running with API access enabled for the trading engine to connect.

## Customization

### Changing user credentials

Edit `CONTAINER_USER`, `CONTAINER_PASSWORD`, `CONTAINER_UID`, `CONTAINER_GID` in the `build.args` of `devcontainer.json`. Make sure the host directory ownership matches the new UID/GID.

### Changing NoMachine version

Update `NOMACHINE_VERSION` and `NOMACHINE_REV` in `build.args`. Check [NoMachine downloads](https://www.nomachine.com/download) for available versions.

### Adding APT packages

Add packages to the relevant `RUN apt-get install` layer in the Dockerfile. Group related packages in the same layer to avoid creating unnecessary image layers.

### Adding Python packages

Create a `requirements.txt` and add a `COPY` + `RUN pip install` layer in the Dockerfile, or install manually inside the container with `pip install --user`.

### Changing the home mount path

Update the `source` field in `devcontainer.json` → `mounts`. The target must remain `/home/ibuser` (matching `workspaceFolder` and `remoteUser`).
