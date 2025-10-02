#include "../Canvas.hpp"
#include <qcontainerfwd.h>
#include <qgraphicsitem.h>

class IPDisplayNode : public NetworkNode {
public:
    IPDisplayNode(const QString& name, QGraphicsItem *parent = nullptr);

    void setIP(const QString &ip);
    QString ip() const;

private:
    QGraphicsTextItem *m_ipLabel;
    NetworkPort *m_inputPort;
};