#include "SubnetNode.hpp"
#include <qcontainerfwd.h>

int subnet_name_counter = 0;

SubnetNode::SubnetNode(const QString &cidr, QGraphicsItem *parent) : Node(QString("subn%1").arg(subnet_name_counter), parent) {
    subnet_name_counter++;

    m_subnetOutputPort = addPort("Subnet", Port::Output, Port::Type_SUBNET);

    m_subnetOutputPort->setData<QString>(cidr);
}