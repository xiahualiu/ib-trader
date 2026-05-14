# Real-Time Trading Simulation Environment: Complete Development Guide

**For Forex Traders Moving from Backtesting to Live-Like Simulation**

---

## Table of Contents
1. [Overview & Problem Statement](#overview--problem-statement)
2. [System Architecture](#system-architecture)
3. [Components Breakdown](#components-breakdown)
4. [Data Flow & Event Loop](#data-flow--event-loop)
5. [Dependencies & Stack](#dependencies--stack)
6. [Step-by-Step Implementation](#step-by-step-implementation)
7. [Integration with ib-trader Repo](#integration-with-ib-trader-repo)
8. [Testing & Validation](#testing--validation)
9. [Real-World Considerations](#real-world-considerations)
10. [Common Pitfalls](#common-pitfalls)
11. [Next Steps: Moving to Actual Paper Trading](#next-steps-moving-to-actual-paper-trading)

---

## Overview & Problem Statement

### The Gap You're Addressing
You've backtested a forex strategy successfully. But your backtest likely makes unrealistic assumptions:
- **Orders fill instantly** at requested prices
- **No slippage variation** based on market depth
- **No queue position** (your order might be behind others at the same price)
- **No partial fills** across price levels for market orders
- **No order rejection** or latency delays
- **No realistic price gaps** or fast market moves that prevent fills

**Real-world problem**: Your backtest returns 25% annually, but in paper trading, you get 12%. Why? Because your order execution model was wrong.

### What This Simulation Does
This guide covers building a **real-time order-driven simulation engine** that:
1. Ingests live L2 market data (bid/ask with sizes, depth levels)
2. Maintains a realistic limit order book for each instrument
3. Executes your strategy's orders against that book
4. Tracks fills, slippage, and execution costs
5. Validates whether your strategy logic handles real-world execution
6. Logs everything for post-trade analysis

Think of it as a **local paper trading simulator** that runs in-process before you connect to a real broker.

### Why This Matters
- **Cost**: Catches bugs before connecting to real brokers
- **Speed**: Test iterations in minutes, not days
- **Control**: Replay any market scenario; simulate gaps, flash crashes, etc.
- **Learning**: Understand execution subtleties your backtest missed

---

## System Architecture

### High-Level Design
```
┌─────────────────────────────────────────────────────────────────┐
│                   REAL-TIME SIMULATION ENGINE                    │
├─────────────────────────────────────────────────────────────────┤
│                                                                   │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  Market Data Feed Layer                                 │    │
│  │  - WebSocket/API connection to data provider            │    │
│  │  - Tick parsing (bid/ask/size)                          │    │
│  │  - Order book reconstruction                            │    │
│  └─────────────────────────────────────────────────────────┘    │
│                          ↓                                        │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  Event Loop & Time Management                           │    │
│  │  - Timestamp synchronization                            │    │
│  │  - Event queue (data ticks, order fills, timers)        │    │
│  │  - Real-time or replay mode                             │    │
│  └─────────────────────────────────────────────────────────┘    │
│                          ↓                                        │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  Order Book Engine (ib-trader C++ core)                 │    │
│  │  - Limit order book for each symbol                     │    │
│  │  - Price-time priority matching                         │    │
│  │  - Fill generation                                      │    │
│  └─────────────────────────────────────────────────────────┘    │
│                          ↓                                        │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  Strategy Execution Layer                               │    │
│  │  - Your trading strategy (entry/exit logic)             │    │
│  │  - Position tracking                                    │    │
│  │  - Order generation                                     │    │
│  └─────────────────────────────────────────────────────────┘    │
│                          ↓                                        │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  Risk Management & Execution Checks                     │    │
│  │  - Margin/capital validation                            │    │
│  │  - Position limits                                      │    │
│  │  - Stop-loss enforcement                                │    │
│  └─────────────────────────────────────────────────────────┘    │
│                          ↓                                        │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  Logging & Storage Layer                                │    │
│  │  - PostgreSQL: Orders, fills, positions                 │    │
│  │  - In-memory: P&L, metrics, performance stats           │    │
│  └─────────────────────────────────────────────────────────┘    │
│                          ↓                                        │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  Monitoring & Reporting                                 │    │
│  │  - Real-time dashboard (optional: Flask/Streamlit)      │    │
│  │  - Post-run analysis                                    │    │
│  │  - Performance metrics, slippage reports                │    │
│  └─────────────────────────────────────────────────────────┘    │
│                                                                   │
└─────────────────────────────────────────────────────────────────┘
```

### Key Design Principles
1. **Event-Driven**: Everything is an event (data tick, fill, timer). Process them in order.
2. **Time-Aware**: Every action has a timestamp. Supports both real-time and replay modes.
3. **Modular**: Each layer can be tested independently.
4. **Realistic**: Models actual exchange semantics (price-time priority, partial fills, latency).
5. **Observable**: Log everything for debugging and analysis.

---

## Components Breakdown

### 1. Market Data Feed Layer
**Responsibility**: Ingest real-time L2 data and reconstruct the order book.

#### What It Needs to Handle
- **Data Source**: WebSocket or REST API from your data provider
  - Options: Interactive Brokers (TWS), Dukascopy, IQFeed, Alpaca, CoinGecko (for crypto)
  - Format: Ticks (timestamp, bid, ask, bid_size, ask_size, volume)
  - Frequency: For forex, typically 1-10 ticks per second per pair
  
- **Order Book Reconstruction**
  - Raw tick → add/update bid/ask levels in the order book
  - Example: If EUR/USD tick is (1.0950 bid, 10M size; 1.0951 ask, 15M size)
  - Add two limit orders to the book: buy limit at 1.0950 (10M), sell limit at 1.0951 (15M)

- **Handling Missing Data**
  - Network disconnections
  - Data gaps (e.g., market closes for 1 hour)
  - Stale data (what's "too old" to use?)

#### Implementation Blueprint
```python
class MarketDataFeed:
    def __init__(self, symbol, data_provider):
        self.symbol = symbol
        self.provider = data_provider  # e.g., IB API, WebSocket client
        self.current_tick = None
        self.tick_history = []
        
    async def connect(self):
        """Establish connection to data source."""
        pass
    
    async def on_tick(self, tick):
        """Called when new market data arrives."""
        # Parse tick: extract bid, ask, sizes, timestamp
        # Add to order book (via order book engine)
        # Emit tick event to event loop
        pass
    
    def get_bid_ask(self):
        """Return current best bid/ask."""
        if self.current_tick:
            return self.current_tick['bid'], self.current_tick['ask']
        return None, None
    
    def get_depth(self, levels=5):
        """Return L2 depth (bid/ask levels with sizes)."""
        pass
```

#### Data Format Example
```python
tick = {
    'timestamp': 1715779200123,  # milliseconds
    'symbol': 'EUR/USD',
    'bid': 1.09501,
    'ask': 1.09504,
    'bid_size': 10000000,  # 10M units
    'ask_size': 12000000,  # 12M units
    'volume': 25000000,  # Traded volume this tick
}
```

---

### 2. Order Book Engine (ib-trader core)
**Responsibility**: Match orders against realistic market semantics.

This is where the **ib-trader repo** comes in. You'll use the C++ `LimitOrderBook` class via Python bindings.

#### What It Does
- **Maintains bid/ask levels** with sizes and order IDs
- **Matches incoming orders** using price-time priority
- **Generates fills** with realistic slippage
- **Handles partial fills** (market order sweeps multiple levels)
- **Tracks order status** (resting, filled, canceled)

#### Usage in Your Simulation
```python
from ibalgotrade import LimitOrderBook, Order, OrderType, Side

class OrderBookManager:
    def __init__(self, symbol):
        self.symbol = symbol
        self.book = LimitOrderBook(symbol)
        self.pending_orders = {}  # ID -> Order
        self.filled_orders = {}   # ID -> list of Fill
        
    def add_market_data(self, tick):
        """Update book with market data."""
        # Simulate market liquidity by adding liquidity orders to book
        bid_order = Order()
        bid_order.id = hash((self.symbol, 'bid', tick['timestamp']))
        bid_order.side = Side.Buy
        bid_order.type = OrderType.Limit
        bid_order.price = tick['bid']
        bid_order.quantity = tick['bid_size']
        self.book.add(bid_order)
        
        ask_order = Order()
        ask_order.id = hash((self.symbol, 'ask', tick['timestamp']))
        ask_order.side = Side.Sell
        ask_order.type = OrderType.Limit
        ask_order.price = tick['ask']
        ask_order.quantity = tick['ask_size']
        self.book.add(ask_order)
    
    def submit_order(self, order):
        """Submit strategy order to book."""
        fills = self.book.add(order)
        self.pending_orders[order.id] = order
        if fills:
            self.filled_orders[order.id] = fills
        return fills
    
    def cancel_order(self, order_id):
        """Cancel resting order."""
        self.book.cancel(order_id)
        if order_id in self.pending_orders:
            del self.pending_orders[order_id]
```

#### Key Insight: Realistic Slippage
- **Your backtest**: "Buy EUR/USD at 1.0950" → assumes fill at 1.0950
- **This simulation**: 
  - If ask is 1.0951, limit buy at 1.0950 **doesn't fill** (rests)
  - Market buy sweeps: 10M @ 1.0951, 5M @ 1.0952 = avg 1.09512
  - **Slippage = 1.09512 - 1.0950 = 0.00012 (12 pips)**
  - This slippage might turn a 20-pip profit into a 5-pip profit

---

### 3. Event Loop & Time Management
**Responsibility**: Orchestrate the flow of data and events in real-time or replay mode.

#### Why This Matters
- **Synchronization**: Ensure orders and fills happen in the correct sequence
- **Latency Simulation**: Add delays to realistic order execution times
- **Replay Mode**: Run historical data at controlled speed for backtesting
- **Real-Time Mode**: Process live ticks as they arrive

#### Implementation Blueprint
```python
import asyncio
from dataclasses import dataclass
from enum import Enum
from typing import Callable, List
import heapq

class EventType(Enum):
    DATA_TICK = 1        # Market data arrives
    ORDER_FILL = 2       # Order filled
    ORDER_TIMEOUT = 3    # Order expires
    RISK_CHECK = 4       # Periodic risk check
    STRATEGY_SIGNAL = 5  # Your strategy generates signal

@dataclass
class Event:
    timestamp: float
    event_type: EventType
    data: dict
    
    def __lt__(self, other):
        # For heap queue (priority by timestamp)
        return self.timestamp < other.timestamp

class SimulationEngine:
    def __init__(self, mode='replay'):
        self.mode = mode  # 'replay' or 'realtime'
        self.event_queue = []  # Min heap by timestamp
        self.current_time = 0
        self.handlers = {}  # event_type -> list of callbacks
        self.running = False
        
    def register_handler(self, event_type: EventType, callback: Callable):
        """Register a callback for an event type."""
        if event_type not in self.handlers:
            self.handlers[event_type] = []
        self.handlers[event_type].append(callback)
    
    def emit_event(self, event: Event):
        """Add event to queue."""
        heapq.heappush(self.event_queue, event)
    
    async def process_events(self):
        """Main event loop."""
        while self.running and self.event_queue:
            event = heapq.heappop(self.event_queue)
            self.current_time = event.timestamp
            
            # Call all handlers for this event type
            if event.event_type in self.handlers:
                for handler in self.handlers[event.event_type]:
                    await handler(event)
            
            # In realtime mode, sleep to match real time
            if self.mode == 'realtime':
                await asyncio.sleep(0.001)  # 1ms minimum
    
    async def run(self):
        """Start simulation."""
        self.running = True
        await self.process_events()
        self.running = False
```

#### Real-Time vs. Replay Mode
- **Replay (Backtesting)**:
  - Process events instantly from historical data
  - Speed up: 1 day of data in 1 second
  - Perfect for iterating strategy logic
  
- **Real-Time (Paper Trading)**:
  - Add small sleeps between events
  - Match wall-clock time
  - Perfect for testing latency-sensitive logic

---

### 4. Strategy Execution Layer
**Responsibility**: Implement your trading strategy and generate orders.

#### What It Needs
- **Access to market data**: Current prices, historical prices, indicators
- **Position tracking**: How many units long/short?
- **Order generation**: When to buy/sell and at what price?
- **Risk limits**: Don't exceed max loss, position size, margin?

#### Implementation Blueprint
```python
import pandas as pd
from datetime import datetime

class ForexStrategy:
    def __init__(self, symbol, capital=100000):
        self.symbol = symbol
        self.capital = capital
        self.position = 0  # Units held
        self.entry_price = None
        self.pnl = 0
        
        # Historical data for indicators
        self.price_history = []
        self.trade_history = []
        
    def on_tick(self, tick):
        """Called when new market data arrives."""
        # Store price for indicator calculation
        self.price_history.append({
            'timestamp': tick['timestamp'],
            'bid': tick['bid'],
            'ask': tick['ask'],
            'mid': (tick['bid'] + tick['ask']) / 2,
        })
        
        # Keep only last N bars for efficiency
        if len(self.price_history) > 1000:
            self.price_history.pop(0)
        
        # Calculate indicators
        if len(self.price_history) >= 20:  # Need 20 bars for MA
            ma20 = self.calculate_ma(20)
            ma50 = self.calculate_ma(50)
            
            # Strategy logic: Golden cross
            if ma20 > ma50 and self.position <= 0:
                # Buy signal
                return self.generate_buy_order(tick)
            elif ma20 < ma50 and self.position > 0:
                # Sell signal
                return self.generate_sell_order(tick)
        
        return None
    
    def calculate_ma(self, period):
        """Calculate moving average."""
        prices = [x['mid'] for x in self.price_history[-period:]]
        return sum(prices) / len(prices)
    
    def generate_buy_order(self, tick):
        """Generate market buy order."""
        order = {
            'id': len(self.trade_history),
            'symbol': self.symbol,
            'side': 'BUY',
            'type': 'MARKET',
            'quantity': 100000,  # 100k units
            'price': tick['ask'],  # Market order at ask
            'timestamp': tick['timestamp'],
        }
        return order
    
    def generate_sell_order(self, tick):
        """Generate market sell order."""
        order = {
            'id': len(self.trade_history),
            'symbol': self.symbol,
            'side': 'SELL',
            'type': 'MARKET',
            'quantity': self.position,  # Close full position
            'price': tick['bid'],  # Market order at bid
            'timestamp': tick['timestamp'],
        }
        return order
    
    def on_fill(self, fill):
        """Called when order fills."""
        if fill['side'] == 'BUY':
            self.position += fill['quantity']
            self.entry_price = fill['price']
        else:  # SELL
            pnl = (fill['price'] - self.entry_price) * fill['quantity']
            self.pnl += pnl
            self.position -= fill['quantity']
        
        self.trade_history.append({
            'timestamp': fill['timestamp'],
            'side': fill['side'],
            'price': fill['price'],
            'quantity': fill['quantity'],
            'pnl': pnl if fill['side'] == 'SELL' else 0,
        })
```

#### Key Strategy Considerations
- **Indicators**: Use TA-Lib or pandas for technical analysis
- **Entry/Exit**: Define clear logic (not just "sell when profit > 2%")
- **Position Sizing**: Fixed size, Kelly criterion, or volatility-adjusted?
- **Risk Management**: Stop-loss? Trailing stop? Max drawdown?

---

### 5. Risk Management & Execution Checks
**Responsibility**: Enforce trading rules and prevent catastrophic losses.

#### What It Enforces
- **Capital limits**: Don't trade more than available capital
- **Margin checks**: Maintain minimum equity for leverage
- **Position limits**: Max 10M EUR/USD per trade? Max 3 concurrent trades?
- **Drawdown limits**: Stop trading if down 5% from peak?
- **Stop-loss enforcement**: Automatic exit if loss exceeds threshold?

#### Implementation Blueprint
```python
class RiskManager:
    def __init__(self, capital, max_position_size=1000000, max_loss_percent=5):
        self.capital = capital
        self.equity = capital
        self.max_position_size = max_position_size
        self.max_loss_percent = max_loss_percent
        self.peak_equity = capital
        
    def check_before_trade(self, order, current_price, current_position):
        """Validate order before execution."""
        checks = {
            'capital_available': True,
            'position_size_ok': True,
            'margin_ok': True,
            'drawdown_ok': True,
            'errors': []
        }
        
        # Check 1: Position size
        if order['quantity'] > self.max_position_size:
            checks['position_size_ok'] = False
            checks['errors'].append(f"Order size {order['quantity']} exceeds limit {self.max_position_size}")
        
        # Check 2: Margin
        margin_required = order['quantity'] * current_price / 100  # Assuming 100:1 leverage
        if margin_required > self.equity:
            checks['margin_ok'] = False
            checks['errors'].append(f"Insufficient margin: {margin_required} > {self.equity}")
        
        # Check 3: Drawdown
        loss_from_peak = (self.peak_equity - self.equity) / self.peak_equity * 100
        if loss_from_peak > self.max_loss_percent:
            checks['drawdown_ok'] = False
            checks['errors'].append(f"Max drawdown exceeded: {loss_from_peak}% > {self.max_loss_percent}%")
        
        return all(checks[k] for k in checks if k != 'errors'), checks
    
    def update_on_fill(self, fill, current_price):
        """Update equity after fill."""
        if fill['side'] == 'SELL':
            pnl = (fill['price'] - fill.get('entry_price', fill['price'])) * fill['quantity']
            self.equity += pnl
            self.peak_equity = max(self.peak_equity, self.equity)
```

---

### 6. Logging & Storage Layer
**Responsibility**: Record everything for analysis and debugging.

#### What to Log
- **Orders**: ID, timestamp, symbol, side, type, price, quantity, status
- **Fills**: Order ID, fill price, fill quantity, timestamp, slippage
- **Positions**: Timestamp, symbol, quantity, entry price, current P&L
- **Trades**: Entry fill, exit fill, total P&L, holding time
- **Events**: Every significant event (order submitted, fill received, risk check failed)

#### Implementation: PostgreSQL
```python
import psycopg2
from psycopg2.extras import Json
import json
from datetime import datetime

class TradeLogger:
    def __init__(self, db_config):
        self.conn = psycopg2.connect(**db_config)
        self.cursor = self.conn.cursor()
        self.create_tables()
    
    def create_tables(self):
        """Create tables if not exist."""
        self.cursor.execute('''
        CREATE TABLE IF NOT EXISTS orders (
            id SERIAL PRIMARY KEY,
            order_id VARCHAR(50),
            symbol VARCHAR(20),
            side VARCHAR(10),
            type VARCHAR(20),
            price DECIMAL(10, 5),
            quantity BIGINT,
            timestamp BIGINT,
            status VARCHAR(20),
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
        ''')
        
        self.cursor.execute('''
        CREATE TABLE IF NOT EXISTS fills (
            id SERIAL PRIMARY KEY,
            order_id VARCHAR(50),
            fill_price DECIMAL(10, 5),
            fill_qty BIGINT,
            timestamp BIGINT,
            slippage DECIMAL(10, 7),
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
        ''')
        
        self.cursor.execute('''
        CREATE TABLE IF NOT EXISTS positions (
            id SERIAL PRIMARY KEY,
            symbol VARCHAR(20),
            quantity BIGINT,
            entry_price DECIMAL(10, 5),
            current_price DECIMAL(10, 5),
            pnl DECIMAL(15, 2),
            timestamp BIGINT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
        ''')
        
        self.conn.commit()
    
    def log_order(self, order_id, symbol, side, order_type, price, quantity, timestamp, status):
        """Log order to database."""
        self.cursor.execute('''
        INSERT INTO orders (order_id, symbol, side, type, price, quantity, timestamp, status)
        VALUES (%s, %s, %s, %s, %s, %s, %s, %s)
        ''', (order_id, symbol, side, order_type, price, quantity, timestamp, status))
        self.conn.commit()
    
    def log_fill(self, order_id, fill_price, fill_qty, timestamp, slippage):
        """Log fill to database."""
        self.cursor.execute('''
        INSERT INTO fills (order_id, fill_price, fill_qty, timestamp, slippage)
        VALUES (%s, %s, %s, %s, %s)
        ''', (order_id, fill_price, fill_qty, timestamp, slippage))
        self.conn.commit()
    
    def get_trade_stats(self):
        """Get summary statistics."""
        self.cursor.execute('''
        SELECT 
            COUNT(*) as total_fills,
            AVG(slippage) as avg_slippage,
            MAX(slippage) as max_slippage,
            MIN(slippage) as min_slippage
        FROM fills;
        ''')
        return self.cursor.fetchone()
```

#### Alternative: In-Memory Logging (Simpler)
```python
class TradeLogger:
    def __init__(self):
        self.orders = []
        self.fills = []
        self.positions = []
        self.trades = []
    
    def log_order(self, **kwargs):
        self.orders.append({**kwargs, 'timestamp': datetime.now()})
    
    def log_fill(self, **kwargs):
        self.fills.append({**kwargs, 'timestamp': datetime.now()})
    
    def to_dataframe(self):
        return {
            'orders': pd.DataFrame(self.orders),
            'fills': pd.DataFrame(self.fills),
            'trades': pd.DataFrame(self.trades),
        }
```

---

### 7. Monitoring & Reporting
**Responsibility**: Visualize performance and identify issues.

#### Key Metrics to Track
- **Cumulative P&L**: Total profit/loss over time
- **Win Rate**: % of winning trades
- **Average Win/Loss**: Avg profit per winning trade vs. losing trade
- **Sharpe Ratio**: Risk-adjusted returns
- **Max Drawdown**: Largest peak-to-trough decline
- **Average Slippage**: How much are orders slipping?
- **Execution Quality**: % of orders that fill vs. expire

#### Implementation: Pandas-based Analysis
```python
class PerformanceAnalyzer:
    def __init__(self, logger):
        self.logger = logger
        self.df_fills = pd.DataFrame(logger.fills)
        self.df_trades = pd.DataFrame(logger.trades)
    
    def calculate_metrics(self):
        """Calculate all key metrics."""
        if self.df_trades.empty:
            return {}
        
        total_pnl = self.df_trades['pnl'].sum()
        winning_trades = self.df_trades[self.df_trades['pnl'] > 0]
        losing_trades = self.df_trades[self.df_trades['pnl'] < 0]
        
        metrics = {
            'total_pnl': total_pnl,
            'num_trades': len(self.df_trades),
            'win_rate': len(winning_trades) / len(self.df_trades) if len(self.df_trades) > 0 else 0,
            'avg_win': winning_trades['pnl'].mean() if len(winning_trades) > 0 else 0,
            'avg_loss': losing_trades['pnl'].mean() if len(losing_trades) > 0 else 0,
            'profit_factor': abs(winning_trades['pnl'].sum() / losing_trades['pnl'].sum()) if len(losing_trades) > 0 else 0,
            'avg_slippage': self.df_fills['slippage'].mean() if not self.df_fills.empty else 0,
        }
        
        return metrics
    
    def plot_results(self):
        """Generate performance plots."""
        import matplotlib.pyplot as plt
        
        fig, axes = plt.subplots(2, 2, figsize=(14, 10))
        
        # Cumulative P&L
        self.df_trades['cumulative_pnl'] = self.df_trades['pnl'].cumsum()
        axes[0, 0].plot(self.df_trades['cumulative_pnl'])
        axes[0, 0].set_title('Cumulative P&L')
        
        # Win/Loss distribution
        axes[0, 1].hist(self.df_trades['pnl'], bins=30, edgecolor='black')
        axes[0, 1].set_title('P&L Distribution')
        
        # Slippage over time
        axes[1, 0].plot(self.df_fills['slippage'])
        axes[1, 0].set_title('Slippage Over Time')
        
        # Monthly returns
        monthly_pnl = self.df_trades.groupby(pd.Grouper(key='timestamp', freq='M'))['pnl'].sum()
        axes[1, 1].bar(range(len(monthly_pnl)), monthly_pnl.values)
        axes[1, 1].set_title('Monthly P&L')
        
        plt.tight_layout()
        plt.savefig('simulation_results.png')
        plt.show()
```

---

## Data Flow & Event Loop

### Complete Flow (Real-Time Simulation)
```
1. [00:00:00] CONNECT to market data feed
   └─> Event: DATA_SOURCE_CONNECTED

2. [00:00:05] Receive EUR/USD tick: bid=1.09501, ask=1.09504
   └─> Event: DATA_TICK (timestamp=1715779205000, data=tick)
   └─> Handler: MarketDataFeed.on_tick()
   └─> Handler: OrderBookManager.add_market_data() [updates order book]
   └─> Handler: Strategy.on_tick() [generates buy signal]
   └─> Event: STRATEGY_SIGNAL (order=BUY 100k @ market)

3. [00:00:05.002] Strategy signal received (2ms delay)
   └─> Event: ORDER_SUBMITTED (order_id=1, side=BUY, qty=100k)
   └─> Handler: RiskManager.check_before_trade() [validates]
   └─> Handler: OrderBookManager.submit_order() [matches against book]
   └─> Fill: 100k @ 1.09504 (ask price)
   └─> Event: ORDER_FILLED (order_id=1, fill_price=1.09504, qty=100k)

4. [00:00:05.003] Fill processed (1ms after fill)
   └─> Handler: Strategy.on_fill() [updates position]
   └─> Handler: TradeLogger.log_fill() [records to DB]
   └─> Handler: RiskManager.update_on_fill() [updates equity]
   └─> Status: Position = +100k EUR/USD @ 1.09504

5. [00:00:30] (25 seconds later) Receive EUR/USD tick: bid=1.09550, ask=1.09553
   └─> Event: DATA_TICK (new bid/ask)
   └─> Handler: Strategy.on_tick() [calculates MA, checks exit]
   └─> Strategy identifies EXIT signal (MA cross)
   └─> Event: STRATEGY_SIGNAL (order=SELL 100k @ market)

6. [00:00:30.002] SELL order submitted
   └─> Event: ORDER_SUBMITTED (order_id=2, side=SELL, qty=100k)
   └─> RiskManager validates
   └─> OrderBookManager.submit_order() [matches]
   └─> Fill: 100k @ 1.09550 (bid price)
   └─> Event: ORDER_FILLED (order_id=2, fill_price=1.09550, qty=100k)

7. [00:00:30.003] Trade completed
   └─> Entry: 100k @ 1.09504
   └─> Exit:  100k @ 1.09550
   └─> Gross P&L: (1.09550 - 1.09504) * 100k = 460 units
   └─> Minus slippage & commissions
   └─> Net P&L: ~400 units
   └─> Position: 0 (flat)

[Repeat from step 2 until end of data]
```

### Implementation Pattern
```python
async def main():
    # Initialize all components
    strategy = ForexStrategy('EUR/USD')
    order_book = OrderBookManager('EUR/USD')
    risk_manager = RiskManager(capital=100000)
    logger = TradeLogger()
    engine = SimulationEngine(mode='replay')
    
    # Register event handlers
    engine.register_handler(EventType.DATA_TICK, on_data_tick)
    engine.register_handler(EventType.ORDER_FILL, on_order_fill)
    engine.register_handler(EventType.STRATEGY_SIGNAL, on_strategy_signal)
    
    # Load historical data
    data = load_historical_ticks('EUR/USD', start_date, end_date)
    
    # Queue all data ticks as events
    for tick in data:
        event = Event(
            timestamp=tick['timestamp'],
            event_type=EventType.DATA_TICK,
            data=tick
        )
        engine.emit_event(event)
    
    # Run simulation
    await engine.run()
    
    # Analyze results
    analyzer = PerformanceAnalyzer(logger)
    metrics = analyzer.calculate_metrics()
    print(metrics)
    analyzer.plot_results()

async def on_data_tick(event):
    tick = event.data
    
    # 1. Update order book
    order_book.add_market_data(tick)
    
    # 2. Run strategy
    order = strategy.on_tick(tick)
    
    # 3. If strategy generated order
    if order:
        # Check risk
        valid, checks = risk_manager.check_before_trade(
            order, 
            tick['ask'],
            strategy.position
        )
        
        if valid:
            # Submit order
            fills = order_book.submit_order(order)
            
            # Log and process fills
            for fill in fills:
                strategy.on_fill(fill)
                logger.log_fill(
                    order_id=order['id'],
                    fill_price=fill.price,
                    fill_qty=fill.quantity,
                    timestamp=tick['timestamp'],
                    slippage=abs(fill.price - tick['ask'])
                )
        else:
            logger.log_error(f"Order rejected: {checks['errors']}")

async def on_order_fill(event):
    # Process fills (called automatically when order matches)
    fill = event.data
    strategy.on_fill(fill)
    risk_manager.update_on_fill(fill, fill['current_price'])
```

---

## Dependencies & Stack

### Technology Stack
```
Language & Runtime:
  ├─ Python 3.12+          (Strategy, orchestration)
  ├─ C++ 20 (ib-trader)     (Order book engine)
  └─ pybind11              (C++-Python bindings)

Build System:
  ├─ CMake 3.21+
  ├─ Ninja (build tool)
  └─ scikit-build-core     (Python package building)

Data Management:
  ├─ pandas                 (Data manipulation, analysis)
  ├─ numpy                  (Numerical operations)
  ├─ sqlite3 or PostgreSQL  (Persistence)
  └─ psycopg2              (PostgreSQL client)

Market Data & Brokers:
  ├─ ib_insync             (Interactive Brokers API)
  ├─ websocket-client      (WebSocket for data feeds)
  ├─ requests              (REST API calls)
  └─ ccxt                  (Crypto exchange APIs, optional)

Technical Analysis:
  ├─ TA-Lib                (Technical indicators)
  └─ pandas-ta             (Alternative to TA-Lib)

Visualization (Optional):
  ├─ matplotlib            (Static plots)
  ├─ plotly                (Interactive plots)
  ├─ streamlit             (Real-time dashboard)
  └─ dash                  (Web-based dashboard)

Testing & Validation:
  ├─ pytest                (Unit testing)
  ├─ pytest-asyncio        (Async testing)
  └─ hypothesis            (Property-based testing)

Performance:
  ├─ asyncio               (Async I/O)
  ├─ aiohttp               (Async HTTP)
  └─ aiokafka              (If using Kafka for data)

Logging & Monitoring:
  ├─ logging               (Built-in Python logging)
  ├─ loguru                (Enhanced logging)
  └─ prometheus-client     (Metrics collection)
```

### Installation
```bash
# 1. Clone and setup ib-trader
git clone https://github.com/xiahualiu/ib-trader.git
cd ib-trader
pip install -r requirements.txt

# 2. Install build tools
pip install cmake ninja scikit-build-core pybind11

# 3. Build ib-trader
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
pip install -e .

# 4. Install simulation dependencies
pip install \
    pandas numpy \
    ib_insync \
    psycopg2-binary \
    ta-lib \
    matplotlib plotly streamlit \
    pytest pytest-asyncio \
    websocket-client \
    python-dotenv

# 5. Setup PostgreSQL (optional but recommended)
# On macOS: brew install postgresql
# On Linux: sudo apt-get install postgresql
# On Windows: Download from https://www.postgresql.org/download/windows/

# Start PostgreSQL
psql -U postgres  # or create a dedicated user
```

### Environment Configuration
```bash
# .env file for your simulation
MARKET_DATA_PROVIDER=IB  # or DUKASCOPY, IQFEED, etc.
IB_ACCOUNT=DU12345      # Your demo account
IB_GATEWAY_IP=127.0.0.1
IB_GATEWAY_PORT=4002
IB_CLIENT_ID=1

DB_HOST=localhost
DB_PORT=5432
DB_USER=postgres
DB_PASSWORD=yourpassword
DB_NAME=forex_simulation

SYMBOL=EUR/USD
TIMEFRAME=1m  # or 5m, 1h, etc.
CAPITAL=100000
MAX_POSITION_SIZE=1000000
MAX_LOSS_PERCENT=5

MODE=replay  # or realtime
START_DATE=2024-01-01
END_DATE=2024-03-31
```

---

## Step-by-Step Implementation

### Phase 1: Setup & Infrastructure (Days 1-2)

#### Step 1.1: Environment Setup
```bash
# Create project structure
mkdir forex-simulator
cd forex-simulator
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt
```

#### Step 1.2: Clone and Build ib-trader
```bash
# Add ib-trader as a git submodule
git submodule add https://github.com/xiahualiu/ib-trader.git libs/ib-trader
cd libs/ib-trader
pip install -e .
cd ../..
```

#### Step 1.3: Setup PostgreSQL (Optional)
```bash
# Create database
createdb forex_simulation

# Create user
psql forex_simulation
> CREATE USER simulator WITH PASSWORD 'yourpassword';
> GRANT ALL PRIVILEGES ON DATABASE forex_simulation TO simulator;
```

#### Step 1.4: Project Structure
```
forex-simulator/
├── config/
│   ├── settings.py        # Configuration constants
│   ├── logging_config.py  # Logging setup
│   └── .env              # Secrets
├── data/
│   ├── historical/       # Historical market data files
│   └── cache/            # Cached data
├── src/
│   ├── __init__.py
│   ├── market_data.py         # Market data feed
│   ├── order_book.py          # Order book management
│   ├── strategy.py            # Your trading strategy
│   ├── risk_manager.py        # Risk checks
│   ├── event_loop.py          # Event orchestration
│   ├── logger.py              # Logging & storage
│   └── simulator.py           # Main engine
├── strategies/
│   ├── momentum.py        # Example: momentum strategy
│   ├── mean_reversion.py  # Example: mean reversion
│   └── your_strategy.py   # Your actual strategy
├── tests/
│   ├── test_order_book.py
│   ├── test_strategy.py
│   ├── test_risk_manager.py
│   └── test_integration.py
├── analysis/
│   ├── performance_analyzer.py
│   └── report_generator.py
├── notebooks/
│   ├── data_exploration.ipynb
│   ├── backtest_comparison.ipynb
│   └── results_analysis.ipynb
├── requirements.txt
├── run_simulation.py       # Main entry point
├── README.md
└── SIMULATION_LOG.md       # Results & notes
```

---

### Phase 2: Core Components (Days 3-5)

#### Step 2.1: Implement Market Data Feed
**File: `src/market_data.py`**
```python
import asyncio
import json
import websocket
from dataclasses import dataclass
from typing import Callable, Optional
import logging

logger = logging.getLogger(__name__)

@dataclass
class Tick:
    symbol: str
    timestamp: int  # milliseconds
    bid: float
    ask: float
    bid_size: int
    ask_size: int
    volume: int

class MarketDataFeed:
    def __init__(self, symbol: str, data_source: str = 'IB'):
        self.symbol = symbol
        self.data_source = data_source
        self.current_tick: Optional[Tick] = None
        self.tick_history = []
        self.on_tick_callback: Optional[Callable] = None
        self.connected = False
    
    async def connect(self):
        """Connect to market data provider."""
        if self.data_source == 'IB':
            await self._connect_ib()
        elif self.data_source == 'websocket':
            await self._connect_websocket()
        elif self.data_source == 'file':
            # For backtesting with historical files
            pass
    
    async def _connect_ib(self):
        """Connect to Interactive Brokers via ib_insync."""
        from ib_insync import IB
        
        self.ib = IB()
        await self.ib.connect('127.0.0.1', 4002, clientId=1)
        logger.info(f"Connected to IB gateway for {self.symbol}")
        self.connected = True
    
    async def subscribe(self, callback: Callable):
        """Subscribe to tick updates."""
        self.on_tick_callback = callback
    
    async def get_ticks(self, filepath: str):
        """Load historical ticks from file for backtesting."""
        import pandas as pd
        
        df = pd.read_csv(filepath)
        for row in df.itertuples():
            tick = Tick(
                symbol=self.symbol,
                timestamp=int(row.timestamp),
                bid=row.bid,
                ask=row.ask,
                bid_size=int(row.bid_size),
                ask_size=int(row.ask_size),
                volume=int(row.volume)
            )
            yield tick
    
    async def _process_tick(self, tick: Tick):
        """Process incoming tick."""
        self.current_tick = tick
        self.tick_history.append(tick)
        
        # Keep last 10k ticks only
        if len(self.tick_history) > 10000:
            self.tick_history.pop(0)
        
        # Call registered callback
        if self.on_tick_callback:
            await self.on_tick_callback(tick)
```

#### Step 2.2: Implement Order Book Manager
**File: `src/order_book.py`**
```python
from ibalgotrade import LimitOrderBook, Order, OrderType, Side
from dataclasses import dataclass
from typing import List, Dict, Optional
import logging

logger = logging.getLogger(__name__)

@dataclass
class Fill:
    order_id: str
    counter_order_id: str
    price: float
    quantity: int
    timestamp: int

class OrderBookManager:
    def __init__(self, symbol: str):
        self.symbol = symbol
        self.book = LimitOrderBook(symbol)
        self.pending_orders: Dict[str, Order] = {}
        self.fills: List[Fill] = []
        self.next_liquidity_id = 1000000000  # Reserve high IDs for market ticks
    
    def add_market_data_tick(self, tick):
        """Update order book with market data."""
        # Add bid-side liquidity
        bid_order = Order()
        bid_order.id = self.next_liquidity_id
        bid_order.symbol = self.symbol
        bid_order.side = Side.Buy
        bid_order.type = OrderType.Limit
        bid_order.price = tick.bid
        bid_order.quantity = tick.bid_size
        
        self.book.add(bid_order)
        self.next_liquidity_id += 1
        
        # Add ask-side liquidity
        ask_order = Order()
        ask_order.id = self.next_liquidity_id
        ask_order.symbol = self.symbol
        ask_order.side = Side.Sell
        ask_order.type = OrderType.Limit
        ask_order.price = tick.ask
        ask_order.quantity = tick.ask_size
        
        self.book.add(ask_order)
        self.next_liquidity_id += 1
        
        logger.debug(f"Updated order book: bid={tick.bid}({tick.bid_size}), ask={tick.ask}({tick.ask_size})")
    
    def submit_order(self, order_id: str, side: str, order_type: str, price: float, quantity: int, timestamp: int) -> List[Fill]:
        """Submit order to order book."""
        order = Order()
        order.id = hash(order_id)  # Convert to integer
        order.symbol = self.symbol
        order.side = Side.Buy if side == 'BUY' else Side.Sell
        order.type = OrderType.Market if order_type == 'MARKET' else OrderType.Limit
        order.price = price
        order.quantity = quantity
        order.timestamp = timestamp
        
        # Add to book and get fills
        cpp_fills = self.book.add(order)
        
        # Convert C++ fills to Python dataclass
        fills = []
        for cpp_fill in cpp_fills:
            fill = Fill(
                order_id=order_id,
                counter_order_id=str(cpp_fill.counter_order_id),
                price=cpp_fill.price,
                quantity=cpp_fill.quantity,
                timestamp=timestamp
            )
            fills.append(fill)
        
        self.fills.extend(fills)
        self.pending_orders[order_id] = order
        
        logger.info(f"Order {order_id}: {side} {quantity} @ {price} -> {len(fills)} fill(s)")
        return fills
    
    def cancel_order(self, order_id: str):
        """Cancel resting order."""
        if order_id in self.pending_orders:
            order = self.pending_orders[order_id]
            self.book.cancel(order.id)
            del self.pending_orders[order_id]
            logger.info(f"Order {order_id} canceled")
    
    def get_book_state(self):
        """Get current order book state."""
        return {
            'bid': self.book.best_bid(),
            'ask': self.book.best_ask(),
            'bid_size': self.book.bid_size(),
            'ask_size': self.book.ask_size(),
            'bid_levels': self.book.bid_levels(),
            'ask_levels': self.book.ask_levels(),
        }
```

#### Step 2.3: Implement Event Loop
**File: `src/event_loop.py`**
```python
import asyncio
import heapq
from dataclasses import dataclass
from enum import Enum
from typing import Callable, List, Dict
import logging

logger = logging.getLogger(__name__)

class EventType(Enum):
    DATA_TICK = 1
    ORDER_SUBMITTED = 2
    ORDER_FILLED = 3
    STRATEGY_SIGNAL = 4
    RISK_CHECK = 5
    TIMER = 6

@dataclass
class Event:
    timestamp: int  # milliseconds
    event_type: EventType
    data: dict
    
    def __lt__(self, other):
        return self.timestamp < other.timestamp

class SimulationEngine:
    def __init__(self, mode='replay', speed=1.0):
        self.mode = mode  # 'replay' or 'realtime'
        self.speed = speed  # 1.0 = real-time, 10.0 = 10x faster
        self.event_queue: List[Event] = []
        self.current_time = 0
        self.handlers: Dict[EventType, List[Callable]] = {}
        self.running = False
        self.stats = {
            'events_processed': 0,
            'start_time': None,
            'end_time': None,
        }
    
    def register_handler(self, event_type: EventType, handler: Callable):
        """Register async handler for event type."""
        if event_type not in self.handlers:
            self.handlers[event_type] = []
        self.handlers[event_type].append(handler)
        logger.info(f"Registered handler for {event_type.name}")
    
    def emit_event(self, event: Event):
        """Add event to queue."""
        heapq.heappush(self.event_queue, event)
    
    async def run(self):
        """Main event loop."""
        self.running = True
        self.stats['start_time'] = self.current_time
        
        logger.info(f"Starting simulation ({self.mode} mode, {self.speed}x speed)")
        
        while self.running and self.event_queue:
            event = heapq.heappop(self.event_queue)
            self.current_time = event.timestamp
            
            # Process all handlers for this event
            if event.event_type in self.handlers:
                for handler in self.handlers[event.event_type]:
                    try:
                        await handler(event)
                    except Exception as e:
                        logger.error(f"Handler error: {e}", exc_info=True)
            
            self.stats['events_processed'] += 1
            
            # In realtime mode, respect wall-clock time
            if self.mode == 'realtime':
                await asyncio.sleep(0.001 / self.speed)
            
            # Progress logging
            if self.stats['events_processed'] % 10000 == 0:
                logger.info(f"Processed {self.stats['events_processed']} events")
        
        self.stats['end_time'] = self.current_time
        logger.info(f"Simulation complete. Processed {self.stats['events_processed']} events")
        self.running = False
    
    def stop(self):
        """Stop simulation."""
        self.running = False
```

#### Step 2.4: Implement Strategy Base Class
**File: `src/strategy.py`**
```python
from abc import ABC, abstractmethod
from dataclasses import dataclass
import logging

logger = logging.getLogger(__name__)

@dataclass
class StrategyState:
    position: int = 0
    entry_price: float = 0.0
    unrealized_pnl: float = 0.0
    realized_pnl: float = 0.0
    trades: list = None
    
    def __post_init__(self):
        if self.trades is None:
            self.trades = []

class StrategyBase(ABC):
    def __init__(self, symbol: str, capital: float = 100000):
        self.symbol = symbol
        self.capital = capital
        self.equity = capital
        self.state = StrategyState()
        self.order_counter = 0
    
    @abstractmethod
    async def on_tick(self, tick):
        """Called on each market data tick. Should return order or None."""
        pass
    
    async def on_fill(self, fill):
        """Called when order fills."""
        logger.info(f"Fill: {fill.side} {fill.quantity} @ {fill.price}")
        
        if fill.side == 'BUY':
            self.state.position += fill.quantity
            self.state.entry_price = fill.price
        else:  # SELL
            pnl = (fill.price - self.state.entry_price) * fill.quantity
            self.state.realized_pnl += pnl
            self.equity += pnl
            self.state.trades.append({
                'entry': self.state.entry_price,
                'exit': fill.price,
                'qty': fill.quantity,
                'pnl': pnl,
            })
            self.state.position = 0
    
    def generate_order(self, side: str, order_type: str, price: float, quantity: int) -> dict:
        """Generate order dict."""
        self.order_counter += 1
        return {
            'id': f"{self.symbol}_{self.order_counter}",
            'symbol': self.symbol,
            'side': side,
            'type': order_type,
            'price': price,
            'quantity': quantity,
        }
```

---

### Phase 3: Integration (Days 6-7)

#### Step 3.1: Main Simulator Class
**File: `src/simulator.py`**
```python
import asyncio
import logging
from datetime import datetime
from src.event_loop import SimulationEngine, Event, EventType
from src.market_data import MarketDataFeed
from src.order_book import OrderBookManager
from src.strategy import StrategyBase
from src.risk_manager import RiskManager
from src.logger import TradeLogger

logger = logging.getLogger(__name__)

class ForexSimulator:
    def __init__(self, strategy: StrategyBase, config: dict):
        self.strategy = strategy
        self.config = config
        
        # Initialize components
        self.engine = SimulationEngine(
            mode=config.get('mode', 'replay'),
            speed=config.get('speed', 1.0)
        )
        self.market_data = MarketDataFeed(config['symbol'])
        self.order_book = OrderBookManager(config['symbol'])
        self.risk_manager = RiskManager(
            capital=config['capital'],
            max_position_size=config['max_position_size'],
            max_loss_percent=config['max_loss_percent']
        )
        self.trade_logger = TradeLogger(config.get('db_config'))
        
        # Register event handlers
        self._register_handlers()
    
    def _register_handlers(self):
        """Register all event handlers."""
        self.engine.register_handler(EventType.DATA_TICK, self._on_data_tick)
        self.engine.register_handler(EventType.ORDER_FILLED, self._on_order_filled)
    
    async def _on_data_tick(self, event: Event):
        """Handle market data tick."""
        tick = event.data
        
        # Update order book with market data
        self.order_book.add_market_data_tick(tick)
        
        # Run strategy
        order = await self.strategy.on_tick(tick)
        
        # If strategy generated an order
        if order:
            # Check risk
            valid, checks = self.risk_manager.check_before_trade(
                order,
                tick.ask if order['side'] == 'BUY' else tick.bid,
                self.strategy.state.position
            )
            
            if not valid:
                logger.warning(f"Order rejected: {checks['errors']}")
                self.trade_logger.log_rejection(order, checks['errors'])
                return
            
            # Submit order to order book
            fills = self.order_book.submit_order(
                order_id=order['id'],
                side=order['side'],
                order_type=order['type'],
                price=order['price'],
                quantity=order['quantity'],
                timestamp=tick.timestamp
            )
            
            # Process fills
            for fill in fills:
                await self.strategy.on_fill(fill)
                self.risk_manager.update_on_fill(fill, tick.bid if fill.side == 'SELL' else tick.ask)
                self.trade_logger.log_fill(fill, tick.timestamp)
            
            # Log order
            self.trade_logger.log_order(order, tick.timestamp, len(fills) > 0)
    
    async def _on_order_filled(self, event: Event):
        """Handle order fill event."""
        fill = event.data
        await self.strategy.on_fill(fill)
    
    async def run_replay(self, data_file: str):
        """Run simulation on historical data."""
        logger.info(f"Starting replay from {data_file}")
        
        # Load historical ticks
        for tick in await self.market_data.get_ticks(data_file):
            event = Event(
                timestamp=tick.timestamp,
                event_type=EventType.DATA_TICK,
                data=tick
            )
            self.engine.emit_event(event)
        
        # Run engine
        await self.engine.run()
        
        # Analyze results
        self._print_results()
    
    async def run_realtime(self):
        """Run simulation on live market data."""
        logger.info("Starting real-time simulation")
        
        # Connect to market data
        await self.market_data.connect()
        
        # Subscribe to ticks
        await self.market_data.subscribe(self._on_market_data_tick)
        
        # Run engine
        await self.engine.run()
    
    async def _on_market_data_tick(self, tick):
        """Called when new market data arrives."""
        event = Event(
            timestamp=tick.timestamp,
            event_type=EventType.DATA_TICK,
            data=tick
        )
        self.engine.emit_event(event)
    
    def _print_results(self):
        """Print simulation results."""
        metrics = self.trade_logger.get_metrics()
        
        print("\n" + "="*50)
        print("SIMULATION RESULTS")
        print("="*50)
        print(f"Total Trades: {metrics['total_trades']}")
        print(f"Winning Trades: {metrics['winning_trades']}")
        print(f"Losing Trades: {metrics['losing_trades']}")
        print(f"Win Rate: {metrics['win_rate']:.2%}")
        print(f"Gross P&L: {metrics['gross_pnl']:.2f}")
        print(f"Net P&L: {metrics['net_pnl']:.2f}")
        print(f"Sharpe Ratio: {metrics['sharpe_ratio']:.2f}")
        print(f"Max Drawdown: {metrics['max_drawdown']:.2%}")
        print(f"Avg Slippage: {metrics['avg_slippage']:.6f}")
        print("="*50 + "\n")
```

---

## Integration with ib-trader Repo

### How the C++ Core Fits In
The ib-trader repository provides:
1. **LimitOrderBook class**: Maintains bid/ask levels, implements price-time priority matching
2. **Order, Fill, PriceLevel classes**: Data structures for orders and fills
3. **Python bindings (pybind11)**: Call C++ code from Python

### Usage Pattern
```python
from ibalgotrade import LimitOrderBook, Order, OrderType, Side

# Create order book
book = LimitOrderBook("EUR/USD")

# Add liquidity (from market data)
bid_order = Order()
bid_order.id = 1
bid_order.side = Side.Buy
bid_order.type = OrderType.Limit
bid_order.price = 1.0950
bid_order.quantity = 10000000
book.add(bid_order)

# Submit strategy order
strategy_order = Order()
strategy_order.id = 2
strategy_order.side = Side.Buy
strategy_order.type = OrderType.Market
strategy_order.quantity = 5000000
fills = book.add(strategy_order)

# Check fills
for fill in fills:
    print(f"Filled {fill.quantity} @ {fill.price}")
```

### Customization Options
If you want to modify the order matching logic:
1. Edit `include/ibalgotrade/core/order.hpp` (header)
2. Edit `src/core/order.cpp` (implementation)
3. Rebuild: `cmake --build build`
4. Reinstall: `pip install -e .`

Example customizations:
- Add order timeouts (orders expire after N seconds)
- Add market impact (large orders move the market)
- Add partial fills at different levels
- Add commission/slippage calculations

---

## Testing & Validation

### Unit Tests
```python
# tests/test_order_book.py
import pytest
from src.order_book import OrderBookManager
from src.market_data import Tick

@pytest.fixture
def order_book():
    return OrderBookManager("EUR/USD")

@pytest.fixture
def sample_tick():
    return Tick(
        symbol="EUR/USD",
        timestamp=1715779200000,
        bid=1.09500,
        ask=1.09504,
        bid_size=10000000,
        ask_size=15000000,
        volume=25000000,
    )

def test_market_data_update(order_book, sample_tick):
    order_book.add_market_data_tick(sample_tick)
    state = order_book.get_book_state()
    
    assert state['bid'] == 1.09500
    assert state['ask'] == 1.09504
    assert state['bid_size'] == 10000000
    assert state['ask_size'] == 15000000

def test_market_order_execution(order_book, sample_tick):
    order_book.add_market_data_tick(sample_tick)
    fills = order_book.submit_order(
        order_id="test_1",
        side="BUY",
        order_type="MARKET",
        price=sample_tick.ask,
        quantity=5000000,
        timestamp=sample_tick.timestamp
    )
    
    assert len(fills) == 1
    assert fills[0].price == sample_tick.ask
    assert fills[0].quantity == 5000000

def test_limit_order_rest(order_book, sample_tick):
    order_book.add_market_data_tick(sample_tick)
    fills = order_book.submit_order(
        order_id="test_2",
        side="BUY",
        order_type="LIMIT",
        price=sample_tick.bid - 0.00050,  # Below bid
        quantity=5000000,
        timestamp=sample_tick.timestamp
    )
    
    assert len(fills) == 0  # No fill; order rests
```

### Integration Tests
```python
# tests/test_integration.py
@pytest.mark.asyncio
async def test_full_simulation():
    # Setup
    from strategies.example import SimpleStrategy
    config = {
        'symbol': 'EUR/USD',
        'capital': 100000,
        'max_position_size': 1000000,
        'max_loss_percent': 5,
        'mode': 'replay',
    }
    
    strategy = SimpleStrategy()
    simulator = ForexSimulator(strategy, config)
    
    # Run
    await simulator.run_replay('data/test_data.csv')
    
    # Verify
    assert simulator.strategy.state.position == 0  # Flat at end
    assert simulator.trade_logger.total_trades > 0
    assert simulator.trade_logger.net_pnl > 0  # Profitable
```

### Validation Checklist
- [ ] Market data ingestion works
- [ ] Order book updates correctly
- [ ] Market orders fill immediately at bid/ask
- [ ] Limit orders rest when not crossed
- [ ] Partial fills work correctly
- [ ] Slippage is calculated accurately
- [ ] Strategy generates expected trades
- [ ] Risk manager rejects bad orders
- [ ] Logging captures all data
- [ ] Results match manual calculation

---

## Real-World Considerations

### 1. Data Quality
**Problem**: Garbage in, garbage out
**Solutions**:
- Use high-quality L2 data (Interactive Brokers, Dukascopy)
- Validate data (no crazy gaps, outliers)
- Handle missing data (market close, outages)
- Verify bid-ask spread is reasonable

### 2. Latency Simulation
**Problem**: Your backtest assumes instant fills; real life has latency
**Solutions**:
- Add realistic delays (50-200ms for broker round trips)
- Add network jitter (variable delays)
- Simulate order rejections (rare but real)
- Add queue position uncertainty (your limit order might not fill)

Example:
```python
async def add_latency(self, order_id, fill, latency_ms=100):
    """Simulate network latency before fill."""
    await asyncio.sleep(latency_ms / 1000)
    await self.on_fill(fill)
```

### 3. Slippage Realism
**Problem**: Actual fills vary from theoretical fills
**Solutions**:
- Model market impact (large orders get worse fills)
- Add bid-ask bounce (fill at mid, then market moves)
- Simulate liquidity changes (sudden size reduction)
- Add volatility-adjusted slippage

### 4. Partial Fill Handling
**Problem**: Strategy assumes fills are all-or-nothing
**Solutions**:
- Track partial fills correctly
- Adjust position in phases
- Calculate average entry price
- Handle remaining quantity

### 5. Commission & Fees
**Problem**: Backtest often ignores costs
**Solutions**:
- Add realistic broker commissions
- Add ECN fees
- Add currency conversion costs (if multi-currency)
- Track total cost

Example:
```python
def calculate_total_cost(self, fills):
    total = 0
    for fill in fills:
        execution_cost = fill.quantity * fill.price
        commission = execution_cost * 0.0001  # 0.01% commission
        total += commission
    return total
```

### 6. Gap Handling
**Problem**: Markets gap (EUR/USD opens 20 pips away from close)
**Solutions**:
- Don't fill resting orders across gaps
- Update order book atomically
- Log gap events

### 7. Calendar/Holiday Effects
**Problem**: Markets close/are illiquid on certain days
**Solutions**:
- Skip non-trading hours
- Handle weekend gaps
- Increase slippage during low-liquidity hours

### 8. Stress Testing
Run simulations under various market conditions:
- **Trending markets**: Does strategy follow trends?
- **Choppy markets**: Does it get whipsawed?
- **High volatility**: Does it blow up?
- **Low liquidity**: Does slippage kill returns?
- **Flash crashes**: Does it handle gaps?

---

## Common Pitfalls

### 1. Ignoring Bid-Ask Spread
**Mistake**: Buying at mid-price (1.0950), selling at mid-price
**Reality**: Buy at ask (1.09504), sell at bid (1.09501). Instantly down 3 pips.
**Fix**: Always buy at ask, sell at bid in your strategy

### 2. Assuming Instant Fills
**Mistake**: "Place limit order at 1.0950, it'll fill within 1ms"
**Reality**: Limit order might rest for 10 seconds or never fill
**Fix**: Implement timeout logic and fallback to market orders

### 3. Not Accounting for Slippage
**Mistake**: Backtest shows 2% per trade; live you get 0.5%
**Reality**: Market orders sweep through multiple levels; slippage varies
**Fix**: Measure actual slippage in simulation, subtract from backtest returns

### 4. Ignoring Order Rejection
**Mistake**: "I have $100k, I'll trade $150k" (overleveraged)
**Reality**: Broker rejects order; trade never happens
**Fix**: Implement margin checks and position limits

### 5. Not Handling Partial Fills
**Mistake**: Strategy assumes 100k units fill instantly
**Reality**: Fills 50k @ 1.0950, 50k @ 1.0951. Average entry = 1.09505
**Fix**: Track partial fills, calculate average prices

### 6. Ignoring Latency
**Mistake**: "Order fills instantly when I click"
**Reality**: 100ms to send order, 50ms to fill, 50ms to receive notification = 200ms total
**Fix**: Add realistic latency to simulation (100-500ms depending on broker)

### 7. Backtesting on False Signals
**Mistake**: "MA cross looks good in backtest"
**Reality**: Repaints due to incomplete candles
**Fix**: Only signal on bar close, not intrabar updates

### 8. Not Validating Against Reality
**Mistake**: Simulation shows 100% win rate
**Reality**: Something's wrong (probably data issues or unrealistic assumptions)
**Fix**: Compare simulation results to actual paper trading, find discrepancies

---

## Next Steps: Moving to Actual Paper Trading

Once your simulation shows promising results:

### 1. Paper Trading Setup
```python
# Switch from simulation to paper trading
from ib_insync import IB, Stock, Order as IBOrder
from ib_insync import LimitOrder, MarketOrder

ib = IB()
ib.connect('127.0.0.1', 4002, clientId=1)  # TWS demo account

# Place real paper order
contract = Stock('EURUSD', 'IDEALPRO')
order = MarketOrder('BUY', 100000)
trade = ib.placeOrder(contract, order)

# Monitor fill
while not trade.isDone():
    print(trade.status)
    await asyncio.sleep(0.1)

print(f"Filled at {trade.orderStatus.avgFillPrice}")
```

### 2. Monitoring & Alerts
```python
class PaperTradingMonitor:
    def __init__(self):
        self.alerts = []
    
    def check_deviation(self, simulated_price, actual_price, threshold=0.0010):
        """Alert if actual price deviates from simulation."""
        dev = abs(actual_price - simulated_price) / simulated_price
        if dev > threshold:
            self.alerts.append(f"⚠️ Price deviation: sim={simulated_price}, actual={actual_price}, dev={dev:.4%}")
    
    def check_fill_rate(self, simulated_fill_rate, actual_fill_rate, threshold=0.1):
        """Alert if fill rate differs significantly."""
        diff = abs(simulated_fill_rate - actual_fill_rate)
        if diff > threshold:
            self.alerts.append(f"⚠️ Fill rate deviation: sim={simulated_fill_rate:.1%}, actual={actual_fill_rate:.1%}")
```

### 3. Gradual Ramp-Up
- Week 1-2: Paper trading with small size (10% of intended)
- Week 3-4: Increase to 50%
- Week 5+: Full size (if all matches simulation)

### 4. Post-Trade Analysis
```python
# Compare paper trading results to simulation
def compare_results(simulation_log, paper_log):
    sim_trades = pd.read_csv(simulation_log)
    paper_trades = pd.read_csv(paper_log)
    
    comparison = {
        'sim_pnl': sim_trades['pnl'].sum(),
        'paper_pnl': paper_trades['pnl'].sum(),
        'sim_sharpe': calculate_sharpe(sim_trades),
        'paper_sharpe': calculate_sharpe(paper_trades),
        'sim_win_rate': (sim_trades['pnl'] > 0).mean(),
        'paper_win_rate': (paper_trades['pnl'] > 0).mean(),
    }
    
    print(comparison)
    
    # Flag deviations
    if abs(comparison['paper_pnl'] - comparison['sim_pnl']) > 0.1 * comparison['sim_pnl']:
        print("⚠️ P&L deviation detected! Investigate.")
```

---

## Summary Checklist

Before starting development, ensure you have:

### Prerequisites
- [ ] Strategy backtested and profitable
- [ ] Access to real-time L2 market data API
- [ ] Python 3.12+ environment set up
- [ ] Git installed (for ib-trader submodule)
- [ ] PostgreSQL installed (optional but recommended)
- [ ] C++ compiler installed (Clang, GCC, or MSVC)

### Components to Build
- [ ] Market data feed ingestion
- [ ] Order book manager with ib-trader core
- [ ] Event loop & time management
- [ ] Strategy execution layer
- [ ] Risk management engine
- [ ] Logging & storage layer
- [ ] Performance analysis tools

### Testing Before Paper Trading
- [ ] Unit tests for each component
- [ ] Integration tests for full simulation
- [ ] Validation against manual calculations
- [ ] Replay tests on historical data
- [ ] Stress tests (gaps, volatility, liquidity)

### Documentation
- [ ] Strategy logic documented
- [ ] Order flow diagram
- [ ] Data dictionary (what each field means)
- [ ] Configuration guide
- [ ] Troubleshooting guide

---

## Final Notes

This is a **complete blueprint**. You now have:
1. **Architecture**: How components interact
2. **Implementation**: Code structure and patterns
3. **Dependencies**: Exact stack with versions
4. **Data Flow**: End-to-end event sequence
5. **Real-World Issues**: Common problems and fixes
6. **Testing**: How to validate results
7. **Next Steps**: Moving to actual trading

**Timeline**: 1-2 weeks to build, 2-4 weeks to validate, then paper trading.

Start with Phase 1 (infrastructure), then Phase 2 (components), then Phase 3 (integration). Test at each step.

Good luck! 🚀

