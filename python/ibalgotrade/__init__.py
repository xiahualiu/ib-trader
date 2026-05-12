"""
ib-algo-trade — C++ algorithmic trading core with Python verification layer.

C++ is the source of truth for all production logic.
Python wraps the C++ core for prototyping, verification, and analysis.
"""

from ibalgotrade._core import (
    Side,
    OrderType,
    Order,
    Fill,
    PriceLevel,
    LimitOrderBook,
)

__all__ = [
    "Side",
    "OrderType",
    "Order",
    "Fill",
    "PriceLevel",
    "LimitOrderBook",
]
