#include "InterfaceNode.hpp"
#include <algorithm>
#include <iostream>
#include <qcolor.h>
#include <qcontainerfwd.h>
#include <qnamespace.h>

InterfaceNode::InterfaceNode(NetworkInterface* iface,
                             Canvas& canvas,
                             std::vector<QString>& subnets,
                             std::vector<SubnetNode*>& subnetNodes,
                             const std::vector<RouterNode*>& routerNodes,
                             QPointF pos)
    : Node(QString::fromStdString(iface->name))
{
    setPos(pos);

    m_subnetInPort  = addPort("Subnet", Port::Input, Port::Type_SUBNET, 1);
    m_gatewayInPort = addPort("Gateway IP", Port::Input, Port::Type_IP);

    SubnetNode* node_subn;
    QString subnetCIDR = QString::fromStdString(iface->subnetCIDR());
    auto it = std::find(subnets.begin(), subnets.end(), subnetCIDR);
    if (it == subnets.end()) {
        node_subn = new SubnetNode(subnetCIDR);
        node_subn->setPos(pos.x(), pos.y() + 500);
        canvas.scene()->addItem(node_subn);
        subnetNodes.push_back(node_subn);
        subnets.push_back(subnetCIDR);
        std::cout << "Adding node for subnet: " << subnetCIDR.toStdString() << std::endl;
    } else {
        size_t index = std::distance(subnets.begin(), it);
        node_subn = subnetNodes[index];
    }

    canvas.connectPorts(node_subn->subnetOutputPort(), m_subnetInPort);

    for (RouterNode* routerNode : routerNodes) {
        if (subnetCIDR == routerNode->subnetCIDR()) {
            canvas.connectPorts(routerNode->outputPortGateway(), m_gatewayInPort);
            break;
        }
    }

    QColor statusColor = Qt::red;
    std::string statusText = "DOWN";
    if (iface->isUp) {
        statusColor = Qt::green;
        statusText = "UP";
    }

    /*if (iface->isRunning) {
        statusColor = QColor(255, 0, 255);
        statusText = "RUNNING";
    }*/

    addChildText(QString::fromStdString(statusText), statusColor, 0.7);

    addChildText(QString::fromStdString("IP: " + iface->ipString()), Qt::blue);
    addChildText(QString::fromStdString("MAC: " + iface->mac.toString()), Qt::blue);

    adjustSize();
}
