#include "UtilsController.h"
#include <iostream>
#include <sstream>
using namespace std;

using json = nlohmann::json; 
std::string jsonToString( json & document) {
   return document.dump(1);
}

void prettyPrintJson(const std::string &json) {
    int indentLevel = 0; 
    bool inString = false;
    
    for (char c : json) {
        // Handle string literals
        if (c == '"') {
            inString = !inString;
        }
        
        if (!inString) {
            if (c == '{' || c == '[') {
                std::cout << c << "\n";
                indentLevel++;
                for (int i = 0; i < indentLevel; ++i) {
                    std::cout << "    ";  // 4 spaces for indentation
                }
            } else if (c == '}' || c == ']') {
                std::cout << "\n";
                indentLevel--;
                for (int i = 0; i < indentLevel; ++i) {
                    std::cout << "    ";  // 4 spaces for indentation
                }
                std::cout << c;
            } else if (c == ',') {
                std::cout << c << "\n";
                for (int i = 0; i < indentLevel; ++i) {
                    std::cout << "    ";  // 4 spaces for indentation
                }
            } else if (c == ':') {
                std::cout << ": ";
            } else if (c != ' ' && c != '\n' && c != '\r') {
                std::cout << c;
            }
        } else {
            std::cout << c;
        }
    }
}
// Display the main menu for network connection selection
void showNetworkMenu(){
    cout << "Establish connection via:" << endl;
    cout << "1. Using WebSocket\n";
    cout << "2. Over HTTP\n";
    cout << "3. Exit\n";
    cout << "Enter your choice: ";
}

// Display the main trading menu
void showTradeMenu() {
    cout << "\n======= Order Management System =======\n";
    cout << "1. Place Order\n";
    cout << "2. Modify Order\n";
    cout << "3. Sell Order\n";
    cout << "4. Cancel Order\n";
    cout << "5. Cancel All Orders\n";
    cout << "6. Get Open Orders\n";
    cout << "7. Get Order State\n";
    cout << "8. Get OrderBook by Instrument\n";
    cout << "9. Get Position by Instrument\n";
    cout << "10. Get Positions\n";
    cout << "11. Exit to Network Selection Menu\n";
    cout << "======================================\n";
    cout << "Enter your choice: ";
}
