cmake_minimum_required(VERSION 3.15)
project(GoQuant VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

set(SOURCES
    src/main.cpp
    src/WebSocketClient.cpp
    src/UtilsController.cpp
    src/TradeFacade.cpp
    src/TradeController.cpp
    src/ConnectionController.cpp
)

add_executable(GoQuant ${SOURCES})

target_link_libraries(GoQuant PRIVATE
    Boost::system
    OpenSSL::SSL
    OpenSSL::Crypto
    CURL::libcurl
    nlohmann_json::nlohmann_json 
)

target_include_directories(GoQuant PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${nlohmann_json_INCLUDE_DIRS} 
)

# 4. Include the generated header directory (if still needed)
target_include_directories(GoQuant PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

message(STATUS "CMake Toolchain File: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "Boost Include Dir: ${Boost_INCLUDE_DIRS}")
message(STATUS "CURL Include Dir: ${CURL_INCLUDE_DIRS}")
message(STATUS "OpenSSL Include Dir: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "nlohmann_json Include Dir: ${nlohmann_json_INCLUDE_DIRS}")