#include "nabto_client.hpp"
#include <nabto/nabto_client_experimental.h>
#include "json_config.hpp"
#include "timestamp.hpp"

#include <cxxopts.hpp>
#include <nlohmann/json.hpp>

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <thread>

using json = nlohmann::json;

enum {
  COAP_CONTENT_FORMAT_APPLICATION_CBOR = 60
};

// TODO reconnect when connection is closed.

class MyLogger : public nabto::client::Logger
{
 public:
    void log(nabto::client::LogMessage message) {
        std::cout << time_in_HH_MM_SS_MMM() << "[" << message.getSeverity() << "] " << message.getMessage() << std::endl;
    }
};

std::shared_ptr<nabto::client::Connection> connection_;
std::shared_ptr<nabto::client::ConnectionEventsListener> connectionEventsListener_;

void my_handler(int s){
    printf("Caught signal %d\n",s);
    connectionEventsListener_->stop();
    connection_->close();
}

static bool isPaired(std::shared_ptr<nabto::client::Connection> connection);

std::shared_ptr<nabto::client::Connection> createConnection(std::shared_ptr<nabto::client::Context> ctx, const std::string& logLevel, const std::string& configFile)
{
    json config;
    if(!json_config_load(configFile, config)) {
        std::cerr << "Could not read config file" << std::endl;
        exit(1);
    }

    if (!logLevel.empty()) {
        ctx->setLogger(std::make_shared<MyLogger>());
        ctx->setLogLevel(logLevel);
    }

    auto connection = ctx->createConnection();
    connection->setProductId(config["ProductId"].get<std::string>());
    connection->setDeviceId(config["DeviceId"].get<std::string>());
    connection->setServerUrl(config["ServerUrl"].get<std::string>());
    connection->setServerKey(config["ServerKey"].get<std::string>());
    connection->setPrivateKey(config["PrivateKey"].get<std::string>());
    try {
        connection->connect()->waitForResult();
    } catch (std::exception& e) {
        std::cerr << "Connect failed" << e.what() << std::endl;
        exit(1);
    }

    try {
        if (connection->getDeviceFingerprintHex() != config["DeviceFingerprint"].get<std::string>()) {
            std::cerr << "device fingerprint does not match the paired fingerprint." << std::endl;
            exit(1);
        }
    } catch (...) {
        std::cerr << "Missing device fingerprint in config, pair with the device again" << std::endl;
        exit(1);
    }

    if (!isPaired(connection)) {
        std::cerr << "Client is not paired with device, do the pairing again" << std::endl;
        exit(1);
    }

    return connection;
}

bool isPaired(std::shared_ptr<nabto::client::Connection> connection)
{
    auto coap = connection->createCoap("GET", "/pairing/is-paired");

    try {
        coap->execute()->waitForResult();

        return (coap->getResponseStatusCode() == 205);

    } catch(...) {
        std::cerr << "Cannot get pairing state" << std::endl;
        exit(1);
    }
}

void tcptunnel(const std::string& logLevel, const std::string& configFile, uint16_t localPort, const std::string& remoteHost, uint16_t remotePort)
{
    std::cout << "Creating tunnel " << configFile << " local port " << localPort << " remote host " << remoteHost << " remote port " << remotePort << std::endl;

    auto ctx = nabto::client::Context::create();

    auto connection = createConnection(ctx, logLevel, configFile);

    std::shared_ptr<nabto::client::TcpTunnel> tunnel;
    try {
        tunnel = connection->createTcpTunnel();
        tunnel->open(localPort, remoteHost, remotePort)->waitForResult();
    } catch (std::exception& e) {
        std::cout << "open tunnel error: " << e.what() << std::endl;
        connection->close();
        return;
    }
    std::cout << "tunnel is opened" << std::endl;

    // wait for ctrl c
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    auto listener = connection->createEventsListener();

    connection_ = connection;
    connectionEventsListener_ = listener;

    std::thread t([listener](){
            try {
                while (true) {
                    nabto::client::ConnectionEvent ce = listener->listen()->waitForResult();
                    if (ce.getEvent() == NABTO_CLIENT_CONNECTION_EVENT_CLOSED) {
                        std::cout << "Connection closed, closing application" << std::endl;
                        return;
                    }
                }
            } catch (...) {
                return;
            }
        });

    t.join();

    connection_.reset();
    connectionEventsListener_.reset();
}

void tcptunnel_pairing(const std::string& logLevel, const std::string& configFile, const std::string& productId, const std::string& deviceId, const std::string& server, const std::string& serverKey, const std::string& password)
{
    json config;
    std::cout << "Pairing with tcp tunnel " << productId << "." << deviceId << std::endl;
    auto ctx = nabto::client::Context::create();

    if (!logLevel.empty()) {
        ctx->setLogger(std::make_shared<MyLogger>());
        ctx->setLogLevel(logLevel);
    }

    auto connection = ctx->createConnection();
    connection->setProductId(productId);
    connection->setDeviceId(deviceId);
    connection->setServerUrl(server);
    connection->setServerKey(serverKey);

    std::string privateKey = ctx->createPrivateKey();
    connection->setPrivateKey(privateKey);

    config["ProductId"] = productId;
    config["DeviceId"] = deviceId;
    config["ServerUrl"] = server;
    config["ServerKey"] = serverKey;
    config["PrivateKey"] = privateKey;

    try {
        connection->connect()->waitForResult();
    } catch (std::exception& e) {
        std::cout << "Connect failed" << e.what() << std::endl;
        exit(1);
    }

    std::cout << "Connected to device with fingerprint " << connection->getDeviceFingerprintHex() << std::endl;
    std::cout << "Is the fingerprint valid [yn]" << std::endl;

    for (;;) {
        char input;
        std::cin >> input;
        if (input == 'y') {
            break;
        }
        if (input == 'n') {
            std::cout << "Fingerprint not accepted, quitting" << std::endl;
            exit(1);
        }
        std::cout << "only y or n is accepted as answers" << std::endl;
    }

    config["DeviceFingerprint"] = connection->getDeviceFingerprintHex();

    auto coap = connection->createCoap("POST", "/pairing/password");

    json pwd = password;
    auto cbor = std::make_shared<nabto::client::BufferImpl>(json::to_cbor(pwd));
    coap->setRequestPayload(COAP_CONTENT_FORMAT_APPLICATION_CBOR, cbor);

    coap->execute()->waitForResult();

    if (coap->getResponseStatusCode() == 201 || coap->getResponseStatusCode() == 205 /* we returned wrong status code(205) before 2019-11-13*/)
    {
        std::cout << "Paired with the device, writing configuration to disk" << std::endl;
        if (!json_config_save(configFile, config)) {
            std::cerr << "Failed to write config to " << configFile << std::endl;
        }
        connection->close();
        return;
    } else {
        std::string reason;
        auto buffer = coap->getResponsePayload();
        reason = std::string(reinterpret_cast<char*>(buffer->data()), buffer->size());
        std::cout << "Could not pair with the device status: " << coap->getResponseStatusCode() << " " << reason << std::endl;
        connection->close();
        return;
    }
}

int main(int argc, char** argv)
{
    cxxopts::Options options("Tunnel client", "Nabto tunnel client example.");

    options.add_options("General")
        ("h,help", "Show help")
        ("version", "Show version")
        ("password-pairing", "Do a pairing with the device using a password")
        ("tcptunnel", "Create a tcp tunnel with the device.")
        ("c,config", "Configutation File", cxxopts::value<std::string>()->default_value("tcptunnel_client.json"))
        ("log-level", "Log level (error|info|trace)", cxxopts::value<std::string>()->default_value(""))
        ;

    options.add_options("Pairing")
        ("p,product", "Product id", cxxopts::value<std::string>())
        ("d,device", "Device id", cxxopts::value<std::string>())
        ("s,server", "Server url of basestation", cxxopts::value<std::string>())
        ("k,server-key", "Key to use with the server", cxxopts::value<std::string>())
        ("password", "Password to use in the pairing with the device", cxxopts::value<std::string>())
        ;

    options.add_options("TCPTunnelling")
        ("local-port", "Local port to bind tcp listener to", cxxopts::value<uint16_t>()->default_value("0"))
        ("remote-host", "Remote ip to connect to", cxxopts::value<std::string>()->default_value(""))
        ("remote-port", "Remote port to connect to", cxxopts::value<uint16_t>()->default_value("0"));

    auto result = options.parse(argc, argv);
    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    if (result.count("version"))
    {
        std::cout << "nabto_client_sdk: " << nabto::client::Context::version() << std::endl;
        exit(0);
    }

    if (result.count("password-pairing")) {
        try {
            tcptunnel_pairing(result["log-level"].as<std::string>(),
                              result["config"].as<std::string>(),
                              result["product"].as<std::string>(),
                              result["device"].as<std::string>(),
                              result["server"].as<std::string>(),
                              result["server-key"].as<std::string>(),
                              result["password"].as<std::string>());
        } catch(...) {
            std::cout << "Missing required option" << std::endl;
            std::cout << options.help() << std::endl;
            exit(1);
        }
    } else if (result.count("tcptunnel")) {
        try {
            tcptunnel(result["log-level"].as<std::string>(),
                      result["config"].as<std::string>(),
                      result["local-port"].as<uint16_t>(),
                      result["remote-host"].as<std::string>(),
                      result["remote-port"].as<uint16_t>());
        } catch (const std::exception& e) {
            std::cout << e.what() << std::endl;
            exit(1);
        }
    } else {
        std::cout << "Missing mode option" << std::endl;
        std::cout << options.help() << std::endl;
        exit(1);
    }
}
