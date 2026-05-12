#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <format>

#include <ibalgotrade/core/order.hpp>

namespace py = pybind11;
using namespace ibalgotrade;

PYBIND11_MODULE(_core, m) {
    m.doc() = "ib-algo-trade C++ core bindings";

    py::enum_<Side>(m, "Side")
        .value("Buy", Side::Buy)
        .value("Sell", Side::Sell);

    py::enum_<OrderType>(m, "OrderType")
        .value("Market", OrderType::Market)
        .value("Limit", OrderType::Limit);

    py::class_<Order>(m, "Order")
        .def(py::init<>())
        .def_readwrite("id", &Order::id)
        .def_readwrite("symbol", &Order::symbol)
        .def_readwrite("side", &Order::side)
        .def_readwrite("type", &Order::type)
        .def_readwrite("price", &Order::price)
        .def_readwrite("quantity", &Order::quantity)
        .def_readwrite("timestamp", &Order::timestamp);

    py::class_<Fill>(m, "Fill")
        .def(py::init<>())
        .def_readonly("order_id", &Fill::order_id)
        .def_readonly("counter_order_id", &Fill::counter_order_id)
        .def_readonly("price", &Fill::price)
        .def_readonly("quantity", &Fill::quantity)
        .def("__repr__", [](const Fill& f) {
            return std::format("Fill(oid={}, cid={}, px={}, qty={})",
                               f.order_id, f.counter_order_id, f.price, f.quantity);
        });

    py::class_<PriceLevel>(m, "PriceLevel")
        .def(py::init<>())
        .def_readonly("price", &PriceLevel::price)
        .def_readonly("total_quantity", &PriceLevel::total_quantity)
        .def_readonly("order_count", &PriceLevel::order_count)
        .def("__repr__", [](const PriceLevel& pl) {
            return std::format("PriceLevel(px={}, qty={}, n={})",
                               pl.price, pl.total_quantity, pl.order_count);
        });

    py::class_<LimitOrderBook>(m, "LimitOrderBook")
        .def(py::init<std::string>())
        .def("add", &LimitOrderBook::add)
        .def("cancel", &LimitOrderBook::cancel)
        .def("best_bid", &LimitOrderBook::best_bid)
        .def("best_ask", &LimitOrderBook::best_ask)
        .def("bid_size", &LimitOrderBook::bid_size)
        .def("ask_size", &LimitOrderBook::ask_size)
        .def("bid_levels", &LimitOrderBook::bid_levels)
        .def("ask_levels", &LimitOrderBook::ask_levels)
        .def_property_readonly("symbol", &LimitOrderBook::symbol)
        .def_property_readonly("order_count", &LimitOrderBook::order_count)
        .def("__repr__", [](const LimitOrderBook& ob) {
            return std::format("LimitOrderBook({}, bids={}, asks={})",
                               ob.symbol(), ob.bid_levels().size(), ob.ask_levels().size());
        });
}
