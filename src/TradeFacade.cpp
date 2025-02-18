#include "TradeFacade.h"
#include <iostream>
#include <unordered_map>
#include "UtilsController.h" 
#include <future>

using json = nlohmann::json;  
// Constructor initializes the connection, trading object, and thread pool
TradeFacade::TradeFacade(std::string &BASE_URL) : tradeController(BASE_URL) {
}


// Place a single order synchronously
json TradeFacade::placeOrder(const std::string &token, const std::string &instrument, const std::string &type, double amount, double price, const std::string &label)
{
    return tradeController.placeOrder(token, instrument, type, amount, price, label);
}

// Modify an existing order
json TradeFacade::modifyOrder(const std::string &order_id, const std::string &token, const std::optional<double> &amount, const std::optional<double> &contracts, const std::optional<double> &price, const std::optional<std::string> &advanced, const std::optional<bool> &post_only, const std::optional<bool> &reduce_only)
{
    return tradeController.modifyOrder(order_id, token, amount, contracts, price, advanced, post_only, reduce_only);
}

// Place a single sell order synchronously
json TradeFacade::sellOrder(const std::string &token, const std::string &instrument, const std::optional<double> &amount, const std::optional<double> &contracts, const std::optional<double> &price, const std::optional<std::string> &type, const std::optional<std::string> &trigger, const std::optional<double> &trigger_price)
{
    return tradeController.sellOrder(token, instrument, amount, contracts, price, type, trigger, trigger_price);
}
// Cancel a single order synchronously
json TradeFacade::cancelOrder(const std::string &orderid, const std::string &token)
{
    return tradeController.cancelOrder(orderid, token);
}

// Cancel all open orders
json TradeFacade::cancelAllOrder(const std::string &token)
{
    return tradeController.cancelAllOrder(token);
}

// Get all open orders
json TradeFacade::getOpenOrder(const std::string &token)
{
    return tradeController.getOpenOrder(token);
}
json TradeFacade::getOrderState(const std::string &orderid, const std::string &token)
{
    return tradeController.getOrderState(orderid, token);
}
// Get user trades by order
json TradeFacade::getOrderBook(const std::string& instrument_name) {
    return tradeController.getOrderBook(instrument_name);
}
// Get user trades by order
json TradeFacade::getPositions(const std::string &token)
{
    return tradeController.getPositions(token);
}
// Get user trades by order
json TradeFacade::getPosition(const std::string &token, const std::string &currency)
{
    return tradeController.getPosition(token, currency);
}
