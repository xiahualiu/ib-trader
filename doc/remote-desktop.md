# Remote desktop

The dev container includes a NoMachine remote desktop server with Xfce4 and NVIDIA GPU passthrough.

## Host requirements

- Docker Engine 24+
- NVIDIA drivers (`nvidia-smi`) and [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)
- A [NoMachine client](https://www.nomachine.com/download/)

## Starting the server

```bash
sudo /etc/NX/nxserver --startup
```

## Connecting

Connect your NoMachine client to `<host-ip>:<port>` (default **4000**).

| Setting | Default |
|---|---|
| Username | `ibuser` |
| Password | `ibpass` |

## Verify GPU access

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

## Local devcontainer changes

`devcontainer.json` is tracked in the repo with a baseline configuration. To prevent accidentally committing your local tweaks (port numbers, GPU devices, etc.):

```bash
# Ignore local changes to the file
git update-index --skip-worktree .devcontainer/devcontainer.json

# List all files currently skipped
git ls-files -v . | grep '^S'
```

To intentionally update the baseline later:

```bash
git update-index --no-skip-worktree .devcontainer/devcontainer.json
# edit, commit, push, then re-skip:
git update-index --skip-worktree .devcontainer/devcontainer.json
```

`--skip-worktree` is used here over `--assume-unchanged` because it is designed specifically for "I might change this locally, don't track it" scenarios, and it refuses to silently overwrite your local copy on `git pull`.
