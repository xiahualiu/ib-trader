#!/bin/bash
set -e
dbus-daemon --system --fork
pulseaudio --daemonize=yes --exit-idle-time=-1 2>/dev/null || true
exec sleep infinity
