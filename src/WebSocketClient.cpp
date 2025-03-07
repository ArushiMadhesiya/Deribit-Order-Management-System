// WebSocketClient.cpp
#include "WebSocketClient.h"
#include <iostream>
#include <chrono>
#include <boost/asio/ssl.hpp>
#include <iostream>
#include <chrono>
#include <boost/asio/ssl.hpp>

using json = nlohmann::json; 
// Constructor initializes the client object and sets up default values
WebSocketClient::WebSocketClient()
    : connected(false)
    , should_run(true)
    , m_isRunning(false)
{
    // Initialize the client library
    client.init_asio(); 

    // Set TLS initialization handler for secure connections
    client.set_tls_init_handler([](websocketpp::connection_hdl) {
        return websocketpp::lib::make_shared<boost::asio::ssl::context>(
            boost::asio::ssl::context::sslv23);
    });

    // Configure logging levels
    client.clear_access_channels(websocketpp::log::alevel::all); 
    client.set_access_channels(websocketpp::log::alevel::connect);
    client.set_access_channels(websocketpp::log::alevel::disconnect);
    client.set_access_channels(websocketpp::log::alevel::fail);

    // Register event handlers for different connection states
    client.set_open_handler(std::bind(&WebSocketClient::on_open, this, std::placeholders::_1));
    client.set_close_handler(std::bind(&WebSocketClient::on_close, this, std::placeholders::_1));
    client.set_message_handler(std::bind(&WebSocketClient::on_message, this, std::placeholders::_1, std::placeholders::_2));
    client.set_fail_handler(std::bind(&WebSocketClient::on_error, this, std::placeholders::_1));
}

WebSocketClient::~WebSocketClient() {
    // Gracefully close the connection
    close();
}
bool WebSocketClient::connect(const std::string& host, const std::string& port) {
    m_host = host;
    m_port = port;
    std::string uri = "wss://" + host + ":" + port + "/ws/api/v2/";

    websocketpp::lib::error_code ec;
    Client::connection_ptr con = client.get_connection(uri, ec);

    if (ec) {
        std::cerr << "Connect initialization error: " << ec.message() << std::endl;
        return false;
    }

    client.connect(con);

    std::thread([this]() {
        try {
            client.run();
        } catch (const std::exception& e) {
            std::cerr << "Error in client run loop: " << e.what() << std::endl;
        }
    }).detach();

    int attempts = 0;
    while (!connected && attempts < 50) { // 5 second timeout
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        attempts++;
    }

    if (!connected) { // Check connection status after timeout
        std::cerr << "Connection timed out" << std::endl;
        return false; // Return false if connection failed
    }

    return connected;
}

std::string WebSocketClient::constructSubscriptionMessage(const std::string& channel, const std::string& token) {
    json document; // Use nlohmann::json
    document["jsonrpc"] = "2.0";
    document["id"] = 1;
    document["method"] = "private/subscribe";

    json params;
    params["access_token"] = token;

    json channels = json::array();
    channels.push_back(channel);
    params["channels"] = channels;

    document["params"] = params;

    return document.dump();
}

// Subscribe to a specific channel
bool WebSocketClient::subscribe(const std::string& channel, const std::string& token) {
    if (!connected) {
        std::cerr << "Not connected to server" << std::endl;
        return false;
    }

    std::string message = constructSubscriptionMessage(channel, token);

    websocketpp::lib::error_code ec;
    client.send(connection, message, websocketpp::frame::opcode::text, ec);

    if (ec) {
        std::cerr << "Send error: " << ec.message() << std::endl;
        return false;
    }

    std::cout << "Subscribed to channel: " << channel << std::endl;
    return true;
}

// Main loop for listening to incoming messages
void WebSocketClient::listen() {
    while (should_run) {
        std::unique_lock<std::mutex> lock(queueMutex); 
        if (!messageQueue.empty()) {
            std::string message = messageQueue.front();
            messageQueue.pop();
            lock.unlock();

            processMessage(message);
        } else {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
        }
    }
}

// Process incoming messages 
void WebSocketClient::processMessage(const std::string& message) {
    try {
        json document = json::parse(message); // Use nlohmann::json

        if (document.is_discarded()) { // Check for parse errors
            std::cerr << "Error parsing JSON message" << std::endl;
            return;
        }

        // Calculate propagation delay if timestamp information is available
        if (document.contains("params") &&
            document["params"].contains("data") &&
            document["params"]["data"].contains("timestamp")) {

            try { // Inner try-catch for potential type errors in the timestamp
                auto server_time = document["params"]["data"]["timestamp"].get<int64_t>();
                auto client_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();

                auto propagation_delay = client_time - server_time;
                std::cout << "Propagation delay: " << propagation_delay << " ms" << std::endl;
            } catch (const json::type_error& e) {
                std::cerr << "Error getting timestamp: " << e.what() << std::endl;
            }
        }

        // Call the user-defined message handler if provided
        if (messageHandler) {
            messageHandler(message);
        }
    } catch (const json::parse_error& e) {  // Catch specific json parse errors
        std::cerr << "JSON Parse error: " << e.what() << std::endl;
    } catch (const std::exception& e) { // Catch other exceptions
        std::cerr << "Error processing message: " << e.what() << std::endl;
    }
}

// Attempt to reconnect to the server
bool WebSocketClient::reconnect() {
    close();
    std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_DELAY_MS)); 
    return connect(m_host, m_port);
}

// Close the WebSocket connection gracefully
void WebSocketClient::close() {
    should_run = false;
    m_isRunning = false;

    if (connected) {
        websocketpp::lib::error_code ec;
        client.close(connection, websocketpp::close::status::normal, "Closing connection", ec);

        if (ec) {
            std::cerr << "Error closing connection: " << ec.message() << std::endl;
        } else {
            std::cout << "Connection closed successfully." << std::endl;
        }
    }

    if (m_listenerThread.joinable()) {
        m_listenerThread.join();
    }
}
// Start a new WebSocket session
void WebSocketClient::startWebSocketSession(const std::string& token) {
    if (token.empty()) {
        throw std::runtime_error("Invalid token");
    }

    try {
        if (!connect("test.deribit.com", "443")) {
            throw std::runtime_error("Failed to connect to WebSocket server");
        }

        std::string symbol;
        std::cout << "Enter the instrument/symbol (e.g., BTC-PERPETUAL): ";
        std::cin >> symbol;
        if (symbol.empty()) {
            throw std::runtime_error("Invalid instrument symbol");
        }

        int intervalChoice;
        std::cout << "Choose the interval:\n1. 100ms\n2. raw\n3. agg2\n";
        std::cin >> intervalChoice;

        std::string interval;
        switch (intervalChoice) {
            case 1:
                interval = "100ms";
                break;
            case 2:
                interval = "raw";
                break;
            case 3:
                interval = "agg2";
                break;
            default:
                throw std::runtime_error("Invalid interval choice");
        }

        std::string subscription = "book." + symbol + "." + interval;

        if (!subscribe(subscription, token)) {
            throw std::runtime_error("Subscription failed");
        }

        m_isRunning = true;
        m_listenerThread = std::thread([this]() {
            int reconnectAttempts = 0;
            while (m_isRunning) {
                try {
                    listen();
                    reconnectAttempts = 0; // Reset attempts on successful loop
                } catch (const std::exception& e) {
                    std::cerr << "WebSocket error: " << e.what() << std::endl;
                    if (m_isRunning && reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
                        reconnectAttempts++;
                        std::cerr << "Reconnection attempt " << reconnectAttempts << "..." << std::endl; // Indicate attempt
                        if (!reconnect()) {
                            std::cerr << "Reconnection attempt " << reconnectAttempts << " failed" << std::endl;
                        }
                    } else {
                        m_isRunning = false;
                        std::cerr << "Max reconnection attempts reached or session stopped" << std::endl;
                    }
                }
            }
        });

        m_listenerThread.detach();

    } catch (const std::exception& e) {
        m_isRunning = false; // Ensure m_isRunning is set to false in the outer catch too
        std::cerr << "Failed to start WebSocket session: " << e.what() << std::endl;
        throw; // Re-throw the exception after setting m_isRunning to false.
    }
}
void WebSocketClient::on_open(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(mutex_);
    connection = hdl;
    connected = true;
    std::cout << "Connection established" << std::endl;
}

void WebSocketClient::on_close(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(mutex_);
    connection.reset(); // Important: Reset the connection handle
    connected = false;
    std::cout << "Connection closed" << std::endl;
}

void WebSocketClient::on_message(websocketpp::connection_hdl hdl, MessagePtr msg) {
    std::lock_guard<std::mutex> lock(queueMutex);
    messageQueue.push(msg->get_payload());
}

void WebSocketClient::on_error(websocketpp::connection_hdl hdl) {
    Client::connection_ptr con = client.get_con_from_hdl(hdl);
    if (con) { // Check if the connection exists before accessing it.
       std::cerr << "Error occurred. Error: " << con->get_ec().message() << std::endl;
    } else {
        std::cerr << "Error occurred, but connection handle is no longer valid." << std::endl;
    }
}