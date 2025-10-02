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
    m_ispHostname = getISPHostname();
    
    std::cout << "Public IP: " << m_routerPublicIP << std::endl;
    std::cout << "Private IP: " << m_routerPrivateIP << std::endl;
    std::cout << "ISP Hostname: " << m_ispHostname << std::endl;

    m_privateIPLabel = new QGraphicsTextItem(QString::fromStdString("Private IP: " + m_routerPrivateIP), this);
    m_privateIPLabel->setDefaultTextColor(Qt::blue);
    
    m_publicIPLabel = new QGraphicsTextItem(QString::fromStdString("Public IP: " + m_routerPublicIP), this);
    m_publicIPLabel->setDefaultTextColor(Qt::blue);
    
    m_outputPortGateway = addPort("Gateway IP", Port::Output, Port::Type_IP);
    m_outputPortSubnet = addPort("Subnet", Port::Output, Port::Type_SUBNET);

    qreal maxWidth = std::max({
        m_privateIPLabel->boundingRect().width(),
        m_publicIPLabel->boundingRect().width(),
        
    });

    qreal outputPortsMaxWidth = std::max({
        m_outputPortGateway->boundingRect().width() + 10,
        m_outputPortSubnet->boundingRect().width() + 10,
    });
    
    qreal totalHeight = m_privateIPLabel->boundingRect().height() +
                        m_publicIPLabel->boundingRect().height();
    
    qreal padding = 10;
    qreal topMargin = 30;
    qreal bottomMargin = 30;
    
    QRectF newRect(0, 0, maxWidth + outputPortsMaxWidth + padding * 2, totalHeight + topMargin + bottomMargin);
    setRect(newRect);
    
    m_privateIPLabel->setPos(padding, rect().height() - bottomMargin - m_privateIPLabel->boundingRect().height());
    m_publicIPLabel->setPos(padding, m_privateIPLabel->pos().y() - m_publicIPLabel->boundingRect().height());
    
    m_outputPortGateway->setPos(rect().right() - m_outputPortGateway->boundingRect().width() + 10,
                     rect().top() + label()->boundingRect().height() + 5 + 0);

    m_outputPortSubnet->setPos(rect().right() - m_outputPortSubnet->boundingRect().width() + 10,
                     rect().top() + label()->boundingRect().height() + 5 + 28);

    m_outputPortGateway->setData<QString>(QString::fromStdString(m_routerPrivateIP));
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

std::string RouterNode::getISPHostname() {
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    inet_pton(AF_INET, getPublicRouterIP().c_str(), &sa.sin_addr);
    
    char host[1024];
    int result = getnameinfo((struct sockaddr*)&sa, sizeof(sa),
                            host, sizeof(host), NULL, 0, 0);
    
    if (result == 0) {
        return host;
    }
    return "Unknown";
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