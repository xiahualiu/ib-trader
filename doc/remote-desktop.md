# Remote desktop

The dev container includes a NoMachine remote desktop server with Xfce4 and NVIDIA GPU passthrough.

## Host requirements

- Docker Engine 24+
- NVIDIA drivers (`nvidia-smi`) and [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)
- A [NoMachine client](https://www.nomachine.com/download/)

## Starting the server

The Ansible playbook starts the NX server automatically:

```bash
ansible-playbook /opt/ansible/setup.yml
```

To start manually:

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

Configuration lives in `.devcontainer/devcontainer.json`.

### Image build args

| Variable | Default | Notes |
|---|---|---|
| `CONTAINER_USER` | ibuser | Login username |
| `CONTAINER_PASSWORD` | ibpass | Login password |
| `CONTAINER_UID` | 1000 | User ID in container |
| `CONTAINER_GID` | 1000 | Group ID in container |
| `NOMACHINE_VERSION` | 9.5.7 | NoMachine release |
| `NOMACHINE_REV` | 2 | NoMachine revision |

### Runtime flags

GPU, shared memory, and port mapping are set via `runArgs`:

```json
"--gpus", "all",
"--shm-size=16gb",
"--publish", "0.0.0.0:4000:4000"
```

### Secrets (`.env`)

| Variable | Notes |
|---|---|
| `GH_TOKEN` | GitHub personal access token |
| `ANTHROPIC_AUTH_TOKEN` | Anthropic API key |
| `DATABASE_URL` | PostgreSQL connection string |

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
