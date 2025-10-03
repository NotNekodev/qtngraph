#pragma once

#include "../Canvas.hpp"
#include <cstdint>
#include <qcontainerfwd.h>
#include <qgraphicsitem.h>
#include <vector>
#include <tuple>
#include <string>

class RouterNode : public Node {
public:
    RouterNode(const QString &name,
               const std::string &gatewayIP,
               uint32_t subnet,
               uint32_t netmask,
               QGraphicsItem *parent = nullptr);

    static std::vector<std::tuple<std::string, uint32_t, uint32_t>> getLocalRouterIPs();

    std::string getPublicRouterIP();
    std::string getISPHostname();

    std::string subnetCIDR();

    Port *inputPortSubnet() const { return m_inputPortSubnet; };
    Port *outputPortGateway() const { return m_outputPortGateway; };

private:
    std::string m_routerPrivateIP;
    std::string m_routerPublicIP;
    std::string m_ispHostname;

    QGraphicsTextItem *m_publicIPLabel;
    QGraphicsTextItem *m_privateIPLabel;
    QGraphicsTextItem *m_ispHostnameLabel;

    Port *m_inputPortSubnet;
    Port *m_outputPortGateway;

    uint32_t netmask;
    uint32_t subnet;
};
