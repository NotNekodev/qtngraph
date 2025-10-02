#pragma once

#include "../Canvas.hpp"
#include <qcontainerfwd.h>
#include <qgraphicsitem.h>

class RouterNode : public Node {
public:
    RouterNode(const QString &name, QGraphicsItem *parent = nullptr);

    std::string getLocalRouterIP();
    std::string getPublicRouterIP();

private:
    std::string m_routerPrivateIP;
    std::string m_routerPublicIP;

    QGraphicsTextItem *m_publicIPLabel;
    QGraphicsTextItem *m_privateIPLabel;

    Port *m_outputPortSubnet;
    Port *m_outputPortGateway;
};