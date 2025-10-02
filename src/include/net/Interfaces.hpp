#pragma once

#include <cstdint>
#include <string>
#include <vector>

class MacAddress {
private:
    std::vector<uint8_t> int_addr;

public:
    void assign(const std::vector<uint8_t>& addr) { int_addr = addr; }
    const std::vector<uint8_t>& get() const { return int_addr; }
    bool operator==(const MacAddress& other) const { return int_addr == other.int_addr; }
    std::string toString() const;
};

class NetworkInterface {
public:
    std::string name;
    uint32_t ip;
    uint32_t netmask;
    uint32_t broadcast;
    std::string netmaskString;
    std::string broadcastString;
    MacAddress mac;
    bool isUp;
    bool isLoopback;
    bool isRunning;
    bool isWireless;
    int mtu;

    std::string ipString() const;
};

class NetworkRegistry {
public:
    std::vector<NetworkInterface> interfaces;
    NetworkInterface* getInterfaceByName(const std::string& name);
    NetworkInterface* getInterfaceByIP(uint32_t ip);
    NetworkInterface* getInterfaceByMAC(const MacAddress& addr);
    void scanForInterfaces();
};
