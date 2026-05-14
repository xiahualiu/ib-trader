#!/bin/bash
set -e

# ── DBus system daemon ──────────────────────────────────────────────
if [ "$(id -u)" -eq 0 ]; then
    dbus-daemon --system --fork
else
    sudo dbus-daemon --system --fork
fi

# ── PulseAudio ──────────────────────────────────────────────────────
pulseaudio --daemonize=no --exit-idle-time=-1 &
echo "PulseAudio started (PID: $!)"

# ── Git credential helper ───────────────────────────────────────────
gh auth setup-git --hostname github.com 2>/dev/null || true

# ── PostgreSQL ──────────────────────────────────────────────────────
PGDATA=/home/ibuser/pgdata
PGSOCKET=/home/ibuser/pgsocket

if [ ! -f "$PGDATA/PG_VERSION" ]; then
    mkdir -p "$PGDATA" "$PGSOCKET"
    initdb -D "$PGDATA"
    echo "unix_socket_directories = '/home/ibuser/pgsocket'" >> "$PGDATA/postgresql.conf"
fi

mkdir -p "$PGSOCKET"
pg_ctl -D "$PGDATA" -l "$PGDATA/logfile" -w start
echo "PostgreSQL started (socket: $PGSOCKET)"

# ── IBKR TWS ────────────────────────────────────────────────────────
TWS_DIR=/home/ibuser/tws
if [ ! -d "$TWS_DIR" ] || [ -z "$(ls -A "$TWS_DIR" 2>/dev/null)" ]; then
    echo "Installing TWS..."
    wget -q "https://download2.interactivebrokers.com/installers/tws/latest/tws-latest-linux-x64.sh" \
        -O /tmp/tws-installer.sh
    chmod +x /tmp/tws-installer.sh
    /tmp/tws-installer.sh -q -dir "$TWS_DIR"
    rm /tmp/tws-installer.sh
    echo "TWS installed to $TWS_DIR"
fi

# ── NoMachine remote desktop ───────────────────────────────────────
if [ "$(id -u)" -eq 0 ]; then
    /usr/NX/bin/nxserver --startup
else
    sudo /usr/NX/bin/nxserver --startup
fi
echo "NoMachine started"
