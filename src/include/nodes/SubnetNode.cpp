#include "SubnetNode.hpp"
#include <qcontainerfwd.h>

#include "../util/FormatUtils.hpp"

int subnet_name_counter = 0;

SubnetNode::SubnetNode(const QString &cidr, QGraphicsItem *parent) : Node(QString("subn%1").arg(subnet_name_counter), parent) {
    subnet_name_counter++;

    m_subnetOutputPort = addPort("Subnet", Port::Output, Port::Type_SUBNET);

    m_subnetOutputPort->setData<QString>(cidr);

    QStringList parts = cidr.split('/');

    QString ipStr = parts[0];
    bool ok;
    int prefix = parts[1].toInt(&ok);

    QStringList octets = ipStr.split('.');

    m_subnet = 0;
    for (int i = 0; i < 4; ++i) {
        int octet = octets[i].toInt(&ok);
        m_subnet = (m_subnet << 8) | static_cast<uint32_t>(octet);
    }

    if (prefix == 0) {
        m_netmask = 0;
    } else {
        m_netmask = 0xFFFFFFFF << (32 - prefix);
    }

    m_subnet &= m_netmask;

    m_labelSubnetAddress = new QGraphicsTextItem(QString::fromStdString("Subnet Address: " + FormatUtils::ipFormat(m_subnet)), this);
    m_labelSubnetAddress->setDefaultTextColor(Qt::blue);

    m_labelSubnetAddress->setPos(5, rect().height() - 25);

    m_labelNetmask = new QGraphicsTextItem(QString::fromStdString("Netmask: " + FormatUtils::netmaskFormat(m_netmask)), this);
    m_labelNetmask->setDefaultTextColor(Qt::blue);

    m_labelNetmask->setPos(5, rect().height() - 25 - 28);

    qreal maxWidth = std::max({
        m_labelSubnetAddress->boundingRect().width(),
        m_labelNetmask->boundingRect().width(),
    });

    QRectF newRect(0, 0, maxWidth + 10 * 2, rect().height());
    setRect(newRect);

    m_labelNetmask->setPos(5, rect().height() - 25 - 28);
    m_labelSubnetAddress->setPos(5, rect().height() - 25);

    m_subnetOutputPort->setPos(rect().right() - m_subnetOutputPort->boundingRect().width() + 10,
                     rect().top() + m_subnetOutputPort->boundingRect().height() + 5 + 0);
}