# ib-algo-trade

Dev container for Interactive Brokers algorithmic trading development with NoMachine remote desktop + Xfce, NVIDIA GPU passthrough, and a full LLVM/CMake/C++ toolchain.

## What's inside

| Layer | Tools |
|---|---|
| **Remote desktop** | NoMachine server + Xfce4 with GPU acceleration |
| **C++ toolchain** | Clang 20, lld, lldb, clangd, clang-tidy, clang-format |
| **Build** | CMake, Ninja, ccache |
| **Python** | Python 3.12, ib_insync, pandas, numpy, matplotlib, pytest |
| **GPU** | NVIDIA Container Toolkit passthrough (graphics + compute + utility) |

## Host requirements

- Docker Engine 24+
- NVIDIA drivers (`nvidia-smi`) and [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)
- A [NoMachine client](https://www.nomachine.com/download/)

## Quick start

```bash
cp .env.example .env          # edit credentials if desired
```

Open this repo in VS Code. When prompted "Reopen in Container", click yes — or run **Dev Containers: Reopen in Container** from the command palette.

## Connecting

Start the NoMachine server inside the container:

```bash
sudo /etc/NX/nxserver --startup
```

Connect your NoMachine client to `<host-ip>:<port>` (default **4000**).

| Setting | Default |
|---|---|
| Username | `ibuser` |
| Password | `ibpass` |

## Dev environment

The repo root (`..`) is mounted at `/workspace` inside the container. Each user's home directory gets a `workspace` symlink pointing there.

### Shell aliases (pre-configured in `.bashrc`)

```bash
configure    # cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
dbg          # same with -DCMAKE_BUILD_TYPE=Debug
build        # cmake --build build
```

### Default toolchain

```bash
CC=clang        # Clang 20
CXX=clang++     # Clang 20
```

### Verify GPU access

```bash
nvidia-smi
vulkaninfo --summary
```

## Customization

All tunables are in `.env` — see `.env.example` for the full list.

| Variable | Default | Notes |
|---|---|---|
| `HOST_NX_PORT` | 4000 | Port on the host |
| `CONTAINER_USER` | ibuser | Login username |
| `CONTAINER_PASSWORD` | ibpass | Login password |
| `NOMACHINE_VERSION` | 9.5.7 | NoMachine release |
| `NVIDIA_VISIBLE_DEVICES` | all | GPU selector |
| `SHM_SIZE` | 16gb | `/dev/shm` for GUI apps |

## Adding tools

Edit `.devcontainer/Dockerfile` and rebuild the container with **Dev Containers: Rebuild Container** from the VS Code command palette.

## License

MIT — see [LICENSE](LICENSE).
