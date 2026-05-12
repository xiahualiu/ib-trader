FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV NVIDIA_VISIBLE_DEVICES=all
ENV NVIDIA_DRIVER_CAPABILITIES=all

# ---------------------------------------------------------------------------
# Base desktop + system
# ---------------------------------------------------------------------------
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    curl \
    dbus \
    dbus-x11 \
    git \
    gnupg \
    libegl1 \
    libgl1 \
    libgles2 \
    libglvnd0 \
    libglx0 \
    libopengl0 \
    libvulkan1 \
    pulseaudio \
    sudo \
    wget \
    xfce4 \
    xfce4-goodies \
    && rm -rf /var/lib/apt/lists/*

# ---------------------------------------------------------------------------
# GitHub CLI
# ---------------------------------------------------------------------------
RUN mkdir -p -m 755 /etc/apt/keyrings && \
    wget -qO- https://cli.github.com/packages/githubcli-archive-keyring.gpg | tee /etc/apt/keyrings/githubcli-archive-keyring.gpg > /dev/null && \
    echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" > /etc/apt/sources.list.d/github-cli.list && \
    apt-get update && apt-get install -y --no-install-recommends gh && \
    rm -rf /var/lib/apt/lists/*

# ---------------------------------------------------------------------------
# NoMachine remote desktop
# ---------------------------------------------------------------------------
ARG NOMACHINE_VERSION=9.5.7
ARG NOMACHINE_REV=2
RUN wget -q "https://download.nomachine.com/download/${NOMACHINE_VERSION%.*}/Linux/nomachine_${NOMACHINE_VERSION}_${NOMACHINE_REV}_amd64.deb" \
        -O /tmp/nomachine.deb && \
    dpkg -i /tmp/nomachine.deb 2>/dev/null; apt-get update && apt-get install -f -y --no-install-recommends && \
    rm /tmp/nomachine.deb && \
    rm -rf /var/lib/apt/lists/*

# ---------------------------------------------------------------------------
# LLVM + CMake + C++ build toolchain
# ---------------------------------------------------------------------------
RUN wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc && \
    echo "deb http://apt.llvm.org/noble/ llvm-toolchain-noble-20 main" > /etc/apt/sources.list.d/llvm.list && \
    apt-get update && apt-get install -y --no-install-recommends \
    # LLVM
    clang-20 \
    clang-format-20 \
    clang-tidy-20 \
    clangd-20 \
    libc++-20-dev \
    libc++abi-20-dev \
    lld-20 \
    lldb-20 \
    # CMake
    cmake \
    ninja-build \
    # Build essentials
    build-essential \
    gdb \
    pkg-config \
    ccache \
    # Python + IB API / tooling
    python3 \
    python3-dev \
    python3-pip \
    python3-venv \
    python3-wheel \
    python3-setuptools \
    && rm -rf /var/lib/apt/lists/*

# Set LLVM 20 as default
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-20 200 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-20 200 \
    && update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-20 200 \
    && update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-20 200 \
    && update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-20 200 \
    && update-alternatives --install /usr/bin/lld lld /usr/bin/lld-20 200 \
    && update-alternatives --install /usr/bin/lldb lldb /usr/bin/lldb-20 200

# ---------------------------------------------------------------------------
# User setup
# ---------------------------------------------------------------------------
ARG CONTAINER_USER=ibuser
ARG CONTAINER_PASSWORD=ibpass
ARG CONTAINER_UID=1000
ARG CONTAINER_GID=1000

RUN userdel -r ubuntu 2>/dev/null || true && \
    groupadd -g "$CONTAINER_GID" "$CONTAINER_USER" 2>/dev/null || true && \
    useradd -m -s /bin/bash -u "$CONTAINER_UID" -g "$CONTAINER_GID" "$CONTAINER_USER" && \
    echo "$CONTAINER_USER:$CONTAINER_PASSWORD" | chpasswd && \
    usermod -aG sudo "$CONTAINER_USER" && \
    echo "$CONTAINER_USER ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/$CONTAINER_USER && \
    mkdir -p /opt/ibkr/tws && \
    chown $CONTAINER_USER:$CONTAINER_USER /opt/ibkr/tws

ENV NX_USER=${CONTAINER_USER}
ENV CONTAINER_USER=${CONTAINER_USER}

# Pre-configure NoMachine for XFCE
RUN sed -i 's|^#\?DefaultDesktopCommand .*|DefaultDesktopCommand "/usr/bin/startxfce4"|' /usr/NX/etc/node.cfg || \
        echo 'DefaultDesktopCommand "/usr/bin/startxfce4"' >> /usr/NX/etc/node.cfg && \
    sed -i 's|^#\?CreateDisplay .*|CreateDisplay 1|' /usr/NX/etc/server.cfg || \
        echo 'CreateDisplay 1' >> /usr/NX/etc/server.cfg && \
    sed -i 's|^#\?VirtualDesktopAuthorization .*|VirtualDesktopAuthorization 0|' /usr/NX/etc/server.cfg || \
        echo 'VirtualDesktopAuthorization 0' >> /usr/NX/etc/server.cfg && \
    sed -i 's|^#\?DisplayOwner .*|DisplayOwner "'"$NX_USER"'"|' /usr/NX/etc/server.cfg || \
        echo 'DisplayOwner "'"$NX_USER"'"' >> /usr/NX/etc/server.cfg && \
    mkdir -p /run/dbus /tmp/.X11-unix

USER $CONTAINER_USER
WORKDIR /home/$CONTAINER_USER

# ---------------------------------------------------------------------------
# IBKR TWS (Trader Workstation)
# ---------------------------------------------------------------------------
ARG TWS_VERSION=latest
RUN wget -q "https://download2.interactivebrokers.com/installers/tws/${TWS_VERSION}/tws-${TWS_VERSION}-linux-x64.sh" \
        -O /tmp/tws-installer.sh && \
    chmod +x /tmp/tws-installer.sh && \
    /tmp/tws-installer.sh -q -dir /opt/ibkr/tws && \
    rm /tmp/tws-installer.sh

# ---------------------------------------------------------------------------
# Git config
# ---------------------------------------------------------------------------
ARG GIT_USER_NAME
ARG GIT_USER_EMAIL

RUN if [ -n "$GIT_USER_NAME" ]; then \
        git config --global user.name "$GIT_USER_NAME"; \
    fi && \
    if [ -n "$GIT_USER_EMAIL" ]; then \
        git config --global user.email "$GIT_USER_EMAIL"; \
    fi && \
    gh auth setup-git --hostname github.com

# ---------------------------------------------------------------------------
# Runtime
# ---------------------------------------------------------------------------
USER root
COPY tws.desktop /usr/share/applications/tws.desktop
RUN mkdir -p /home/$CONTAINER_USER/Desktop && \
    cp /usr/share/applications/tws.desktop /home/$CONTAINER_USER/Desktop/tws.desktop && \
    chown $CONTAINER_USER:$CONTAINER_USER /home/$CONTAINER_USER/Desktop/tws.desktop

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
EXPOSE 4000
ENTRYPOINT ["/entrypoint.sh"]
