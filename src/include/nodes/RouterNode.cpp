#include "RouterNode.hpp"

#include <fstream>
#include <iostream>
#include <qgraphicsitem.h>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

RouterNode::RouterNode(const QString &name, QGraphicsItem *parent) : Node(name, parent) {
    m_routerPrivateIP = getLocalRouterIP();
    m_routerPublicIP = getPublicRouterIP();

    std::cout << "Public IP: " << m_routerPublicIP << std::endl;
    std::cout << "Private IP: " << m_routerPrivateIP << std::endl;

    m_privateIPLabel = new QGraphicsTextItem(QString::fromStdString("PrivIP: " + m_routerPrivateIP), this);
    m_privateIPLabel->setDefaultTextColor(Qt::blue);
    m_privateIPLabel->setPos(5, rect().height() - 25);

    m_publicIPLabel = new QGraphicsTextItem(QString::fromStdString("PubIP: " + m_routerPublicIP), this);
    m_publicIPLabel->setDefaultTextColor(Qt::blue);
    m_publicIPLabel->setPos(5, rect().height() - 25 -  m_privateIPLabel->boundingRect().height());

    m_outputPortGateway = addPort("Gateway IP", Port::Output, Port::Type_IP);
    m_outputPortSubnet = addPort("Subnet", Port::Output, Port::Type_SUBNET);
}

std::string RouterNode::getLocalRouterIP() {
    std::ifstream route("/proc/net/route");
    std::string line;

    std::getline(route, line);

    while (std::getline(route, line)) {
        char iface[16], dest[9], gateway[9];
        sscanf(line.c_str(), "%s %s %s", iface, dest, gateway);

        if (strcmp(dest, "00000000") == 0) {
            unsigned int gw;
            sscanf(gateway, "%X", &gw);
            
            struct in_addr addr;
            addr.s_addr = gw;
            return inet_ntoa(addr);
        }
    }

    return "No gateway found";
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