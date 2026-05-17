FROM ubuntu:24.04

ARG GIT_USER_NAME
ARG GIT_USER_EMAIL

ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8

# ---------------------------------------------------------------------------
# Base system
# ---------------------------------------------------------------------------
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    curl \
    git \
    gnupg \
    locales \
    sudo \
    && locale-gen en_US.UTF-8 && \
    update-locale LANG=en_US.UTF-8 && \
    rm -rf /var/lib/apt/lists/*

# ---------------------------------------------------------------------------
# GitHub CLI
# ---------------------------------------------------------------------------
RUN mkdir -p -m 755 /etc/apt/keyrings && \
    curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg | tee /etc/apt/keyrings/githubcli-archive-keyring.gpg > /dev/null && \
    echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" > /etc/apt/sources.list.d/github-cli.list && \
    apt-get update && apt-get install -y --no-install-recommends gh && \
    rm -rf /var/lib/apt/lists/*

# ---------------------------------------------------------------------------
# LLVM + CMake + C++ build toolchain
# ---------------------------------------------------------------------------
RUN curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key | tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc && \
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
    libpq-dev \
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
# Runtime
# ---------------------------------------------------------------------------

USER ubuntu
WORKDIR /home/ubuntu

# ---------------------------------------------------------------------------
# Git config
# ---------------------------------------------------------------------------
RUN if [ -n "$GIT_USER_NAME" ]; then \
        git config --global user.name "$GIT_USER_NAME"; \
    fi && \
    if [ -n "$GIT_USER_EMAIL" ]; then \
        git config --global user.email "$GIT_USER_EMAIL"; \
    fi

CMD ["/bin/bash"]
