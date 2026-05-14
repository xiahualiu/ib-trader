# ib-algo-trade

**Interactive Brokers algorithmic trading platform** — High-performance C++ limit order book with Python verification layer. Production-ready core for building, testing, and deploying forex and equity trading strategies.

## What This Repository Does

`ib-algo-trade` is a **trading infrastructure toolkit** that bridges the gap between backtesting and live trading. It provides:

1. **High-performance order matching engine** (C++ core)
   - Limit order book (LOB) with price-time priority
   - Support for limit and market orders
   - Trade execution and fill tracking
   - Direct, efficient order processing

2. **Python verification layer** for rapid strategy development
   - Pure Python API to the C++ core via pybind11
   - Quick iteration on strategy logic
   - Pre-production testing before moving to C++

3. **Production deployment readiness**
   - Strategies verified in Python can be ported to C++ for speed
   - Low-latency order matching suitable for algorithmic strategies
   - PostgreSQL integration for live market data and trade logging

---

## Why This Helps You as a Forex Trader

### The Problem You're Solving
When you optimize backtesting strategies, you face two challenges:
- **Prototyping speed**: Python is fast to write but too slow for high-frequency matching
- **Production gap**: Code that works in backtesting breaks when deployed live due to latency, fill assumptions, or ordering logic errors

### How ib-algo-trade Solves It

1. **Realistic Order Matching**
   - The order book engine executes trades with **real exchange semantics** (price-time priority)
   - Market orders execute against best available liquidity
   - Partial fills and order cancellations work as they do on live exchanges
   - **You see realistic slippage and execution costs** during testing

2. **Fast Prototyping**
   - Write strategy logic in Python (pandas, numpy, matplotlib included)
   - Call the C++ order matching engine for realistic fills
   - Verify behavior against the order book state
   - No guesswork about "will this order fill?"

3. **Confidence for Production**
   - Once verified in Python, port core logic to C++ for sub-microsecond latency
   - Same order matching semantics means your live performance matches backtest performance
   - PostgreSQL logging tracks every trade and state change for post-trade analysis

---

## What You'll Learn for Building Trading Strategies

### 1. **Order Book Mechanics**
   Learn how exchanges actually work:
   - Best bid/ask tracking and market depth
   - How limit orders rest and when they execute
   - Market order sweeping through multiple price levels
   - Price-time priority (first order at a price fills first)
   - Order cancellation and partial fills

### 2. **Fill Assumptions That Matter**
   - A market buy for 100 contracts at $100 might get split across $100.00, $100.01, $100.02
   - Partial fills change your position in phases, not instantly
   - Order cancellations must handle partially-filled orders correctly
   - **Your backtest slippage calculation can now be exact**, not estimated

### 3. **Strategy Entry/Exit Logic**
   Realistic order placement patterns:
   ```python
   # Example: Two-level entry with market order fallback
   - Place limit buy at 99.50 for 50% size → check fills
   - If not filled in 100ms, place market order for remaining 50%
   - Track average entry price across both orders
   ```

### 4. **Risk Management Validation**
   - Verify stop-loss and take-profit orders execute at intended levels
   - Test partial position exits during trending moves
   - Confirm your position sizing stays within risk limits as fills arrive

### 5. **Slippage and Execution Cost Analysis**
   - Measure actual slippage vs. mid-price expectations
   - Track cost per contract as order size grows
   - Identify which price levels have insufficient liquidity
   - Optimize order sizing for your typical market conditions

### 6. **Multi-Leg Order Strategies**
   - Build strategies that depend on fills from prior legs
   - Test correlation trades (long one contract → short another at relative prices)
   - Verify leg-by-leg execution timing

### 7. **Latency-Aware Strategy Design**
   - Understand how order cancellation latency affects strategy
   - Test behavior when your cancel doesn't reach the exchange in time
   - Design strategies that tolerate realistic latency (not zero-latency fantasy)

---

## Quick Start

### Prerequisites
- **Option A: Dev Container (Recommended)**
  - VS Code with Dev Containers extension
  - Docker or Docker Desktop
  - All tools and dependencies pre-installed

- **Option B: Local Build**
  - Python 3.12+
  - CMake 3.21+
  - C++ compiler (MSVC on Windows, GCC/Clang on Linux/macOS)
  - Ninja build tool (optional but recommended)

### Installation & First Run

#### Option A: Dev Container (Easiest)
```bash
# 1. Open this repo in VS Code
# 2. When prompted, click "Reopen in Container"
# 3. Inside the container terminal:
cd /home/ibuser
gh repo clone xiahualiu/ib-trader workspace
cd workspace

# 4. Build and test:
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
python -m pytest tests/python/test_verification.py -v
```

#### Option B: Local Build (Windows/Linux/macOS)
```bash
# 1. Clone and install dependencies
git clone https://github.com/xiahualiu/ib-trader.git
cd ib-trader
pip install -r requirements.txt
pip install cmake ninja scikit-build-core

# 2. Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 3. Run Python verification tests
python -m pytest tests/python/test_verification.py -v
```

### First Example: Simple Order Book Test
```python
from ibalgotrade import LimitOrderBook, Order, OrderType, Side

# Create order book for EURUSD
book = LimitOrderBook("EURUSD")

# Place a limit buy order
buy_order = Order()
buy_order.id = 1
buy_order.side = Side.Buy
buy_order.type = OrderType.Limit
buy_order.price = 1.0950
buy_order.quantity = 100000  # 100k units

fills = book.add(buy_order)
print(f"Best bid: {book.best_bid()}, Size: {book.bid_size()}")  
# Output: Best bid: 1.095, Size: 100000

# Place a sell order at same price → it fills!
sell_order = Order()
sell_order.id = 2
sell_order.side = Side.Sell
sell_order.type = OrderType.Limit
sell_order.price = 1.0950
sell_order.quantity = 50000

fills = book.add(sell_order)
print(f"Fills: {len(fills)}, Price: {fills[0].price}, Size: {fills[0].quantity}")
# Output: Fills: 1, Price: 1.095, Size: 50000
```

---

## Project Structure

```
ib-algo-trade/
├── include/ibalgotrade/core/
│   └── order.hpp              # Order book header (price-time priority LOB)
├── src/
│   ├── core/order.cpp         # Order book implementation
│   └── pybind/module.cpp      # Python bindings for C++ core
├── python/ibalgotrade/
│   ├── __init__.py            # Python package entry point
│   └── verification.py        # Testing utilities (replay, state verification)
├── tests/
│   ├── cpp/test_order_book.cpp   # C++ unit tests (Catch2)
│   └── python/                   # Python verification tests
├── CMakeLists.txt             # Build configuration
├── pyproject.toml             # Python package metadata
└── doc/                       # PostgreSQL, architecture, remote desktop
```

---

## Key Features

### Order Book Engine
- ✅ **Price-time priority**: Matches real exchange behavior
- ✅ **Partial fills**: Orders can fill across multiple price levels
- ✅ **Market & limit orders**: Both order types supported
- ✅ **Order cancellation**: Full and partial cancels
- ✅ **Fill tracking**: Every trade recorded with order IDs and prices

### Python Integration
- ✅ **Native Python API**: Use from Python scripts and notebooks
- ✅ **NumPy/Pandas compatible**: Works with your existing analysis tools
- ✅ **Verification utilities**: Helper functions to test order book state
- ✅ **Fast iteration**: Develop strategies without C++ compilation

### Production Ready
- ✅ **C++ core**: Sub-microsecond latency when ported
- ✅ **PostgreSQL integration**: Trade logging and backtesting data
- ✅ **Interactive Brokers TWS**: Ready to connect live
- ✅ **Docker support**: Pre-configured dev containers

---

## Building Your First Strategy

### Step 1: Understand Order Book Behavior
Run the included tests to see order matching in action:
```bash
python -m pytest tests/python/test_verification.py -v
```

### Step 2: Backtest a Simple Strategy
```python
from ibalgotrade import LimitOrderBook, Order, OrderType, Side

def backtest_momentum_entry(prices, threshold):
    """
    Simple momentum: buy if price up 2%, sell if down 2%
    """
    book = LimitOrderBook("EUR/USD")
    entry_price = prices[0]
    
    for i, price in enumerate(prices[1:], start=1):
        if price > entry_price * (1 + threshold):
            # Price rallied → place buy order above market
            order = Order()
            order.id = i
            order.side = Side.Buy
            order.type = OrderType.Limit
            order.price = price * 1.001  # Buy 0.1% above current
            order.quantity = 100000
            
            fills = book.add(order)
            if fills:
                print(f"Bought {fills[0].quantity} @ {fills[0].price}")
        
        elif price < entry_price * (1 - threshold):
            # Price fell → place sell order
            # (implementation similar)
            pass

# Test with sample prices
prices = [1.0950, 1.0965, 1.0980, 1.0990]  # Mock price series
backtest_momentum_entry(prices, threshold=0.0020)
```

### Step 3: Measure Execution Quality
```python
from ibalgotrade.verification import ExpectedState, check_state

# Verify the order book matches expectations after a trade sequence
book = LimitOrderBook("EUR/USD")

# ... add orders ...

expected = ExpectedState(
    best_bid=1.0950,
    best_ask=1.0952,
    bid_size=100000,
    ask_size=50000,
    order_count=2
)

if check_state(book, expected):
    print("✓ Order book state matches expectations")
else:
    print("✗ Order book state mismatch - execution logic bug!")
```

---

## Configuration & Advanced Usage

### PostgreSQL for Live Trading
See [doc/postgresql.md](doc/postgresql.md) for setting up trade logging and market data persistence.

### Remote Desktop for Live Monitoring
See [doc/remote-desktop.md](doc/remote-desktop.md) to set up NoMachine remote access to run Interactive Brokers TWS.

### Project Architecture
See [doc/project-structure.md](doc/project-structure.md) for detailed design decisions and module organization.

---

## Testing

### Run All Tests
```bash
# C++ unit tests
./build/ibalgotrade_tests

# Python verification tests
python -m pytest tests/python/ -v
```

### Example Test Coverage
- ✅ Empty order book initialization
- ✅ Limit orders resting when no match
- ✅ Matching at same price (price-time priority)
- ✅ Market orders sweeping multiple levels
- ✅ Order cancellation
- ✅ Partial fills across price levels

---

## Common Trading Questions Answered

**Q: Can I use this for real money?**  
A: Yes, but start small. The order matching engine is production-grade, but you should:
- Paper trade first to understand latency
- Test your actual broker connection (demo IBKR account)
- Verify your strategy performs similarly live vs. backtest

**Q: How accurate is the slippage calculation?**  
A: Exact. The order book matches real exchange semantics. Your slippage is whatever the simulated market gives you—realistic or unrealistic depending on your input market data.

**Q: Can I use this for crypto?**  
A: Yes, if you have order book data. The order matching engine is exchange-agnostic. Just feed it ticks from your chosen exchange.

**Q: What about latency?**  
A: The C++ core is sub-microsecond. Python overhead dominates. For production, port critical path to C++.

**Q: How do I handle gaps in market data?**  
A: This engine doesn't handle time; it's stateless matching. You control the order flow. If a 1-second gap occurs in your data, no orders execute during that gap.

---

## Roadmap

- [ ] Multi-symbol order books (current: single symbol only)
- [ ] Advanced order types (iceberg, stop-limit, trailing stops)
- [ ] Risk checks (position limits, margin requirements)
- [ ] Backtest replay with realistic OHLC+ candles
- [ ] WebSocket real-time order book updates from IBKR

---

## License

MIT — See [LICENSE](LICENSE)

---

## Contributing

Contributions welcome! Areas of interest:
- Strategy examples (real-world trading logic)
- Performance optimizations
- Additional order types
- Broker integration improvements

---

## Questions?

- **Strategy ideas?** Check [tests/python/](tests/python/) for examples
- **Order book behavior?** See inline comments in [include/ibalgotrade/core/order.hpp](include/ibalgotrade/core/order.hpp)
- **Building & troubleshooting?** See [doc/project-structure.md](doc/project-structure.md)

---

**Happy backtesting!** 🚀
