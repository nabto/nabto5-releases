#include <nabto_client.hpp>

#include <cxxopts.hpp>

#include <iostream>
#include <nlohmann/json.hpp>

#include <iostream>
#include <fstream>

#include "json_config.hpp"

using json = nlohmann::json;

const static int CONTENT_FORMAT_APPLICATION_CBOR = 60; // rfc 7059

void heat_pump_pair(const std::string& configFile, const std::string& productId, const std::string& deviceId, const std::string& server, const std::string& serverKey)
{
    json config;
    std::cout << "Pairing with heat pump " << productId << "." << deviceId << std::endl;

    auto ctx = nabto::client::Context::create();
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

    std::cout << "Press the pairing button on the heat pump to proceed" << std::endl;
    std::cout << "This clients fingerprint is " << connection->getClientFingerprintHex() << std::endl;

    auto coap = connection->createCoap("POST", "/pairing/button");
    coap->execute()->waitForResult();

    if (coap->getResponseStatusCode() == 205)
    {
        std::cout << "Paired with the device, writing configuration to disk" << std::endl;
        if (!json_config_save(configFile, config)) {
            std::cerr << "Failed to write config to " << configFile << std::endl;
        }
        exit(0);
    } else {
        std::string reason;
        auto buffer = coap->getResponsePayload();
        reason = std::string(reinterpret_cast<char*>(buffer->data()), buffer->size());
        std::cout << "Could not pair with the device status: " << coap->getResponseStatusCode() << " " << reason << std::endl;
        exit(1);
    }
}

void heat_pump_scan()
{
    std::cout << "Scanning for local devices" << std::endl;
    auto ctx = nabto::client::Context::create();
    auto resolver = ctx->createMdnsResolver();
    for (;;) {
        auto res = resolver->getResult()->waitForResult();
        std::cout << "Device " << res->getProductId() << "." << res->getDeviceId() << " address: " << res->getAddress() << " port: " << res->getPort() << std::endl;
    }
}

std::shared_ptr<nabto::client::Connection> createConnection(std::shared_ptr<nabto::client::Context> context, const std::string& configFile)
{
    json config;
    if(!json_config_load(configFile, config)) {
        std::cerr << "Could not read config file" << std::endl;
        exit(1);
    }

    auto connection = context->createConnection();
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


    return connection;
}

void handle_coap_error(std::shared_ptr<nabto::client::Coap> coap)
{
    std::string data;
    int contentFormat;
    contentFormat = coap->getResponseContentFormat();
    if (contentFormat == -1) {
        // no content format handle response as a string
        auto buffer = coap->getResponsePayload();
        data = std::string(reinterpret_cast<char*>(buffer->data()), buffer->size());
    } else if (contentFormat == CONTENT_FORMAT_APPLICATION_CBOR) {
        auto buffer = coap->getResponsePayload();
        std::vector<uint8_t> cbor(buffer->data(), buffer->data()+buffer->size());
        data = json::from_cbor(cbor);
    }

    std::cout << "Response Code: " << coap->getResponseStatusCode() << " " << data << std::endl;
}

void heat_pump_get(std::shared_ptr<nabto::client::Connection> connection)
{
    auto coap = connection->createCoap("GET", "/heat-pump");
    coap->execute()->waitForResult();
    if (coap->getResponseStatusCode() == 205 && coap->getResponseContentFormat() == CONTENT_FORMAT_APPLICATION_CBOR)
    {
        auto buffer = coap->getResponsePayload();
        std::vector<uint8_t> cbor(buffer->data(), buffer->data()+buffer->size());
        std::cout << json::from_cbor(cbor) << std::endl;
        connection->close()->waitForResult();
        exit(0);
    } else {
        handle_coap_error(coap);
        connection->close()->waitForResult();
        exit(1);
    }
}

void heat_pump_set_data(std::shared_ptr<nabto::client::Coap> coap, json doc)
{
    std::vector<uint8_t> cbor = json::to_cbor(doc);
    auto buffer = std::make_shared<nabto::client::BufferImpl>(cbor);
    coap->setRequestPayload(CONTENT_FORMAT_APPLICATION_CBOR, buffer);
}

void heat_pump_set_target(std::shared_ptr<nabto::client::Connection> connection, double target)
{
    json root;
    root = target;

    std::cout << "target " << target << std::endl;

    auto coap = connection->createCoap("POST", "/heat-pump/target");
    heat_pump_set_data(coap, root);
    coap->execute()->waitForResult();
    if (coap->getResponseStatusCode() == 204)
    {
        std::cout << "Target temperature set" << std::endl;
        connection->close()->waitForResult();
        exit(0);
    } else {
        handle_coap_error(coap);
        connection->close()->waitForResult();
        exit(1);
    }
}

void heat_pump_set_power(std::shared_ptr<nabto::client::Connection> connection, std::string power)
{
    json root;
    if (power == "ON") {
        root = true;
    } else if (power == "OFF") {
        root = false;
    } else {
        std::cerr << "Invalid power state " << power << std::endl;
        exit(1);
    }

    auto coap = connection->createCoap("POST", "/heat-pump/power");
    heat_pump_set_data(coap, root);
    coap->execute()->waitForResult();
    if (coap->getResponseStatusCode() == 204)
    {

        std::cout << "Power set" << std::endl;
        connection->close()->waitForResult();
        exit(0);
    } else {
        handle_coap_error(coap);
        connection->close()->waitForResult();
        exit(1);
    }
}

void heat_pump_set_mode(std::shared_ptr<nabto::client::Connection> connection, const std::string& mode)
{
    json root;
    root = mode;

    auto coap = connection->createCoap("POST", "/heat-pump/mode");
    heat_pump_set_data(coap, root);
    coap->execute()->waitForResult();
    if (coap->getResponseStatusCode() == 204)
    {

        std::cout << "Mode set" << std::endl;
        connection->close()->waitForResult();
        exit(0);
    } else {
        handle_coap_error(coap);
        connection->close()->waitForResult();
        exit(1);
    }
}

void iam_users_list(std::shared_ptr<nabto::client::Connection> connection)
{
    auto coap = connection->createCoap("GET", "/iam/users");
    coap->execute()->waitForResult();
    if (coap->getResponseStatusCode() == 205 && coap->getResponseContentFormat() == CONTENT_FORMAT_APPLICATION_CBOR)
    {
        auto buffer = coap->getResponsePayload();
        std::vector<uint8_t> cbor(buffer->data(), buffer->data()+buffer->size());
        std::cout << json::from_cbor(cbor) << std::endl;
        connection->close()->waitForResult();
        exit(0);
    } else {
        handle_coap_error(coap);
        connection->close()->waitForResult();
        exit(1);
    }
}

void iam_users_get(std::shared_ptr<nabto::client::Connection> connection, const std::string& userName)
{
    std::stringstream path;
    path << "/iam/users/" << userName;
    auto coap = connection->createCoap("GET", path.str());
    coap->execute()->waitForResult();
    if (coap->getResponseStatusCode() == 205 && coap->getResponseContentFormat() == CONTENT_FORMAT_APPLICATION_CBOR)
    {
        auto buffer = coap->getResponsePayload();
        std::vector<uint8_t> cbor(buffer->data(), buffer->data()+buffer->size());
        std::cout << json::from_cbor(cbor) << std::endl;
        connection->close()->waitForResult();
        exit(0);
    } else {
        handle_coap_error(coap);
        connection->close()->waitForResult();
        exit(1);
    }
}

void iam_users_delete(std::shared_ptr<nabto::client::Connection> connection, const std::string& userName)
{
    std::stringstream path;
    path << "/iam/users/" << userName;
    auto coap = connection->createCoap("DELETE", path.str());
    coap->execute()->waitForResult();
    if (coap->getResponseStatusCode() == 202)
    {
        std::cout << "User deleted" << std::endl;
        connection->close()->waitForResult();
        exit(0);
    } else {
        handle_coap_error(coap);
        connection->close()->waitForResult();
        exit(1);
    }
}

void iam_users_add_fingerprint(std::shared_ptr<nabto::client::Connection> connection, const std::string& userName, const std::string& fingerprint)
{
    std::stringstream path;
    path << "/iam/users/" << userName << "/fingerprints/" << fingerprint;
    auto coap = connection->createCoap("PUT", path.str());
    coap->execute()->waitForResult();
    if (coap->getResponseStatusCode() == 201)
    {
        std::cout << "Fingerprint added" << std::endl;
        connection->close()->waitForResult();
        exit(0);
    } else {
        handle_coap_error(coap);
        connection->close()->waitForResult();
        exit(1);
    }
}

void iam_users_remove_fingerprint(std::shared_ptr<nabto::client::Connection> connection, const std::string& userName, const std::string& fingerprint)
{
    std::stringstream path;
    path << "/iam/users/" << userName << "/fingerprints/" << fingerprint;
    auto coap = connection->createCoap("DELETE", path.str());
    coap->execute()->waitForResult();
    if (coap->getResponseStatusCode() == 202)
    {
        std::cout << "Fingerprint removed" << std::endl;
        connection->close()->waitForResult();
        exit(0);
    } else {
        handle_coap_error(coap);
        connection->close()->waitForResult();
        exit(1);
    }
}

int main(int argc, char** argv)
{
    cxxopts::Options options("Heat pump", "Nabto heat pump client example.");

    options.add_options("General")
        ("h,help", "Show help")
        ("version", "Show version")
        ("c,config", "Configuration file", cxxopts::value<std::string>()->default_value("heat_pump_client.json"))
        ("scan", "Scan for heat pumps");

    options.add_options("Pairing")
        ("pair", "Pair with a device")
        ("p,product", "Product id", cxxopts::value<std::string>())
        ("d,device", "Device id", cxxopts::value<std::string>())
        ("s,server", "Server url of basestation", cxxopts::value<std::string>())
        ("k,server-key", "Key to use with the server", cxxopts::value<std::string>());

    options.add_options("IAM")
        ("users-get", "Get a user")
        ("users-list", "List Users")
        ("users-delete", "Delete user")
        ("users-add-fingerprint", "Add a fingerprint to a user")
        ("users-remove-fingerprint", "Add a fingerprint to a user")
        ("user", "User name", cxxopts::value<std::string>())
        ("fingerprint", "Fingerprint", cxxopts::value<std::string>());

    options.add_options("Heatpump")
        ("get", "Get heatpump state")
        ("set-target", "Set target temperature", cxxopts::value<double>())
        ("set-power", "Turn ON or OFF", cxxopts::value<std::string>())
        ("set-mode", "Set heatpump mode, valid modes: COOL, HEAT, FAN, DRY", cxxopts::value<std::string>());

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

    if (result.count("scan")) {
        heat_pump_scan();
        exit(0);
    }

    if (result.count("pair")) {
        if (result.count("product") == 0 ||
            result.count("device") == 0 ||
            result.count("server") == 0 ||
            result.count("server-key") == 0)
        {
            std::cout << "missing options for pairing" << std::endl;
            std::cout << options.help() << std::endl;
            exit(1);
        }
        heat_pump_pair(result["config"].as<std::string>(),
                       result["product"].as<std::string>(),
                       result["device"].as<std::string>(),
                       result["server"].as<std::string>(),
                       result["server-key"].as<std::string>());
        exit(0);
    }

    auto context = nabto::client::Context::create();
    auto connection = createConnection(context, result["config"].as<std::string>());

    if (result.count("get")) {
        heat_pump_get(connection);
    } else if (result.count("users-list")) {
        iam_users_list(connection);
    } else if (result.count("users-get")) {
        iam_users_get(connection, result["user"].as<std::string>());
    } else if (result.count("users-delete")) {
        iam_users_delete(connection, result["user"].as<std::string>());
    } else if (result.count("users-add-fingerprint")) {
        iam_users_add_fingerprint(connection, result["user"].as<std::string>(), result["fingerprint"].as<std::string>());
    } else if (result.count("users-remove-fingerprint")) {
        iam_users_remove_fingerprint(connection, result["user"].as<std::string>(), result["fingerprint"].as<std::string>());
    } else if (result.count("set-target")) {
        heat_pump_set_target(connection, result["set-target"].as<double>());
    } else if (result.count("set-mode")) {
        heat_pump_set_mode(connection, result["set-mode"].as<std::string>());
    } else if (result.count("set-power")) {
        heat_pump_set_power(connection, result["set-power"].as<std::string>());
    } else {
        std::cerr << "No such command" << std::endl;
        exit(1);
    }
}
