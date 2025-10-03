#include "Interfaces.hpp"
#include <ifaddrs.h>
#include <iostream>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#ifdef __sun__
#include <sys/ethernet.h>
#else
#include <net/ethernet.h>
#endif

#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <map>
#include <filesystem>

#ifdef __linux__
#include <netpacket/packet.h>
#else
#include <sys/types.h>
#include <netinet/in.h>
#include <net/if_dl.h>
#endif

#ifdef __linux__
#include <netpacket/packet.h>
#else
#include <sys/types.h>
#include <netinet/in.h>
#endif

#ifdef __linux__
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#elif defined(__sun__) || defined(__sun)
#include <ifaddrs.h>
#include <net/if.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_dl.h>
#endif

std::string MacAddress::toString() const {
    char buf[18];
    if (int_addr.size() == 6) {
        snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
                 int_addr[0], int_addr[1], int_addr[2],
                 int_addr[3], int_addr[4], int_addr[5]);
    } else {
        snprintf(buf, sizeof(buf), "00:00:00:00:00:00");
    }
    return std::string(buf);
}

std::string NetworkInterface::ipString() const {
    struct in_addr addr;
    addr.s_addr = htonl(ip);
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr, buf, INET_ADDRSTRLEN);
    return std::string(buf);
}

std::string NetworkInterface::subnetString() const {
    struct in_addr addr;
    addr.s_addr = htonl(subnet);
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr, buf, INET_ADDRSTRLEN);
    return std::string(buf);
}

int NetworkInterface::getCIDR() const {
    uint32_t mask = netmask;
    int count = 0;
    while (mask) {
        count += mask & 1;
        mask >>= 1;
    }
    return count;
}

std::string NetworkInterface::subnetCIDR() const {
    return subnetString() + "/" + std::to_string(getCIDR());
}

NetworkInterface* NetworkRegistry::getInterfaceByName(const std::string& name) {
    for (auto& iface : interfaces)
        if (iface.name == name)
            return &iface;
    return nullptr;
}

NetworkInterface* NetworkRegistry::getInterfaceByIP(uint32_t ip) {
    for (auto& iface : interfaces)
        if (iface.ip == ip)
            return &iface;
    return nullptr;
}

NetworkInterface* NetworkRegistry::getInterfaceByMAC(const MacAddress& addr) {
    for (auto& iface : interfaces)
        if (iface.mac == addr)
            return &iface;
    return nullptr;
}

void NetworkRegistry::scanForInterfaces() {
    interfaces.clear();
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) return;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        freeifaddrs(ifaddr);
        return;
    }

    std::map<std::string, NetworkInterface> ifaceMap;

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;

        std::string name = ifa->ifa_name;
        
        if (ifaceMap.find(name) == ifaceMap.end()) {
            NetworkInterface iface;
            iface.name = name;
            iface.isUp = (ifa->ifa_flags & IFF_UP) != 0;
            iface.isLoopback = (ifa->ifa_flags & IFF_LOOPBACK) != 0;
            iface.isRunning = (ifa->ifa_flags & IFF_RUNNING) != 0;
            iface.isWireless = false;
            iface.mtu = 0;
            iface.ip = iface.netmask = iface.broadcast = iface.subnet = 0;

            // MTU
            struct ifreq ifr;
            memset(&ifr, 0, sizeof(ifr));
            strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ-1);
            ifr.ifr_addr.sa_family = AF_INET;

#ifndef __sun__
            if (ioctl(sock, SIOCGIFMTU, &ifr) == 0)
                iface.mtu = ifr.ifr_mtu;
#endif

#ifdef __linux__
            // Linux MAC address
            if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0)
                iface.mac.assign(std::vector<uint8_t>(ifr.ifr_hwaddr.sa_data, ifr.ifr_hwaddr.sa_data + 6));
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__) || defined(__MidnightBSD__) || defined(__Bitrig__)
            // BSD MAC address
            if (ifa->ifa_addr->sa_family == AF_LINK) {
                struct sockaddr_dl* sdl = (struct sockaddr_dl*)ifa->ifa_addr;
                unsigned char* mac = (unsigned char*)LLADDR(sdl);
                iface.mac.assign(std::vector<uint8_t>(mac, mac + 6));
            }
#endif
            ifaceMap[name] = iface;
        }

        // IPv4 info
        if (ifa->ifa_addr->sa_family == AF_INET) {
            NetworkInterface& iface = ifaceMap[name];
            
            sockaddr_in* sa = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
            iface.ip = ntohl(sa->sin_addr.s_addr);

            sockaddr_in* nm = reinterpret_cast<sockaddr_in*>(ifa->ifa_netmask);
            if (nm) {
                char buf[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &nm->sin_addr, buf, sizeof(buf));
                iface.netmaskString = buf;
                iface.netmask = ntohl(nm->sin_addr.s_addr);

                iface.subnet = iface.ip & iface.netmask;
            }

#ifdef __linux__
            if (ifa->ifa_ifu.ifu_broadaddr) {
                sockaddr_in* bc = reinterpret_cast<sockaddr_in*>(ifa->ifa_ifu.ifu_broadaddr);
                char buf[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &bc->sin_addr, buf, sizeof(buf));
                iface.broadcastString = buf;
                iface.broadcast = ntohl(bc->sin_addr.s_addr);
            }
#endif
        }
    }

    for (auto& pair : ifaceMap) {
        NetworkInterface& iface = pair.second;

        iface.type = NetworkInterface::detectType(iface.name);

        switch (iface.type) {
        case NetworkInterface::IFACE_TYPE_BRIDGE:
            std::cout << "Device " << iface.name << " is a bridge" << std::endl;
            break;
        case NetworkInterface::IFACE_TYPE_PHYS:
            std::cout << "Device " << iface.name << " is a physical device" << std::endl;
            break;
        case NetworkInterface::IFACE_TYPE_TAP:
            std::cout << "Device " << iface.name << " is a tun/tap device" << std::endl;
            break;
        case NetworkInterface::IFACE_TYPE_UNKNOWN:
            std::cout << "Device " << iface.name << " has an unknown iface type" << std::endl;
            break;
        }

        interfaces.push_back(iface);
    }

    close(sock);
    freeifaddrs(ifaddr);
}

NetworkInterface::Type NetworkInterface::detectType(const std::string& ifaceName) {
#ifdef __linux__
    namespace fs = std::filesystem;

    if (fs::exists("/sys/class/net/" + ifaceName + "/bridge"))
        return IFACE_TYPE_BRIDGE;

    if (fs::exists("/sys/class/net/" + ifaceName + "/tun_flags"))
        return IFACE_TYPE_TAP;

    if (fs::exists("/sys/class/net/" + ifaceName + "/device"))
        return IFACE_TYPE_PHYS;

    return IFACE_TYPE_UNKNOWN;

#elif defined(__sun__) || defined(__sun) || \
      defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    struct ifaddrs* ifaddr;
    if (getifaddrs(&ifaddr) == -1)
        return IFACE_TYPE_UNKNOWN;

    Type result = IFACE_TYPE_UNKNOWN;
    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_name || ifaceName != ifa->ifa_name)
            continue;

        // Loopback interfaces
        if (ifa->ifa_flags & IFF_LOOPBACK) {
            result = IFACE_TYPE_PHYS; // treat loopback as physical-ish
            break;
        }

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_LINK) {
            struct sockaddr_dl* sdl = (struct sockaddr_dl*)ifa->ifa_addr;
            if (sdl->sdl_type == IFT_ETHER)
                result = IFACE_TYPE_PHYS;
            else if (sdl->sdl_type == IFT_BRIDGE)
                result = IFACE_TYPE_BRIDGE;
            else if (sdl->sdl_type == IFT_TUN || sdl->sdl_type == IFT_SLIP)
                result = IFACE_TYPE_TAP;
            break;
        }
#elif defined(__sun__) || defined(__sun)
        // On Solaris/SunOS we don't have AF_LINK; rely on flags
        if (ifa->ifa_flags & IFF_POINTOPOINT)
            result = IFACE_TYPE_TAP;
        else if (ifa->ifa_flags & IFF_BROADCAST)
            result = IFACE_TYPE_PHYS;
        else
            result = IFACE_TYPE_UNKNOWN;
        break;
#endif
    }

    freeifaddrs(ifaddr);
    return result;

#else
    return IFACE_TYPE_UNKNOWN;
#endif
}
