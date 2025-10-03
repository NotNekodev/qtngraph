#pragma once

#include "../Canvas.hpp"
#include "SubnetNode.hpp"
#include "RouterNode.hpp"
#include "../net/Interfaces.hpp"
#include <vector>

class InterfaceNode : public Node {
public:
    InterfaceNode(NetworkInterface* iface,
                  Canvas& canvas,
                  std::vector<QString>& subnets,
                  std::vector<SubnetNode*>& subnetNodes,
                  const std::vector<RouterNode*>& routerNodes,
                  QPointF pos);
    Port* subnetInPort() const { return m_subnetInPort; }
    Port* gatewayInPort() const { return m_gatewayInPort; }
private:
    Port* m_subnetInPort;
    Port* m_gatewayInPort;
};
