# Project structure

C++ is the source of truth for all production logic. Python wraps the C++ core for prototyping, verification, and analysis.

## Layout

```
ib-algo-trade/
├── include/ibalgotrade/core/   # C++ headers — source of truth
├── src/
│   ├── core/                   # C++ implementation
│   └── pybind/                 # pybind11 bindings → Python _core module
├── python/ibalgotrade/         # Python package (wraps C++ bindings)
│   ├── __init__.py
│   └── verification.py         # Verification helpers for pre-prod testing
├── tests/
│   ├── cpp/                    # C++ unit tests (Catch2)
│   └── python/                 # Python verification tests (pytest)
├── CMakeLists.txt              # Builds lib, bindings, C++ tests
└── pyproject.toml              # pip-installable via scikit-build-core
```

## Workflow

### Day-to-day development

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/ibalgotrade_tests
```

### Python verification

```bash
PYTHONPATH=python pytest tests/python/ -v
PYTHONPATH=python python3 -c "from ibalgotrade import LimitOrderBook; ..."
```

### Adding a new C++ class

1. Create header in `include/ibalgotrade/core/`
2. Implement in `src/core/`
3. Add pybind11 bindings in `src/pybind/module.cpp`
4. Write C++ tests in `tests/cpp/` (Catch2)
5. Write Python verification in `tests/python/` (pytest)
6. Add new `.cpp` files to `CMakeLists.txt`

### Debug build

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
lldb ./build/ibalgotrade_tests
```

## Design decisions

- **`[[nodiscard]]` on `add()`** — fills must never be silently discarded in production; callers must explicitly handle or ignore them.
- **Vectors over deques for price levels** — price levels are small (typically < 100). The O(n) insert/erase is negligible vs. cache-friendly contiguous storage. Revisit if profiling shows otherwise.
- **Pybind11 over nanobind** — broader ecosystem support; nanobind can be swapped in later if the perf difference is measurable.
- **`_core` naming** — the pybind11 module uses a leading underscore to signal it's an implementation detail; the Python package re-exports its symbols.
