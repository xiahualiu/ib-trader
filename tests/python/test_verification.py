"""
Verification tests that run against the C++ core via Python bindings.

These prove the C++ implementation is correct. Python is the
verification harness — the C++ code is what ships to production.
"""

import pytest
from ibalgotrade import Fill, LimitOrderBook, Order, OrderType, Side
from ibalgotrade.verification import ExpectedState, check_state, replay_orders


class TestLimitOrderBook:
    def test_empty_book(self):
        book = LimitOrderBook("AAPL")
        assert book.symbol == "AAPL"
        assert book.best_bid() is None
        assert book.best_ask() is None
        assert book.bid_size() == 0
        assert book.ask_size() == 0
        assert book.order_count == 0

    def test_limit_order_rests(self):
        book = LimitOrderBook("AAPL")
        order = Order()
        order.id = 1
        order.side = Side.Buy
        order.type = OrderType.Limit
        order.price = 100.0
        order.quantity = 10

        fills = book.add(order)
        assert len(fills) == 0
        assert book.best_bid() == 100.0
        assert book.bid_size() == 10

    def test_match_same_price(self):
        book = LimitOrderBook("AAPL")

        buy = Order()
        buy.id = 1
        buy.side = Side.Buy
        buy.type = OrderType.Limit
        buy.price = 100.0
        buy.quantity = 10
        book.add(buy)

        sell = Order()
        sell.id = 2
        sell.side = Side.Sell
        sell.type = OrderType.Limit
        sell.price = 100.0
        sell.quantity = 7

        fills = book.add(sell)
        assert len(fills) == 1
        assert fills[0].price == 100.0
        assert fills[0].quantity == 7
        assert book.bid_size() == 3
        assert book.order_count == 1

    def test_market_order_sweeps(self):
        book = LimitOrderBook("AAPL")

        def make_order(oid, side, otype, price, qty):
            o = Order()
            o.id = oid
            o.side = side
            o.type = otype
            o.price = price
            o.quantity = qty
            return o

        book.add(make_order(1, Side.Sell, OrderType.Limit, 100.0, 5))
        book.add(make_order(2, Side.Sell, OrderType.Limit, 101.0, 10))

        fills = book.add(make_order(3, Side.Buy, OrderType.Market, 0, 12))
        assert len(fills) == 2
        assert fills[0].price == 100.0
        assert fills[0].quantity == 5
        assert fills[1].price == 101.0
        assert fills[1].quantity == 7
        assert book.ask_size() == 3

    def test_price_time_priority(self):
        book = LimitOrderBook("AAPL")

        def make_order(oid, side, otype, price, qty):
            o = Order()
            o.id = oid
            o.side = side
            o.type = otype
            o.price = price
            o.quantity = qty
            return o

        book.add(make_order(1, Side.Sell, OrderType.Limit, 100.0, 5))
        book.add(make_order(2, Side.Sell, OrderType.Limit, 100.0, 10))

        fills = book.add(make_order(3, Side.Buy, OrderType.Limit, 100.0, 8))
        assert fills[0].counter_order_id == 1
        assert fills[0].quantity == 5
        assert fills[1].counter_order_id == 2
        assert fills[1].quantity == 3

    def test_verification_helper(self):
        """End-to-end: run a sequence through C++ and verify state snapshot."""
        book = LimitOrderBook("MSFT")

        def o(oid, side, otype, price, qty):
            order = Order()
            order.id = oid
            order.side = side
            order.type = otype
            order.price = price
            order.quantity = qty
            return order

        fills = replay_orders(
            book,
            [
                o(1, Side.Buy, OrderType.Limit, 50.0, 100),
                o(2, Side.Buy, OrderType.Limit, 51.0, 200),
                o(3, Side.Sell, OrderType.Limit, 52.0, 150),
            ],
        )

        assert len(fills) == 0  # no cross
        assert check_state(
            book,
            ExpectedState(best_bid=51.0, best_ask=52.0, bid_size=300, ask_size=150, order_count=3),
        )
