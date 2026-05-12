"""
Pre-production verification helpers.

All strategy verification should happen here against the C++ core.
Once a strategy is verified, port it to C++ for production.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Sequence

from ibalgotrade._core import (
    Fill,
    LimitOrderBook,
    Order,
    OrderType,
    Side,
)


@dataclass
class ExpectedState:
    """Snapshot of expected order book state after a sequence of orders."""

    best_bid: float | None = None
    best_ask: float | None = None
    bid_size: int = 0
    ask_size: int = 0
    order_count: int = 0
    fills: list[Fill] = field(default_factory=list)


def replay_orders(
    book: LimitOrderBook,
    orders: Sequence[Order],
) -> list[Fill]:
    """Apply a sequence of orders and return all fills."""
    all_fills: list[Fill] = []
    for order in orders:
        all_fills.extend(book.add(order))
    return all_fills


def check_state(book: LimitOrderBook, expected: ExpectedState) -> bool:
    """Verify the C++ order book matches an expected state snapshot."""
    return (
        book.best_bid() == expected.best_bid
        and book.best_ask() == expected.best_ask
        and book.bid_size() == expected.bid_size
        and book.ask_size() == expected.ask_size
        and book.order_count == expected.order_count
    )
