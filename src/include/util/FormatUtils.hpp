#pragma once

#include <string>
#include <cstdint>

class FormatUtils {
public:
    inline static std::string ipFormat(uint32_t ip) {
        return std::to_string((ip >> 24) & 0xFF) + "." +
               std::to_string((ip >> 16) & 0xFF) + "." +
               std::to_string((ip >> 8) & 0xFF) + "." +
               std::to_string(ip & 0xFF);
    }

    inline static std::string netmaskFormat(uint32_t netmask) {
        return ipFormat(netmask);
    }
};