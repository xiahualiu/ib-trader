# Forex Market Data API: Real-Time Streaming Service

**Standalone Local API for Forex 1-Minute Data Streaming**

---

## Overview

Create a **local REST API** that serves real-time forex market data from CSV files. The API streams 1-minute OHLCV (Open, High, Low, Close, Volume) data for forex pairs, simulating live market data feeds with configurable latency.

### Key Features
- ✅ **CSV Data Ingestion**: Loads yearly 1-minute forex data from CSV files
- ✅ **Real-Time Streaming**: WebSocket and REST endpoints for live data
- ✅ **Manual Latency Control**: Adjustable delays for testing different network conditions
- ✅ **Multiple Pairs**: Support for EUR/USD, GBP/USD, USD/JPY, etc.
- ✅ **Historical Replay**: Stream historical data at controlled speeds
- ✅ **Standalone Service**: Independent from trading simulator
- ✅ **FastAPI Framework**: Modern, async Python API

### Use Case
This API feeds your live-like trading simulator. Instead of connecting to expensive real-time data providers, you stream historical data as if it were live, with controllable latency to test how your strategy handles different network conditions.

---

## Requirements

### Dependencies
```bash
pip install \
    fastapi==0.104.1 \
    uvicorn[standard]==0.24.0 \
    pandas==2.1.3 \
    numpy==1.26.2 \
    websockets==12.0 \
    pydantic==2.5.0 \
    python-multipart==0.0.6 \
    aiofiles==23.2.1
```

### Data Format
Your CSV files must be in this exact format:

**File Structure**: `data/{pair}/{year}.csv`
```
data/
├── EURUSD/
│   ├── 2020.csv
│   ├── 2021.csv
│   └── 2022.csv
├── GBPUSD/
│   ├── 2020.csv
│   ├── 2021.csv
│   └── 2022.csv
└── USDJPY/
    ├── 2020.csv
    ├── 2021.csv
    └── 2022.csv
```

**CSV Format** (no header row):
```
timestamp,open,high,low,close,volume
1609459200000,1.2215,1.2220,1.2210,1.2218,125000
1609459260000,1.2218,1.2225,1.2215,1.2222,98000
1609459320000,1.2222,1.2228,1.2220,1.2225,156000
```

- **timestamp**: Unix timestamp in milliseconds (UTC)
- **open/high/low/close**: Prices as floats (5 decimal places for majors)
- **volume**: Trading volume as integer

### System Requirements
- Python 3.9+
- 2GB RAM minimum (for loading multiple years of data)
- Fast storage (SSD recommended for quick data loading)

---

## API Architecture

### Core Components

#### 1. Data Manager
- Loads CSV files into memory (pandas DataFrames)
- Pre-processes data for fast access
- Handles multiple pairs and years
- Memory-efficient storage

#### 2. Latency Controller
- Configurable delays between data points
- Simulate network latency (0ms to 5000ms)
- Speed control (0.1x to 10x real-time)
- Jitter simulation (variable delays)

#### 3. Streaming Engine
- WebSocket server for real-time data
- REST endpoints for historical data
- Subscription management
- Connection pooling

#### 4. API Server
- FastAPI application
- Async endpoints
- Request validation
- Error handling

### Data Flow
```
CSV Files → Data Manager (loads into memory)
    ↓
Latency Controller (adds delays)
    ↓
Streaming Engine (WebSocket/REST)
    ↓
Trading Simulator (consumes data)
```

---

## API Endpoints

### REST Endpoints

#### GET `/health`
Check API status.
```json
{
  "status": "healthy",
  "pairs": ["EURUSD", "GBPUSD", "USDJPY"],
  "latency_ms": 100,
  "speed": 1.0
}
```

#### GET `/pairs`
List available forex pairs.
```json
{
  "pairs": ["EURUSD", "GBPUSD", "USDJPY"],
  "count": 3
}
```

#### GET `/pairs/{pair}`
Get pair information.
```json
{
  "pair": "EURUSD",
  "years": [2020, 2021, 2022],
  "total_bars": 525600,
  "date_range": {
    "start": "2020-01-01T00:00:00Z",
    "end": "2022-12-31T23:59:00Z"
  }
}
```

#### GET `/data/{pair}/latest`
Get latest bar for a pair.
```json
{
  "timestamp": 1609459200000,
  "open": 1.2215,
  "high": 1.2220,
  "low": 1.2210,
  "close": 1.2218,
  "volume": 125000,
  "pair": "EURUSD"
}
```

#### GET `/data/{pair}/historical?start=2020-01-01&end=2020-01-02&limit=100`
Get historical bars.
```json
{
  "pair": "EURUSD",
  "bars": [
    {
      "timestamp": 1609459200000,
      "open": 1.2215,
      "high": 1.2220,
      "low": 1.2210,
      "close": 1.2218,
      "volume": 125000
    }
  ],
  "count": 1440,
  "start_date": "2020-01-01T00:00:00Z",
  "end_date": "2020-01-02T00:00:00Z"
}
```

#### POST `/latency/set`
Set latency parameters.
```json
{
  "base_latency_ms": 100,
  "jitter_ms": 20,
  "speed_multiplier": 1.0
}
```
Response:
```json
{
  "status": "updated",
  "base_latency_ms": 100,
  "jitter_ms": 20,
  "speed_multiplier": 1.0
}
```

#### GET `/latency/get`
Get current latency settings.
```json
{
  "base_latency_ms": 100,
  "jitter_ms": 20,
  "speed_multiplier": 1.0,
  "effective_latency_range": "80-120ms"
}
```

### WebSocket Endpoints

#### `/ws/market/{pair}`
Real-time streaming for a specific pair.

**Connection**: `ws://localhost:8000/ws/market/EURUSD`

**Messages Sent**:
```json
{
  "type": "bar",
  "data": {
    "timestamp": 1609459200000,
    "open": 1.2215,
    "high": 1.2220,
    "low": 1.2210,
    "close": 1.2218,
    "volume": 125000,
    "pair": "EURUSD"
  }
}
```

**Messages Received**:
```json
{
  "action": "subscribe",
  "pair": "EURUSD"
}
```

#### `/ws/market/all`
Stream all pairs simultaneously.

**Connection**: `ws://localhost:8000/ws/market/all`

**Messages Sent**:
```json
{
  "type": "bars",
  "data": {
    "EURUSD": {
      "timestamp": 1609459200000,
      "open": 1.2215,
      "high": 1.2220,
      "low": 1.2210,
      "close": 1.2218,
      "volume": 125000
    },
    "GBPUSD": {
      "timestamp": 1609459200000,
      "open": 1.3750,
      "high": 1.3755,
      "low": 1.3745,
      "close": 1.3752,
      "volume": 98000
    }
  }
}
```

---

## Latency Control System

### How It Works
The API simulates network latency between your trading simulator and the "market data provider". This tests how your strategy handles:

- **Slow connections**: 500-2000ms latency
- **Fast connections**: 10-50ms latency
- **Variable delays**: Jitter simulation
- **Speed control**: Replay historical data faster/slower than real-time

### Configuration Parameters

#### Base Latency (`base_latency_ms`)
- **Type**: Integer (0-5000)
- **Default**: 100
- **Description**: Minimum delay between data points in milliseconds
- **Use Case**: Simulate network round-trip time

#### Jitter (`jitter_ms`)
- **Type**: Integer (0-1000)
- **Default**: 20
- **Description**: Random variation added to base latency
- **Use Case**: Simulate network variability

#### Speed Multiplier (`speed_multiplier`)
- **Type**: Float (0.1-10.0)
- **Default**: 1.0
- **Description**: How fast to stream data relative to real-time
- **Use Case**: 
  - 0.1 = 10x slower (for debugging)
  - 1.0 = real-time
  - 2.0 = 2x faster (for quick testing)

### Effective Latency Calculation
```
effective_delay = (base_latency_ms + random(-jitter_ms, +jitter_ms)) / speed_multiplier
```

**Examples**:
- `base=100, jitter=20, speed=1.0` → 80-120ms delays
- `base=500, jitter=100, speed=2.0` → 200-300ms delays (but 2x faster streaming)
- `base=0, jitter=0, speed=10.0` → 0ms delays, 10x faster streaming

### Setting Latency Programmatically
```python
import requests

# Set high latency (slow connection simulation)
requests.post("http://localhost:8000/latency/set", json={
    "base_latency_ms": 500,
    "jitter_ms": 100,
    "speed_multiplier": 1.0
})

# Set fast streaming (quick testing)
requests.post("http://localhost:8000/latency/set", json={
    "base_latency_ms": 10,
    "jitter_ms": 5,
    "speed_multiplier": 5.0
})
```

---

## Implementation

### Project Structure
```
forex-data-api/
├── app/
│   ├── __init__.py
│   ├── main.py              # FastAPI app
│   ├── config.py            # Settings
│   ├── data_manager.py      # CSV loading & management
│   ├── latency_controller.py # Delay simulation
│   ├── streaming_engine.py  # WebSocket/REST streaming
│   └── models.py            # Pydantic models
├── data/                    # Your CSV files go here
│   ├── EURUSD/
│   ├── GBPUSD/
│   └── USDJPY/
├── tests/
│   ├── test_api.py
│   ├── test_data.py
│   └── test_latency.py
├── requirements.txt
├── run.py                   # Startup script
├── README.md
└── config.yaml              # Configuration file
```

### Step-by-Step Implementation

#### Step 1: Configuration
**File: `app/config.py`**
```python
from pydantic import BaseSettings

class Settings(BaseSettings):
    # API settings
    host: str = "0.0.0.0"
    port: int = 8000
    debug: bool = False
    
    # Data settings
    data_directory: str = "data"
    supported_pairs: list = ["EURUSD", "GBPUSD", "USDJPY", "AUDUSD", "USDCAD"]
    
    # Latency settings
    default_base_latency_ms: int = 100
    default_jitter_ms: int = 20
    default_speed_multiplier: float = 1.0
    
    # Performance settings
    max_connections: int = 100
    preload_data: bool = True
    
    class Config:
        env_file = ".env"

settings = Settings()
```

#### Step 2: Data Manager
**File: `app/data_manager.py`**
```python
import pandas as pd
import numpy as np
from pathlib import Path
from typing import Dict, List, Optional
import logging

logger = logging.getLogger(__name__)

class ForexDataManager:
    def __init__(self, data_dir: str = "data"):
        self.data_dir = Path(data_dir)
        self.data: Dict[str, pd.DataFrame] = {}
        self.pairs: List[str] = []
        
    def load_all_pairs(self):
        """Load all available forex pairs."""
        for pair_dir in self.data_dir.iterdir():
            if pair_dir.is_dir():
                pair = pair_dir.name.upper()
                self.pairs.append(pair)
                self._load_pair_data(pair, pair_dir)
                logger.info(f"Loaded {pair} data")
        
        logger.info(f"Total pairs loaded: {len(self.pairs)}")
    
    def _load_pair_data(self, pair: str, pair_dir: Path):
        """Load all CSV files for a pair."""
        dfs = []
        for csv_file in sorted(pair_dir.glob("*.csv")):
            year = csv_file.stem
            df = pd.read_csv(csv_file, header=None, 
                           names=['timestamp', 'open', 'high', 'low', 'close', 'volume'])
            df['pair'] = pair
            df['timestamp'] = pd.to_datetime(df['timestamp'], unit='ms', utc=True)
            dfs.append(df)
            logger.debug(f"Loaded {pair} {year}: {len(df)} bars")
        
        if dfs:
            self.data[pair] = pd.concat(dfs, ignore_index=True)
            self.data[pair] = self.data[pair].sort_values('timestamp').reset_index(drop=True)
            logger.info(f"{pair}: {len(self.data[pair])} total bars")
    
    def get_pair_data(self, pair: str) -> Optional[pd.DataFrame]:
        """Get DataFrame for a pair."""
        return self.data.get(pair.upper())
    
    def get_latest_bar(self, pair: str) -> Optional[Dict]:
        """Get most recent bar for a pair."""
        df = self.get_pair_data(pair)
        if df is not None and not df.empty:
            latest = df.iloc[-1]
            return {
                "timestamp": int(latest['timestamp'].timestamp() * 1000),
                "open": float(latest['open']),
                "high": float(latest['high']),
                "low": float(latest['low']),
                "close": float(latest['close']),
                "volume": int(latest['volume']),
                "pair": pair
            }
        return None
    
    def get_historical_bars(self, pair: str, start_date: str = None, 
                           end_date: str = None, limit: int = None) -> List[Dict]:
        """Get historical bars with optional filtering."""
        df = self.get_pair_data(pair)
        if df is None:
            return []
        
        # Apply date filters
        if start_date:
            df = df[df['timestamp'] >= pd.to_datetime(start_date, utc=True)]
        if end_date:
            df = df[df['timestamp'] <= pd.to_datetime(end_date, utc=True)]
        
        # Apply limit
        if limit:
            df = df.tail(limit)
        
        # Convert to dict format
        bars = []
        for _, row in df.iterrows():
            bars.append({
                "timestamp": int(row['timestamp'].timestamp() * 1000),
                "open": float(row['open']),
                "high": float(row['high']),
                "low": float(row['low']),
                "close": float(row['close']),
                "volume": int(row['volume'])
            })
        
        return bars
    
    def get_available_pairs(self) -> List[str]:
        """Get list of loaded pairs."""
        return self.pairs.copy()
    
    def get_pair_info(self, pair: str) -> Optional[Dict]:
        """Get metadata for a pair."""
        df = self.get_pair_data(pair)
        if df is None:
            return None
        
        return {
            "pair": pair,
            "total_bars": len(df),
            "date_range": {
                "start": df['timestamp'].min().isoformat(),
                "end": df['timestamp'].max().isoformat()
            },
            "years": sorted(df['timestamp'].dt.year.unique().tolist())
        }
```

#### Step 3: Latency Controller
**File: `app/latency_controller.py`**
```python
import asyncio
import random
import time
from typing import Optional
import logging

logger = logging.getLogger(__name__)

class LatencyController:
    def __init__(self, base_latency_ms: int = 100, jitter_ms: int = 20, speed_multiplier: float = 1.0):
        self.base_latency_ms = base_latency_ms
        self.jitter_ms = jitter_ms
        self.speed_multiplier = speed_multiplier
        
    def set_latency(self, base_latency_ms: int, jitter_ms: int, speed_multiplier: float):
        """Update latency settings."""
        self.base_latency_ms = base_latency_ms
        self.jitter_ms = jitter_ms
        self.speed_multiplier = speed_multiplier
        logger.info(f"Latency updated: {self.get_settings()}")
    
    def get_settings(self) -> Dict:
        """Get current latency settings."""
        min_delay = (self.base_latency_ms - self.jitter_ms) / self.speed_multiplier
        max_delay = (self.base_latency_ms + self.jitter_ms) / self.speed_multiplier
        
        return {
            "base_latency_ms": self.base_latency_ms,
            "jitter_ms": self.jitter_ms,
            "speed_multiplier": self.speed_multiplier,
            "effective_latency_range": ".1f"
        }
    
    async def apply_latency(self):
        """Apply configured latency delay."""
        if self.base_latency_ms == 0 and self.jitter_ms == 0:
            return  # No delay
        
        # Calculate effective delay
        base_delay = self.base_latency_ms / 1000.0  # Convert to seconds
        jitter = self.jitter_ms / 1000.0
        
        # Add random jitter
        if jitter > 0:
            delay = base_delay + random.uniform(-jitter, jitter)
        else:
            delay = base_delay
        
        # Apply speed multiplier
        delay = delay / self.speed_multiplier
        
        # Ensure non-negative delay
        delay = max(0, delay)
        
        if delay > 0:
            await asyncio.sleep(delay)
    
    def get_effective_delay_range(self) -> tuple:
        """Get min/max possible delay in seconds."""
        min_delay = max(0, (self.base_latency_ms - self.jitter_ms) / 1000.0 / self.speed_multiplier)
        max_delay = max(0, (self.base_latency_ms + self.jitter_ms) / 1000.0 / self.speed_multiplier)
        return min_delay, max_delay
```

#### Step 4: Streaming Engine
**File: `app/streaming_engine.py`**
```python
import asyncio
import json
import logging
from typing import Dict, List, Set
from fastapi import WebSocket
from app.data_manager import ForexDataManager
from app.latency_controller import LatencyController

logger = logging.getLogger(__name__)

class StreamingEngine:
    def __init__(self, data_manager: ForexDataManager, latency_controller: LatencyController):
        self.data_manager = data_manager
        self.latency = latency_controller
        self.active_connections: Dict[str, Set[WebSocket]] = {}
        self.streaming_tasks: Dict[str, asyncio.Task] = {}
        
    async def subscribe_pair(self, websocket: WebSocket, pair: str):
        """Subscribe to real-time streaming for a pair."""
        pair = pair.upper()
        
        if pair not in self.active_connections:
            self.active_connections[pair] = set()
            # Start streaming task for this pair
            self.streaming_tasks[pair] = asyncio.create_task(self._stream_pair(pair))
        
        self.active_connections[pair].add(websocket)
        logger.info(f"Subscribed {websocket.client.host} to {pair}")
        
        try:
            # Send initial data
            latest = self.data_manager.get_latest_bar(pair)
            if latest:
                await websocket.send_json({
                    "type": "initial",
                    "data": latest
                })
            
            # Keep connection alive and handle messages
            while True:
                data = await websocket.receive_text()
                message = json.loads(data)
                
                if message.get("action") == "unsubscribe":
                    break
                    
        except Exception as e:
            logger.error(f"WebSocket error for {pair}: {e}")
        finally:
            self.active_connections[pair].discard(websocket)
            if not self.active_connections[pair]:
                # No more subscribers, stop streaming
                if pair in self.streaming_tasks:
                    self.streaming_tasks[pair].cancel()
                    del self.streaming_tasks[pair]
                del self.active_connections[pair]
    
    async def _stream_pair(self, pair: str):
        """Stream data for a pair to all subscribers."""
        df = self.data_manager.get_pair_data(pair)
        if df is None:
            logger.error(f"No data available for {pair}")
            return
        
        # Start from a recent point (last 1000 bars)
        start_idx = max(0, len(df) - 1000)
        
        try:
            for idx in range(start_idx, len(df)):
                if pair not in self.active_connections:
                    break
                
                row = df.iloc[idx]
                bar_data = {
                    "timestamp": int(row['timestamp'].timestamp() * 1000),
                    "open": float(row['open']),
                    "high": float(row['high']),
                    "low": float(row['low']),
                    "close": float(row['close']),
                    "volume": int(row['volume']),
                    "pair": pair
                }
                
                # Send to all subscribers
                dead_connections = set()
                for websocket in self.active_connections[pair]:
                    try:
                        await websocket.send_json({
                            "type": "bar",
                            "data": bar_data
                        })
                    except Exception:
                        dead_connections.add(websocket)
                
                # Clean up dead connections
                self.active_connections[pair] -= dead_connections
                
                # Apply latency
                await self.latency.apply_latency()
                
        except asyncio.CancelledError:
            logger.info(f"Streaming cancelled for {pair}")
        except Exception as e:
            logger.error(f"Streaming error for {pair}: {e}")
    
    async def get_historical_data(self, pair: str, start_date: str = None, 
                                end_date: str = None, limit: int = None) -> Dict:
        """Get historical data for REST API."""
        bars = self.data_manager.get_historical_bars(pair, start_date, end_date, limit)
        
        return {
            "pair": pair,
            "bars": bars,
            "count": len(bars),
            "start_date": start_date,
            "end_date": end_date
        }
```

#### Step 5: FastAPI Application
**File: `app/main.py`**
```python
from fastapi import FastAPI, HTTPException, WebSocket, WebSocketDisconnect
from fastapi.middleware.cors import CORSMiddleware
import logging
from app.config import settings
from app.data_manager import ForexDataManager
from app.latency_controller import LatencyController
from app.streaming_engine import StreamingEngine
from app.models import LatencySettings

# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Initialize components
data_manager = ForexDataManager(settings.data_directory)
latency_controller = LatencyController(
    base_latency_ms=settings.default_base_latency_ms,
    jitter_ms=settings.default_jitter_ms,
    speed_multiplier=settings.default_speed_multiplier
)
streaming_engine = StreamingEngine(data_manager, latency_controller)

# Load data on startup
data_manager.load_all_pairs()

# Create FastAPI app
app = FastAPI(
    title="Forex Market Data API",
    description="Real-time streaming API for forex market data",
    version="1.0.0"
)

# Add CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.get("/health")
async def health_check():
    """API health check."""
    return {
        "status": "healthy",
        "pairs": data_manager.get_available_pairs(),
        "latency_ms": latency_controller.base_latency_ms,
        "speed": latency_controller.speed_multiplier
    }

@app.get("/pairs")
async def get_pairs():
    """List available pairs."""
    pairs = data_manager.get_available_pairs()
    return {
        "pairs": pairs,
        "count": len(pairs)
    }

@app.get("/pairs/{pair}")
async def get_pair_info(pair: str):
    """Get information about a specific pair."""
    info = data_manager.get_pair_info(pair.upper())
    if not info:
        raise HTTPException(status_code=404, detail=f"Pair {pair} not found")
    return info

@app.get("/data/{pair}/latest")
async def get_latest_bar(pair: str):
    """Get latest bar for a pair."""
    bar = data_manager.get_latest_bar(pair.upper())
    if not bar:
        raise HTTPException(status_code=404, detail=f"No data for pair {pair}")
    return bar

@app.get("/data/{pair}/historical")
async def get_historical_data(pair: str, start: str = None, end: str = None, limit: int = None):
    """Get historical bars for a pair."""
    try:
        data = await streaming_engine.get_historical_data(pair.upper(), start, end, limit)
        return data
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/latency/set")
async def set_latency(settings: LatencySettings):
    """Set latency parameters."""
    latency_controller.set_latency(
        base_latency_ms=settings.base_latency_ms,
        jitter_ms=settings.jitter_ms,
        speed_multiplier=settings.speed_multiplier
    )
    return {
        "status": "updated",
        **latency_controller.get_settings()
    }

@app.get("/latency/get")
async def get_latency():
    """Get current latency settings."""
    return latency_controller.get_settings()

@app.websocket("/ws/market/{pair}")
async def websocket_pair(websocket: WebSocket, pair: str):
    """WebSocket endpoint for real-time pair streaming."""
    await websocket.accept()
    try:
        await streaming_engine.subscribe_pair(websocket, pair)
    except WebSocketDisconnect:
        logger.info(f"WebSocket disconnected for {pair}")
    except Exception as e:
        logger.error(f"WebSocket error for {pair}: {e}")

@app.websocket("/ws/market/all")
async def websocket_all(websocket: WebSocket):
    """WebSocket endpoint for streaming all pairs."""
    await websocket.accept()
    logger.info("Client connected to all pairs stream")
    
    try:
        # Send initial data for all pairs
        initial_data = {}
        for pair in data_manager.get_available_pairs():
            latest = data_manager.get_latest_bar(pair)
            if latest:
                initial_data[pair] = latest
        
        await websocket.send_json({
            "type": "initial",
            "data": initial_data
        })
        
        # Keep connection alive (simplified - in production, implement proper streaming)
        while True:
            await websocket.receive_text()
            
    except WebSocketDisconnect:
        logger.info("WebSocket disconnected from all pairs stream")
    except Exception as e:
        logger.error(f"WebSocket error for all pairs: {e}")
```

#### Step 6: Pydantic Models
**File: `app/models.py`**
```python
from pydantic import BaseModel, Field
from typing import Optional

class LatencySettings(BaseModel):
    base_latency_ms: int = Field(..., ge=0, le=5000, description="Base latency in milliseconds")
    jitter_ms: int = Field(..., ge=0, le=1000, description="Jitter variation in milliseconds")
    speed_multiplier: float = Field(..., ge=0.1, le=10.0, description="Speed multiplier (1.0 = real-time)")

class BarData(BaseModel):
    timestamp: int
    open: float
    high: float
    low: float
    close: float
    volume: int
    pair: str

class HistoricalRequest(BaseModel):
    start: Optional[str] = None
    end: Optional[str] = None
    limit: Optional[int] = Field(None, ge=1, le=10000)
```

#### Step 7: Startup Script
**File: `run.py`**
```python
import uvicorn
from app.config import settings

if __name__ == "__main__":
    uvicorn.run(
        "app.main:app",
        host=settings.host,
        port=settings.port,
        reload=settings.debug,
        log_level="info"
    )
```

#### Step 8: Requirements
**File: `requirements.txt`**
```
fastapi==0.104.1
uvicorn[standard]==0.24.0
pandas==2.1.3
numpy==1.26.2
websockets==12.0
pydantic==2.5.0
python-multipart==0.0.6
aiofiles==23.2.1
```

#### Step 9: Configuration
**File: `config.yaml`**
```yaml
# API Configuration
host: "0.0.0.0"
port: 8000
debug: false

# Data Configuration
data_directory: "data"
supported_pairs:
  - EURUSD
  - GBPUSD
  - USDJPY
  - AUDUSD
  - USDCAD

# Latency Configuration
default_base_latency_ms: 100
default_jitter_ms: 20
default_speed_multiplier: 1.0

# Performance Configuration
max_connections: 100
preload_data: true
```

---

## Usage Examples

### Starting the API
```bash
# Install dependencies
pip install -r requirements.txt

# Run the API
python run.py
```

### Testing the API
```bash
# Health check
curl http://localhost:8000/health

# List pairs
curl http://localhost:8000/pairs

# Get latest EURUSD bar
curl http://localhost:8000/data/EURUSD/latest

# Set high latency (slow connection)
curl -X POST http://localhost:8000/latency/set \
  -H "Content-Type: application/json" \
  -d '{"base_latency_ms": 500, "jitter_ms": 100, "speed_multiplier": 1.0}'

# Get historical data
curl "http://localhost:8000/data/EURUSD/historical?start=2020-01-01&end=2020-01-02&limit=10"
```

### WebSocket Testing
```python
import websockets
import asyncio
import json

async def test_websocket():
    uri = "ws://localhost:8000/ws/market/EURUSD"
    async with websockets.connect(uri) as websocket:
        # Send subscription
        await websocket.send(json.dumps({
            "action": "subscribe",
            "pair": "EURUSD"
        }))
        
        # Receive data
        while True:
            message = await websocket.recv()
            data = json.loads(message)
            print(f"Received: {data}")

asyncio.run(test_websocket())
```

### Integration with Trading Simulator
```python
import websockets
import json
import asyncio

class MarketDataClient:
    def __init__(self, pair="EURUSD"):
        self.pair = pair
        self.uri = f"ws://localhost:8000/ws/market/{pair}"
        self.on_bar_callback = None
    
    async def connect(self):
        async with websockets.connect(self.uri) as websocket:
            await websocket.send(json.dumps({
                "action": "subscribe",
                "pair": self.pair
            }))
            
            while True:
                message = await websocket.recv()
                data = json.loads(message)
                
                if data["type"] == "bar" and self.on_bar_callback:
                    await self.on_bar_callback(data["data"])

# Usage in your simulator
client = MarketDataClient("EURUSD")
client.on_bar_callback = lambda bar: print(f"New bar: {bar}")
asyncio.run(client.connect())
```

---

## Testing

### Unit Tests
**File: `tests/test_api.py`**
```python
import pytest
from fastapi.testclient import TestClient
from app.main import app

client = TestClient(app)

def test_health():
    response = client.get("/health")
    assert response.status_code == 200
    assert response.json()["status"] == "healthy"

def test_pairs():
    response = client.get("/pairs")
    assert response.status_code == 200
    assert "pairs" in response.json()

def test_latency_set():
    response = client.post("/latency/set", json={
        "base_latency_ms": 200,
        "jitter_ms": 50,
        "speed_multiplier": 2.0
    })
    assert response.status_code == 200
    assert response.json()["base_latency_ms"] == 200
```

### Integration Tests
**File: `tests/test_integration.py`**
```python
import pytest
import asyncio
import websockets
import json
from app.main import app
from fastapi.testclient import TestClient

@pytest.mark.asyncio
async def test_websocket_streaming():
    # Test WebSocket connection and data streaming
    uri = "ws://localhost:8000/ws/market/EURUSD"
    
    try:
        async with websockets.connect(uri) as websocket:
            await websocket.send(json.dumps({
                "action": "subscribe",
                "pair": "EURUSD"
            }))
            
            # Should receive initial data
            message = await asyncio.wait_for(websocket.recv(), timeout=5.0)
            data = json.loads(message)
            assert data["type"] == "initial"
            
    except Exception as e:
        pytest.fail(f"WebSocket test failed: {e}")
```

### Performance Tests
**File: `tests/test_performance.py`**
```python
import time
import requests

def test_api_response_time():
    start = time.time()
    response = requests.get("http://localhost:8000/health")
    end = time.time()
    
    assert response.status_code == 200
    assert end - start < 0.1  # Should respond in < 100ms
```

---

## Configuration & Deployment

### Environment Variables
```bash
# .env file
HOST=0.0.0.0
PORT=8000
DEBUG=false
DATA_DIRECTORY=data
DEFAULT_BASE_LATENCY_MS=100
DEFAULT_JITTER_MS=20
DEFAULT_SPEED_MULTIPLIER=1.0
```

### Docker Deployment
**Dockerfile**:
```dockerfile
FROM python:3.11-slim

WORKDIR /app
COPY requirements.txt .
RUN pip install -r requirements.txt

COPY . .
EXPOSE 8000

CMD ["python", "run.py"]
```

**docker-compose.yml**:
```yaml
version: '3.8'
services:
  forex-api:
    build: .
    ports:
      - "8000:8000"
    volumes:
      - ./data:/app/data:ro
    environment:
      - DEBUG=false
```

### Production Deployment
```bash
# Using Gunicorn + Uvicorn workers
pip install gunicorn
gunicorn app.main:app -w 4 -k uvicorn.workers.UvicornWorker --bind 0.0.0.0:8000
```

---

## Troubleshooting

### Common Issues

#### 1. "No data available for pair"
- Check CSV files exist in `data/{PAIR}/` directory
- Verify CSV format (no headers, correct column order)
- Ensure timestamps are in milliseconds

#### 2. "WebSocket connection failed"
- Check API is running on correct port
- Verify firewall allows connections
- Check browser console for CORS errors

#### 3. "High memory usage"
- Reduce preload data or use streaming CSV reading
- Limit historical data requests with smaller limits
- Monitor with `memory_profiler`

#### 4. "Latency not working"
- Check latency settings with `/latency/get`
- Verify WebSocket client handles delays properly
- Test with simple curl commands first

### Debugging Commands
```bash
# Check API logs
tail -f logs/api.log

# Test data loading
python -c "from app.data_manager import ForexDataManager; dm = ForexDataManager(); dm.load_all_pairs(); print(dm.get_available_pairs())"

# Test latency
curl http://localhost:8000/latency/set -X POST -H "Content-Type: application/json" -d '{"base_latency_ms": 1000, "jitter_ms": 0, "speed_multiplier": 1.0}'
```

---

## Performance Considerations

### Memory Usage
- **1 year of 1-min data per pair**: ~50MB
- **5 pairs, 3 years**: ~750MB
- **Optimization**: Use `dask` for out-of-core processing if needed

### Throughput
- **REST API**: 1000+ requests/second
- **WebSocket**: 100+ concurrent connections
- **Data streaming**: 1000+ bars/second per pair

### Scaling
- **Horizontal**: Run multiple API instances behind load balancer
- **Vertical**: Increase server resources for more pairs/years
- **Caching**: Redis for frequently accessed data

---

## Security Considerations

### For Local Development
- CORS enabled for all origins (development only)
- No authentication required
- Data directory should be read-only

### For Production
- Add API key authentication
- Restrict CORS origins
- Use HTTPS/WSS
- Rate limiting
- Input validation
- Secure data directory access

---

## Future Enhancements

- [ ] Real-time data integration (bridge to live feeds)
- [ ] Advanced latency patterns (network congestion simulation)
- [ ] Data compression for WebSocket streams
- [ ] Historical data caching (Redis)
- [ ] Multi-timeframe support (1m, 5m, 1h, 1d)
- [ ] Order book depth simulation
- [ ] Market microstructure events (news, gaps)
- [ ] Performance metrics dashboard
- [ ] API documentation with Swagger UI

---

## Summary

This API provides:
1. **Local market data streaming** from your CSV files
2. **Configurable latency** for testing different network conditions
3. **Real-time WebSocket streaming** for live-like simulation
4. **REST endpoints** for historical data access
5. **Standalone operation** independent of your trading simulator

**Key Benefits**:
- **Cost-effective**: No expensive data subscriptions
- **Controllable**: Adjust latency and speed for testing
- **Realistic**: Simulates actual market data feeds
- **Fast**: Built with async Python for high performance

**Ready to build?** This README contains everything Claude needs to implement the complete API. Just provide your CSV data structure and Claude will build it exactly as specified.