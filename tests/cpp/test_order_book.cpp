#include <ibalgotrade/core/order.hpp>

#include <catch2/catch_test_macros.hpp>

#include <tuple>

using namespace ibalgotrade;
using namespace std::chrono_literals;

TEST_CASE("LimitOrderBook starts empty", "[order_book]") {
    LimitOrderBook book("AAPL");
    CHECK(book.symbol() == "AAPL");
    CHECK(!book.best_bid().has_value());
    CHECK(!book.best_ask().has_value());
    CHECK(book.bid_size() == 0);
    CHECK(book.ask_size() == 0);
    CHECK(book.order_count() == 0);
}

TEST_CASE("Limit order rests when no match", "[order_book]") {
    LimitOrderBook book("AAPL");
    Order buy{.id = 1, .side = Side::Buy, .type = OrderType::Limit, .price = 100.0, .quantity = 10};
    auto fills = book.add(buy);
    CHECK(fills.empty());
    CHECK(book.best_bid() == 100.0);
    CHECK(book.bid_size() == 10);
    CHECK(book.order_count() == 1);
}

TEST_CASE("Limit orders match at same price", "[order_book]") {
    LimitOrderBook book("AAPL");
    Order buy{.id = 1, .side = Side::Buy, .type = OrderType::Limit, .price = 100.0, .quantity = 10};
    Order sell{.id = 2, .side = Side::Sell, .type = OrderType::Limit, .price = 100.0, .quantity = 7};

    std::ignore = book.add(buy);
    auto fills = book.add(sell);

    REQUIRE(fills.size() == 1);
    CHECK(fills[0].order_id == 2);
    CHECK(fills[0].counter_order_id == 1);
    CHECK(fills[0].price == 100.0);
    CHECK(fills[0].quantity == 7);
    CHECK(book.bid_size() == 3);
    CHECK(book.ask_size() == 0);
}

TEST_CASE("Market order sweeps the book", "[order_book]") {
    LimitOrderBook book("AAPL");
    std::ignore = book.add(Order{.id = 1, .side = Side::Sell, .type = OrderType::Limit, .price = 100.0, .quantity = 5});
    std::ignore = book.add(Order{.id = 2, .side = Side::Sell, .type = OrderType::Limit, .price = 101.0, .quantity = 10});

    auto fills = book.add(Order{.id = 3, .side = Side::Buy, .type = OrderType::Market, .price = 0, .quantity = 12});

    REQUIRE(fills.size() == 2);
    CHECK(fills[0].price == 100.0);  // best price first
    CHECK(fills[0].quantity == 5);
    CHECK(fills[1].price == 101.0);
    CHECK(fills[1].quantity == 7);
    CHECK(book.ask_size() == 3);  // 10 - 3 remaining at 101
    CHECK(book.order_count() == 1);
}

TEST_CASE("Cancel removes resting order", "[order_book]") {
    LimitOrderBook book("AAPL");
    std::ignore = book.add(Order{.id = 1, .side = Side::Buy, .type = OrderType::Limit, .price = 99.0, .quantity = 10});
    std::ignore = book.add(Order{.id = 2, .side = Side::Buy, .type = OrderType::Limit, .price = 100.0, .quantity = 5});

    CHECK(book.order_count() == 2);
    book.cancel(1);
    CHECK(book.order_count() == 1);
    CHECK(book.best_bid() == 100.0);
}

TEST_CASE("Price-time priority is maintained", "[order_book]") {
    LimitOrderBook book("AAPL");
    // Two sells at the same price — first one should trade first
    std::ignore = book.add(Order{.id = 1, .side = Side::Sell, .type = OrderType::Limit, .price = 100.0, .quantity = 5});
    std::ignore = book.add(Order{.id = 2, .side = Side::Sell, .type = OrderType::Limit, .price = 100.0, .quantity = 10});

    auto fills = book.add(Order{.id = 3, .side = Side::Buy, .type = OrderType::Limit, .price = 100.0, .quantity = 8});

    REQUIRE(fills.size() == 2);
    CHECK(fills[0].counter_order_id == 1);  // earlier order fills first
    CHECK(fills[0].quantity == 5);
    CHECK(fills[1].counter_order_id == 2);
    CHECK(fills[1].quantity == 3);
}
