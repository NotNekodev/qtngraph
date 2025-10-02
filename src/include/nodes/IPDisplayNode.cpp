#include "IPDisplayNode.hpp"

IPDisplayNode::IPDisplayNode(const QString &name, QGraphicsItem *parent)
    : NetworkNode(name, parent)
{
    m_inputPort = addPort("IP In", NetworkPort::Input, NetworkPort::Type_IP, 1);

    m_ipLabel = new QGraphicsTextItem("IP: N/A", this);
    m_ipLabel->setDefaultTextColor(Qt::blue);
    m_ipLabel->setPos(5, rect().height() - 25);

    m_inputPort->setOnConnected([this](NetworkPort* other){
        if(other->privateData.has_value()) {
            try {
                QString ip = std::any_cast<QString>(other->privateData);
                setIP(ip);
            } catch(...) {}
        }
    });
}

void IPDisplayNode::setIP(const QString &ip)
{
    m_ipLabel->setPlainText("IP: " + ip);
}

QString IPDisplayNode::ip() const
{
    return m_ipLabel->toPlainText();
}
