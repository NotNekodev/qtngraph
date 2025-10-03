#pragma once

#include "../Canvas.hpp"

extern int subnet_name_counter; // so like subn0 subn1 ... subnn

class SubnetNode : public Node {
public:
    SubnetNode(const QString &cidr, QGraphicsItem *parent = nullptr);

    Port *subnetOutputPort() const { return m_subnetOutputPort; };

private:
    Port *m_subnetOutputPort;
};