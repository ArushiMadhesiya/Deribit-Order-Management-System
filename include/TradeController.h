#ifndef TRADE_CONTROLLER_H
#define TRADE_CONTROLLER_H

#include "ConnectionController.h"
#include "nlohmann/json.hpp"
#include <unordered_map>
#include <optional>

using json = nlohmann::json; 
class TradeController {
public:
TradeController(std::string &BASE_URL);

    json placeOrder(
        const std::string& token,
        const std::string& instrument,
        const std::string& type,
        double amount = 0.0,
        double price = 0.0,
        const std::string& label = "");

    json modifyOrder(
        const std::string& order_id,
        const std::string& token,
        const std::optional<double>& amount = std::nullopt,
        const std::optional<double>& contracts = std::nullopt,
        const std::optional<double>& price = std::nullopt,
        const std::optional<std::string>& advanced = std::nullopt,
        const std::optional<bool>& post_only = std::nullopt,
        const std::optional<bool>& reduce_only = std::nullopt);

    json sellOrder(
        const std::string& token,
        const std::string& instrument,
        const std::optional<double>& amount = std::nullopt,
        const std::optional<double>& contracts = std::nullopt,
        const std::optional<double>& price = std::nullopt,
        const std::optional<std::string>& type = std::nullopt,
        const std::optional<std::string>& trigger = std::nullopt,
        const std::optional<double>& trigger_price = std::nullopt);

    json cancelOrder(const std::string& orderid, const std::string& token);
    json cancelAllOrder(const std::string& token);
    json getOpenOrder(const std::string& token);
    json getOrderState(const std::string& orderid, const std::string& token);
    json getOrderBook(const std::string& instrument_name);
    json getPositions(const std::string& token);
    json getPosition(const std::string& token,const std::string& instrument_name);
private:
    ConnectionController connection;
};
#endif


