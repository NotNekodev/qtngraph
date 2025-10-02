#include <QApplication>
#include "include/Canvas.hpp"
#include "include/net/Interfaces.hpp"
#include "include/nodes/IPDisplayNode.hpp"
#include "include/nodes/RouterNode.hpp"
#include <set>
#include <iostream>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    Canvas canvas;
    canvas.setWindowTitle(":3");
    canvas.resize(1200, 800);
    canvas.show();

    NetworkRegistry registry;
    registry.scanForInterfaces();

    std::set<std::string> addedInterfaces;

    for (auto& iface : registry.interfaces) {
        if (addedInterfaces.find(iface.name) != addedInterfaces.end())
            continue;

        addedInterfaces.insert(iface.name);

        QString nodeName = QString::fromStdString(iface.name);
        Node* node = canvas.addNode(nodeName, QPointF(100 + canvas.nodes().size() * 200, 100));

        Port* ipPort = node->addPort("IP", Port::Output, Port::Type_IP);
        node->addPort("MAC", Port::Output, Port::Type_MAC);

        ipPort->setData<QString>(QString::fromStdString(iface.ipString()));

        std::cout << nodeName.toStdString() << ": " 
                  << ipPort->getData<QString>().toStdString() << std::endl;
    }

    IPDisplayNode* displayNode = new IPDisplayNode("IP Display");
    displayNode->setPos(100, 400);
    canvas.scene()->addItem(displayNode);

    RouterNode *routerNode = new RouterNode("Router");
    routerNode->setPos(200, 400);
    canvas.scene()->addItem(routerNode);

    return a.exec();
}
