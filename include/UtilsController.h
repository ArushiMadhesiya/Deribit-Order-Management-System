#ifndef UTILS_H
#define UTILS_H
#include <iostream>
#include <chrono>
#include <stdexcept>
#include <string>
#include <filesystem> // For filesystem operations
#include <fstream>    // For file I/O
#include <vector>
#include <tuple>
#include <thread>
#include <future>
#include <iomanip> // For std::setprecision
#include <sstream> // for stringstream

#include "TradeFacade.h"

using namespace std;
using json = nlohmann::json; 

string jsonToString( json & doc);
void prettyPrintJson(const std::string &json);
void showNetworkMenu();
void showTradeMenu();

#endif