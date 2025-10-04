#include <QApplication>
#include "include/Canvas.hpp"
#include "include/net/Interfaces.hpp"
#include "include/nodes/RouterNode.hpp"
#include "include/nodes/SubnetNode.hpp"
#include "include/nodes/InterfaceNode.hpp"
#include <algorithm>
#include <set>
#include <vector>
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Canvas canvas;
    canvas.setWindowTitle("qtngraph");
    canvas.resize(1200, 800);
    canvas.show();

    NetworkRegistry registry;
    registry.scanForInterfaces();

    std::set<std::string> addedRouters;
    std::set<std::string> addedInterfaces;

    std::vector<QString> subnets;
    std::vector<SubnetNode*> subnetNodes;
    std::vector<RouterNode*> routerNodes;

    auto gateways = RouterNode::getLocalRouterIPs();
    int routerIndex = 0;

    for (auto &[gatewayIP, subnet, netmask] : gateways) {
        if (addedRouters.count(gatewayIP))
            continue;

        addedRouters.insert(gatewayIP);
        QString nodeName = QString("Router %1").arg(routerIndex++);
        RouterNode* routerNode = new RouterNode(nodeName, gatewayIP, subnet, netmask);
        routerNode->setPos(200 + routerIndex * 200, 400);
        canvas.scene()->addItem(routerNode);
        routerNodes.push_back(routerNode);

        std::string cidr = routerNode->subnetCIDR();
        SubnetNode* subnetNode;

        auto it = std::find(subnets.begin(), subnets.end(), QString::fromStdString(cidr));
        if (it == subnets.end()) {
            subnetNode = new SubnetNode(QString::fromStdString(cidr));
            subnetNode->setPos(100 + routerIndex * 200, 600);
            canvas.scene()->addItem(subnetNode);
            subnetNodes.push_back(subnetNode);
            subnets.push_back(QString::fromStdString(cidr));
        } else {
            size_t index = std::distance(subnets.begin(), it);
            subnetNode = subnetNodes[index];
        }

        canvas.connectPorts(subnetNode->subnetOutputPort(), routerNode->inputPortSubnet());
    }

    for (auto& iface : registry.interfaces) {
        if (addedInterfaces.count(iface.name))
            continue;

        addedInterfaces.insert(iface.name);
        QPointF pos(100 + canvas.nodes().size() * 200, 100);

        InterfaceNode* ifaceNode = new InterfaceNode(&iface, canvas, subnets, subnetNodes, routerNodes, pos);
            canvas.scene()->addItem(ifaceNode);
    }

    return app.exec();
}
