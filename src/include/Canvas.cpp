#include "Canvas.hpp"
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>
#include <QWheelEvent>
#include <QScrollBar>

NetworkPort::NetworkPort(const QString &name, PortDirection dir, PortType type, QGraphicsItem *parent, int maxConnections)
    : QGraphicsRectItem(parent), m_dir(dir), m_type(type), m_name(name), m_maxConnections(maxConnections)
{
    QString typeLabel;
    switch(type) {
        case Type_IP: typeLabel = "IP"; break;
        case Type_MAC: typeLabel = "MAC"; break;
        default: typeLabel = "GEN"; break;
    }

    QString fullLabel = QString("%1 [%2]").arg(name).arg(typeLabel);
    m_label = new QGraphicsTextItem(fullLabel, this);
    m_label->setDefaultTextColor(Qt::black);

    QRectF textRect = m_label->boundingRect();
    qreal padding = 6;
    QRectF rect(0, 0, textRect.width() + padding*2, textRect.height());

    setRect(rect);
    setBrush((dir == Input) ? QColor(255,200,200) : QColor(200,255,200));
    setPen(QPen(Qt::black));
    setFlag(ItemSendsScenePositionChanges);
    setAcceptedMouseButtons(Qt::LeftButton);
    setZValue(1);

    m_label->setPos(padding, (rect.height() - textRect.height())/2 - 2);
}


void NetworkPort::addConnection(Connection *conn) {
    if (m_maxConnections >= 0 && m_connections.size() >= m_maxConnections)
     return;
    m_connections.append(conn);
}

void NetworkPort::removeConnection(Connection *conn) { m_connections.removeAll(conn); }

QVariant NetworkPort::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemScenePositionHasChanged) {
        for(auto *conn : m_connections)
            conn->updatePath();
    }
    return QGraphicsRectItem::itemChange(change, value);
}

void NetworkPort::mousePressEvent(QGraphicsSceneMouseEvent *event) { event->accept(); }

Connection::Connection(NetworkPort *from, NetworkPort *to)
    : m_from(from), m_to(to)
{
    if (to->maxConnections() >= 0 && to->connections().size() >= to->maxConnections())
        return;

    setPen(QPen(Qt::darkBlue, 2));
    if(from) from->addConnection(this);
    if(to) to->addConnection(this);
    updatePath();

    if(from) from->triggerConnected(to);
    if(to) to->triggerConnected(from);
}

Connection::~Connection()
{
    if(m_from) m_from->removeConnection(this);
    if(m_to) m_to->removeConnection(this);
}

void Connection::updatePath()
{
    if(!m_from || !m_to) return;

    QPointF p1 = m_from->scenePos();
    QPointF p2 = m_to->scenePos();

    if(m_from->direction() == NetworkPort::Output)
        p1 += QPointF(m_from->rect().width(), m_from->rect().height()/2);
    else
        p1 += QPointF(0, m_from->rect().height()/2);

    if(m_to->direction() == NetworkPort::Input)
        p2 += QPointF(0, m_to->rect().height()/2);
    else
        p2 += QPointF(m_to->rect().width(), m_to->rect().height()/2);

    QPainterPath path(p1);
    QPointF ctrl1 = p1 + QPointF(50,0);
    QPointF ctrl2 = p2 - QPointF(50,0);
    path.cubicTo(ctrl1, ctrl2, p2);
    setPath(path);
}

NetworkNode::NetworkNode(const QString &name, QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
    setRect(0,0,200,140);
    setBrush(Qt::lightGray);
    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);

    m_label = new QGraphicsTextItem(name, this);
    m_label->setPos(5,5);
}

NetworkPort* NetworkNode::addPort(const QString &name, NetworkPort::PortDirection dir, NetworkPort::PortType type, int maxConnections)
{
    NetworkPort *port = new NetworkPort(name, dir, type, this, maxConnections);

    if(dir == NetworkPort::Input) {
        port->setPos(rect().left() - port->rect().width() + port->boundingRect().width() - 10,
                     m_label->boundingRect().height() + 5 + m_inputPorts.size()*28);
        m_inputPorts.append(port);
    } else {
        port->setPos(rect().right() - port->boundingRect().width() + 10,
                     rect().top() + m_label->boundingRect().height() + 5 + m_outputPorts.size()*28);
        m_outputPorts.append(port);
    }
    return port;
}

Canvas::Canvas(QWidget *parent)
    : QGraphicsView(parent)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
}

NetworkNode* Canvas::addNode(const QString &name, const QPointF &pos)
{
    NetworkNode *node = new NetworkNode(name);
    node->setPos(pos);
    m_scene->addItem(node);
    m_nodes.append(node);
    return node;
}

Connection* Canvas::connectPorts(NetworkPort *from, NetworkPort *to)
{
    if(!from || !to) return nullptr;
    if(from->direction() == to->direction()) return nullptr;
    if(from->portType() != to->portType()) return nullptr;

    Connection *conn = new Connection(
        (from->direction() == NetworkPort::Output) ? from : to,
        (from->direction() == NetworkPort::Output) ? to : from
    );
    m_scene->addItem(conn);
    m_connections.append(conn);
    return conn;
}

NetworkPort* Canvas::findPortAt(const QPointF &scenePos)
{
    QList<QGraphicsItem*> itemsAt = m_scene->items(QRectF(scenePos-QPointF(5,5), QSizeF(10,10)));
    for(auto *item : itemsAt)
        if(auto *port = dynamic_cast<NetworkPort*>(item))
            return port;
    return nullptr;
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::MiddleButton) {
        m_panning = true;
        m_lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    QPointF scenePos = mapToScene(event->pos());
    if(auto *port = findPortAt(scenePos)) {
        m_draggingPort = port;
        m_tempPath = new QGraphicsPathItem();
        m_tempPath->setPen(QPen(Qt::DashLine));
        m_scene->addItem(m_tempPath);
    }
    QGraphicsView::mousePressEvent(event);
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    if(m_panning) {
        QPointF delta = mapToScene(m_lastPanPoint) - mapToScene(event->pos());
        translate(delta.x(), delta.y());
        m_lastPanPoint = event->pos();
        event->accept();
        return;
    }

    if(m_draggingPort && m_tempPath) {
        QPointF p1 = m_draggingPort->scenePos() + QPointF(m_draggingPort->rect().width()/2, m_draggingPort->rect().height()/2);
        QPointF p2 = mapToScene(event->pos());
        QPainterPath path(p1);
        QPointF ctrl1 = p1 + QPointF(50,0);
        QPointF ctrl2 = p2 - QPointF(50,0);
        path.cubicTo(ctrl1, ctrl2, p2);
        m_tempPath->setPath(path);
    }
    QGraphicsView::mouseMoveEvent(event);
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::MiddleButton && m_panning) {
        m_panning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }

    if(m_draggingPort && m_tempPath) {
        QPointF scenePos = mapToScene(event->pos());
        NetworkPort *targetPort = findPortAt(scenePos);
        if(targetPort) connectPorts(m_draggingPort, targetPort);

        m_scene->removeItem(m_tempPath);
        delete m_tempPath;
        m_tempPath = nullptr;
        m_draggingPort = nullptr;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void Canvas::wheelEvent(QWheelEvent *event)
{
    const double scaleFactor = 1.15;
    if(event->angleDelta().y() > 0) scale(scaleFactor, scaleFactor);
    else scale(1.0/scaleFactor,1.0/scaleFactor);
}

QList<NetworkNode*> Canvas::nodes() { return m_nodes; }
