#include "TradeFacade.h"
#include "WebSocketClient.h"
#include "UtilsController.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include <curl/curl.h>

#define CLIENT_ID "lpRS0VJm"
#define CLIENT_SECRET "k3eWUZkDrdw8V-qOcnNFk1F2iCH2L-SXRj3Yk9r2gEQ"

using json = nlohmann::json;
using namespace std;

string BASE_URL  = "https://test.deribit.com";


// Callback function to write the response to a string
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}
// Function to retrieve the authentication token
string getAuthToken() {
    const std::string url = std::string(BASE_URL) + "/api/v2/public/auth?client_id=" + CLIENT_ID +
                            "&client_secret=" + CLIENT_SECRET + "&grant_type=client_credentials";
    std::string response;

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return "";
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return "";
    }

    curl_easy_cleanup(curl);

    try {
        json doc = json::parse(response); // Parse with nlohmann::json

        if (doc.contains("result") && doc["result"].is_object() &&
            doc["result"].contains("access_token") && doc["result"]["access_token"].is_string()) {
            return doc["result"]["access_token"].get<std::string>(); // Get string value
        } else {
            std::cerr << "Failed to retrieve access token: " << response << std::endl;
            return "";
        }
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return "";
    } catch (const json::type_error& e) {
        std::cerr << "JSON type error: " << e.what() << std::endl;
        return "";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return "";
    }
}int main()
{
    std::cout << "Trading System Initializing...\n";

    // Initialize core components
    TradeFacade tradeFacade(BASE_URL);

    // Get authentication token
    std::string token = getAuthToken();
    if (token.empty())
    {
        std::cerr << "Failed to obtain authentication token. Exiting...\n";
        return 1;
    }
    std::cout << "Successfully authenticated!\n";

    int networkChoice = 0;
    do
    {
        showNetworkMenu();
        if (!(std::cin >> networkChoice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        switch (networkChoice)
        {
        case 1:
        { // WebSocket Session
            try
            {
                std::cout << "Starting WebSocket session...\n";
                WebSocketClient client;
                client.setMessageHandler([](const std::string &message)
                                         {  prettyPrintJson(message); });

                client.startWebSocketSession(token);
                std::cout << "WebSocket session started. Press Enter to stop...\n";

                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin.get();
                client.close();
            }
            catch (const std::exception &e)
            {
                std::cerr << "WebSocket Error: " << e.what() << std::endl;
            }
            break;
        }

        case 2:
        { //  Over HTTP Trade Menu
            int choice;
            do
            {
                showTradeMenu();
                if (!(std::cin >> choice))
                {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Invalid input. Please enter a number.\n";
                    continue;
                }

                json response;
                std::string instrument, orderId, currency, type, label;
                double amount, price;
                
                try
                {   
                    
                    switch (choice)
                    {
                    case 1:
                    { // Place Order
                        
                        std::cout << "Enter instrument name (e.g., ETH-PERPETUAL): ";
                        std::cin >> instrument;

                        std::cout << "Enter order type (limit/market/stop_limit): ";
                        std::cin >> type;

                        std::cout << "Enter amount: ";
                        if (!(std::cin >> amount))
                        {
                            throw std::runtime_error("Invalid amount input");
                        }

                        price = 0.0;
                        if (type == "limit" || type == "stop_limit")
                        {
                            std::cout << "Enter price: ";
                            if (!(std::cin >> price))
                            {
                                throw std::runtime_error("Invalid price input");
                            }
                        }

                        std::cout << "Enter label (optional, press Enter to skip): ";
                        std::cin.ignore();
                        std::getline(std::cin, label);

                        auto start_time = std::chrono::high_resolution_clock::now();
                        response = tradeFacade.placeOrder(token, instrument, type, amount, price, label);
                        auto end_time = std::chrono::high_resolution_clock::now();
                        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

                        std::cout << "Response: " << jsonToString(response) << std::endl;
                        std::cout << "Order placement latency: " << latency << " milliseconds" << std::endl;
                        break;
                    }

                    case 2:
                    { // Modify Order
                        std::cout << "Enter order ID: ";
                        std::cin >> orderId;

                        std::cout << "Enter new amount: ";
                        if (!(std::cin >> amount))
                        {
                            throw std::runtime_error("Invalid amount input");
                        }

                        std::cout << "Enter new price: ";
                        if (!(std::cin >> price))
                        {
                            throw std::runtime_error("Invalid price input");
                        }
                        auto start_time = std::chrono::high_resolution_clock::now();
                        response = tradeFacade.modifyOrder(orderId, token, amount, std::nullopt, price);
                        auto end_time = std::chrono::high_resolution_clock::now();
                        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
                        std::cout << "Response: " << jsonToString(response) << std::endl;
                        std::cout << "Order modification latency: " << latency << " milliseconds" << std::endl;
                        break;
                    }

                    case 3:
                    { // Sell Order
                        std::cout << "Enter instrument name: ";
                        std::cin >> instrument;

                        std::cout << "Enter amount: ";
                        if (!(std::cin >> amount))
                        {
                            throw std::runtime_error("Invalid amount input");
                        }

                        std::cout << "Enter order type (limit/market/stop_limit): ";
                        std::cin >> type;

                        price = 0.0;
                        if (type == "limit" || type == "stop_limit")
                        {
                            std::cout << "Enter price: ";
                            if (!(std::cin >> price))
                            {
                                throw std::runtime_error("Invalid price input");
                            }
                        }
                        auto start_time = std::chrono::high_resolution_clock::now();
                        response = tradeFacade.sellOrder(token, instrument, amount, std::nullopt, price, type);
                        auto end_time = std::chrono::high_resolution_clock::now();
                        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
                        std::cout << "Response: " << jsonToString(response) << std::endl;
                        std::cout << "Order placement latency: " << latency << " milliseconds" << std::endl;
                        break;
                    }

                    case 4:
                    { // Cancel Order
                        std::cout << "Enter order ID: ";
                        std::cin >> orderId;
                        auto start_time = std::chrono::high_resolution_clock::now();
                        response = tradeFacade.cancelOrder(orderId, token);
                        auto end_time = std::chrono::high_resolution_clock::now();
                        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
                        std::cout << "Response: " << jsonToString(response) << std::endl;
                        std::cout << "Order placement latency: " << latency << " milliseconds" << std::endl;
                        break;
                    }

                    case 5:
                    { // Cancel All Orders
                        auto start_time = std::chrono::high_resolution_clock::now();
                        response = tradeFacade.cancelAllOrder(token);
                        auto end_time = std::chrono::high_resolution_clock::now();
                        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
                        std::cout << "Response: " << jsonToString(response) << std::endl;
                        std::cout << "Order placement latency: " << latency << " milliseconds" << std::endl;
                        break;
                    }

                    case 6:
                    { // Get Open Orders
                        auto start_time = std::chrono::high_resolution_clock::now();
                        response = tradeFacade.getOpenOrder(token);
                        auto end_time = std::chrono::high_resolution_clock::now();
                        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
                        std::cout << "Response: " << jsonToString(response) << std::endl;
                        std::cout << "Order placement latency: " << latency << " milliseconds" << std::endl;
                        break;
                    }
                    case 7:
                    { // Get Order State
                        std::cout << "Enter order ID: ";
                        std::cin >> orderId;
                        auto start_time = std::chrono::high_resolution_clock::now();
                        response = tradeFacade.getOrderState(orderId, token);
                        auto end_time = std::chrono::high_resolution_clock::now();
                        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
                        std::cout << "Response: " << jsonToString(response) << std::endl;
                        std::cout << "Order placement latency: " << latency << " milliseconds" << std::endl;
                        break;
                    }

                    case 8:
                    { // Get OrderBook by Instrument
                        std::cout << "Enter instrument name: ";
                        std::cin >> instrument;
                        auto start_time = std::chrono::high_resolution_clock::now();
                        response = tradeFacade.getOrderBook(instrument);
                        auto end_time = std::chrono::high_resolution_clock::now();
                        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
                        std::cout << "Response: " << jsonToString(response) << std::endl;
                        cout << "Order placement latency: " << latency << " milliseconds" << endl;
                        break;
                    }
                    case 9:
                    { // Get Position by Instrument
                        std::cout << "Enter instrument name: ";
                        std::cin >> instrument;
                        auto start_time = std::chrono::high_resolution_clock::now();
                        response = tradeFacade.getPosition(token, instrument);
                        auto end_time = std::chrono::high_resolution_clock::now();
                        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
                        std::cout << "Response: " << jsonToString(response) << std::endl;
                        cout << "Order placement latency: " << latency << " milliseconds" << endl;
                        break;
                    }
                    case 10:
                    { // Get Positions
                        auto start_time = std::chrono::high_resolution_clock::now();
                        response = tradeFacade.getPositions(token);
                        auto end_time = std::chrono::high_resolution_clock::now();
                        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
                        std::cout << "Response: " << jsonToString(response) << std::endl;
                        cout << "Order placement latency: " << latency << " milliseconds" << endl;
                        break;
                    }
                    case 11: 
                    {// Exit
                        std::cout << "Returning to Network Menu...\n";
                        break;
                    }
                    default:
                        std::cout << "Invalid choice! Please try again.\n";
                    }
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Error: " << e.what() << std::endl;
                }

            } while (choice != 11);
            break;
        }

        case 3: // Exit
            std::cout << "Exiting program...\n";
            break;

        default:
            std::cout << "Invalid choice! Please enter 1, 2, or 3.\n";
        }
    } while (networkChoice != 3);

    return 0;
}