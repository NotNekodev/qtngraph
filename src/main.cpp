#include <QApplication>
#include "include/Canvas.hpp"
#include "include/net/Interfaces.hpp"
#include "include/nodes/IPDisplayNode.hpp"
#include "include/nodes/RouterNode.hpp"
#include "include/nodes/SubnetNode.hpp"
#include <algorithm>
#include <qcontainerfwd.h>
#include <set>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    Canvas canvas;
    canvas.setWindowTitle(":3");
    canvas.resize(1200, 800);
    canvas.show();

    NetworkRegistry registry;
    registry.scanForInterfaces();

    std::set<std::string> addedInterfaces;

    RouterNode *routerNode = new RouterNode("Router");
    routerNode->setPos(200, 400);
    canvas.scene()->addItem(routerNode);

    std::vector<std::string> subnets;

    std::vector<SubnetNode*> subnet_nodes; 

    for (auto& iface : registry.interfaces) {
        if (addedInterfaces.find(iface.name) != addedInterfaces.end())
            continue;

        addedInterfaces.insert(iface.name);

        QString nodeName = QString::fromStdString(iface.name);
        Node* node = canvas.addNode(nodeName, QPointF(100 + canvas.nodes().size() * 200, 100));

        Port* ipPort = node->addPort("IP", Port::Output, Port::Type_IP);
        node->addPort("MAC", Port::Output, Port::Type_MAC);
        ipPort->setData<QString>(QString::fromStdString(iface.ipString()));

        Port* subnetInPort = node->addPort("Subnet", Port::Input, Port::Type_SUBNET);
    
        std::cout << nodeName.toStdString() << ": "
                << ipPort->getData<QString>().toStdString() << std::endl;
    
        SubnetNode *node_subn;
        auto it = std::find(subnets.begin(), subnets.end(), iface.subnetCIDR());
        if (it == subnets.end()) {
            node_subn = new SubnetNode(QString::fromStdString(iface.subnetCIDR()));
            node_subn->setPos(100, 600);
            canvas.scene()->addItem(node_subn);
            subnet_nodes.push_back(node_subn);
            subnets.push_back(iface.subnetCIDR());
            std::cout << "Adding node for subnet: " << iface.subnetCIDR() << std::endl;
        } else {
            size_t index = std::distance(subnets.begin(), it);
            node_subn = subnet_nodes[index];
        }
    
        Connection *conn = canvas.connectPorts(node_subn->subnetOutputPort(), subnetInPort);
    }

    SubnetNode *node_subn;
    auto it = std::find(subnets.begin(), subnets.end(), routerNode->subnetCIDR());
    if (it == subnets.end()) {
        node_subn = new SubnetNode(QString::fromStdString(routerNode->subnetCIDR()));
        node_subn->setPos(100, 600);
        canvas.scene()->addItem(node_subn);
        subnet_nodes.push_back(node_subn);
        subnets.push_back(routerNode->subnetCIDR());
        std::cout << "Adding node for subnet: " << routerNode->subnetCIDR() << std::endl;
    } else {
        size_t index = std::distance(subnets.begin(), it);
        node_subn = subnet_nodes[index];
    }
    
    Connection *conn = canvas.connectPorts(node_subn->subnetOutputPort(), routerNode->inputPortSubnet());

    return a.exec();
}
