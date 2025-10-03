#pragma once

#include "../Canvas.hpp"
#include <qgraphicsitem.h>

extern int subnet_name_counter; // so like subn0 subn1 ... subnn

class SubnetNode : public Node {
public:
    SubnetNode(const QString &cidr, QGraphicsItem *parent = nullptr);

    Port *subnetOutputPort() const { return m_subnetOutputPort; };

private:
    Port *m_subnetOutputPort;

    uint32_t m_netmask;
    uint32_t m_subnet;

    QGraphicsTextItem *m_labelNetmask;
    QGraphicsTextItem *m_labelSubnetAddress;
};