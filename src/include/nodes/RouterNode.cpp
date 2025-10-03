#include "RouterNode.hpp"

#include <fstream>
#include <qcontainerfwd.h>
#include <qgraphicsitem.h>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

RouterNode::RouterNode(const QString &name,
                       const std::string &gatewayIP,
                       uint32_t subnet,
                       uint32_t netmask,
                       QGraphicsItem *parent) : Node(name, parent),
                                                m_routerPrivateIP(gatewayIP),
                                                subnet(subnet),
                                                netmask(netmask) {
    m_routerPublicIP = getPublicRouterIP();
    m_ispHostname = getISPHostname();

    m_outputPortGateway = addPort("Gateway IP", Port::Output, Port::Type_IP);
    m_inputPortSubnet = addPort("Subnet", Port::Input, Port::Type_SUBNET, 1);

    addChildText(QString::fromStdString("Private IP: " + m_routerPrivateIP));
    addChildText(QString::fromStdString("Public IP: " + m_routerPublicIP));
    addChildText(QString::fromStdString("ISP Hostname: " + m_ispHostname));

    m_outputPortGateway->setData<QString>(QString::fromStdString(m_routerPrivateIP));
}

std::vector<std::tuple<std::string, uint32_t, uint32_t>> RouterNode::getLocalRouterIPs() {
    std::ifstream route("/proc/net/route");
    std::string line;
    std::getline(route, line);
    std::vector<std::tuple<std::string, uint32_t, uint32_t>> routers;

    while (std::getline(route, line)) {
        char iface[16], dest[9], gateway[9], flags[5], refcnt[5], use[5], metric[5], mask[9];
        sscanf(line.c_str(), "%s %s %s %s %s %s %s %s", iface, dest, gateway, flags, refcnt, use, metric, mask);

        if (strcmp(dest, "00000000") == 0) {
            unsigned int gw;
            sscanf(gateway, "%X", &gw);
            struct in_addr addr;
            addr.s_addr = gw;

            std::ifstream route2("/proc/net/route");
            std::string line2;
            std::getline(route2, line2);
            uint32_t subnet = 0, netmask = 0;

            while (std::getline(route2, line2)) {
                char iface2[16], dest2[9], gw2[9], flags2[5], refcnt2[5], use2[5], metric2[5], mask2[9];
                sscanf(line2.c_str(), "%s %s %s %s %s %s %s %s", iface2, dest2, gw2, flags2, refcnt2, use2, metric2, mask2);
                if (strcmp(iface, iface2) == 0 && strcmp(gw2, "00000000") == 0 && strcmp(mask2, "00000000") != 0) {
                    sscanf(dest2, "%X", &subnet);
                    sscanf(mask2, "%X", &netmask);
                    break;
                }
            }

            routers.emplace_back(std::string(inet_ntoa(addr)), subnet, netmask);
        }
    }

    return routers;
}

std::string RouterNode::subnetCIDR() {
    uint32_t mask = netmask;
    int count = 0;
    while (mask) {
        count += mask & 1;
        mask >>= 1;
    }

    struct in_addr subnet_addr;
    subnet_addr.s_addr = this->subnet;
    std::string subnet_str = inet_ntoa(subnet_addr);

    return subnet_str + "/" + std::to_string(count);
}

std::string RouterNode::getPublicRouterIP() {
    struct hostent* host = gethostbyname("api.ipify.org");
    if (!host) return "Error resolving host";

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return "Error creating socket";

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(80);
    memcpy(&server.sin_addr, host->h_addr_list[0], host->h_length);

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        close(sock);
        return "Error connecting";
    }

    const char* request = "GET / HTTP/1.1\r\nHost: api.ipify.org\r\nConnection: close\r\n\r\n";
    send(sock, request, strlen(request), 0);

    char buffer[1024] = {0};
    std::string response;
    int bytes;

    while ((bytes = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes] = '\0';
        response += buffer;
    }

    close(sock);

    size_t pos = response.find("\r\n\r\n");
    if (pos != std::string::npos) {
        return response.substr(pos + 4);
    }

    return "Error parsing response";
}

std::string RouterNode::getISPHostname() {
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    inet_pton(AF_INET, m_routerPublicIP.c_str(), &sa.sin_addr);

    char host[1024];
    int result = getnameinfo((struct sockaddr*)&sa, sizeof(sa),
                             host, sizeof(host), NULL, 0, 0);

    if (result == 0) {
        return host;
    }
    return "Unknown";
}
