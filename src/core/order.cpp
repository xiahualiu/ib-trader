#include <ibalgotrade/core/order.hpp>

#include <algorithm>
#include <stdexcept>

namespace ibalgotrade {

LimitOrderBook::LimitOrderBook(std::string symbol) : symbol_(std::move(symbol)) {}

std::vector<Fill> LimitOrderBook::add(Order order) {
    std::vector<Fill> fills;
    auto remaining = order.quantity;
    const bool is_buy = order.side == Side::Buy;

    auto can_trade = [&](const Level& level) -> bool {
        if (order.type == OrderType::Market) return true;
        if (is_buy) return level.price <= order.price;
        return level.price >= order.price;
    };

    // Cross against the opposite side.  Storage is arranged so back() is the
    // best price for the aggressor: asks_ descending (back = lowest),
    // bids_ ascending (back = highest).
    auto& opposite = is_buy ? asks_ : bids_;

    while (remaining > 0 && !opposite.empty() && can_trade(opposite.back())) {
        auto& level = opposite.back();
        auto fill_qty = std::min(remaining, level.quantity);
        fills.push_back(Fill{
            .order_id = order.id,
            .counter_order_id = level.order_id,
            .price = level.price,
            .quantity = fill_qty,
        });
        remaining -= fill_qty;
        level.quantity -= fill_qty;
        if (level.quantity == 0) {
            opposite.pop_back();
        }
    }

    // Resting limit order sits on own side book
    if (order.type == OrderType::Limit && remaining > 0) {
        auto& side_book = is_buy ? bids_ : asks_;
        Level new_level{order.price, remaining, order.id};

        if (is_buy) {
            // Bids: ascending price, back() = highest
            auto it = std::lower_bound(
                side_book.begin(), side_book.end(), new_level,
                [](const Level& a, const Level& b) { return a.price < b.price; });
            side_book.insert(it, new_level);
        } else {
            // Asks: descending price, back() = lowest
            auto it = std::lower_bound(
                side_book.begin(), side_book.end(), new_level,
                [](const Level& a, const Level& b) { return a.price > b.price; });
            side_book.insert(it, new_level);
        }
    }

    return fills;
}

void LimitOrderBook::cancel(std::uint64_t order_id) {
    auto pred = [order_id](const Level& l) { return l.order_id == order_id; };
    if (auto it = std::find_if(bids_.begin(), bids_.end(), pred); it != bids_.end()) {
        bids_.erase(it);
        return;
    }
    if (auto it = std::find_if(asks_.begin(), asks_.end(), pred); it != asks_.end()) {
        asks_.erase(it);
    }
}

std::optional<double> LimitOrderBook::best_bid() const {
    if (bids_.empty()) return std::nullopt;
    return bids_.back().price;
}
std::optional<double> LimitOrderBook::best_ask() const {
    if (asks_.empty()) return std::nullopt;
    return asks_.back().price;
}
std::int64_t LimitOrderBook::bid_size() const {
    std::int64_t s = 0;
    for (auto& l : bids_) s += l.quantity;
    return s;
}
std::int64_t LimitOrderBook::ask_size() const {
    std::int64_t s = 0;
    for (auto& l : asks_) s += l.quantity;
    return s;
}
std::vector<PriceLevel> LimitOrderBook::bid_levels() const {
    std::vector<PriceLevel> out;
    // Bids stored ascending; iterate forward for display
    for (auto& l : bids_) {
        if (!out.empty() && out.back().price == l.price) {
            out.back().total_quantity += l.quantity;
            out.back().order_count++;
        } else {
            out.push_back({l.price, l.quantity, 1});
        }
    }
    return out;
}
std::vector<PriceLevel> LimitOrderBook::ask_levels() const {
    std::vector<PriceLevel> out;
    // Asks stored descending; iterate forward for display
    for (auto& l : asks_) {
        if (!out.empty() && out.back().price == l.price) {
            out.back().total_quantity += l.quantity;
            out.back().order_count++;
        } else {
            out.push_back({l.price, l.quantity, 1});
        }
    }
    return out;
}
const std::string& LimitOrderBook::symbol() const { return symbol_; }
std::size_t LimitOrderBook::order_count() const {
    return bids_.size() + asks_.size();
}

}  // namespace ibalgotrade
