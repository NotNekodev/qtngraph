#include "Canvas.hpp"
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>
#include <QWheelEvent>
#include <QScrollBar>

Port::Port(const QString &name, PortDirection dir, PortType type, QGraphicsItem *parent, int maxConnections)
    : QGraphicsRectItem(parent), m_dir(dir), m_type(type), m_name(name), m_maxConnections(maxConnections)
{
    QString fullLabel = QString("%1").arg(name);
    m_label = new QGraphicsTextItem(fullLabel, this);
    m_label->setDefaultTextColor(Qt::black);

    QRectF textRect = m_label->boundingRect();
    qreal padding = 6;
    QRectF rect(0, 0, textRect.width() + padding*2, textRect.height());

    QColor color;
    switch (type) {
    case Port::Type_Generic: color = Qt::red; break;
    case Port::Type_IP: color = Qt::green; break;
    case Port::Type_MAC: color = Qt::blue; break;
    case Port::Type_SUBNET: color = QColor(255, 0, 255); break;
    }

    m_color = color;
    m_label->setDefaultTextColor(color.darker(150));

    setRect(rect);
    setBrush(QColor(color.red(), color.green(), color.blue(), 128));
    setPen(QPen(color));
    setFlag(ItemSendsScenePositionChanges);
    setAcceptedMouseButtons(Qt::LeftButton);
    setZValue(1);
    m_label->setPos(padding, (rect.height() - textRect.height())/2 - 2);
}

void Port::addConnection(Connection *conn) {
    if (m_maxConnections >= 0 && m_connections.size() >= m_maxConnections) return;
    m_connections.append(conn);
}

void Port::removeConnection(Connection *conn) { m_connections.removeAll(conn); }

QVariant Port::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemScenePositionHasChanged) {
        for(auto *conn : m_connections)
            conn->updatePath();
    }
    return QGraphicsRectItem::itemChange(change, value);
}

void Port::mousePressEvent(QGraphicsSceneMouseEvent *event) { event->accept(); }

Connection::Connection(Port *from, Port *to) : m_from(from), m_to(to)
{
    if (to->maxConnections() >= 0 && to->connections().size() >= to->maxConnections())
        return;
    setPen(QPen(from->color(), 2));
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
    if(m_from->direction() == Port::Output)
        p1 += QPointF(m_from->rect().width(), m_from->rect().height()/2);
    else
        p1 += QPointF(0, m_from->rect().height()/2);
    if(m_to->direction() == Port::Input)
        p2 += QPointF(0, m_to->rect().height()/2);
    else
        p2 += QPointF(m_to->rect().width(), m_to->rect().height()/2);
    QPainterPath path(p1);
    QPointF ctrl1 = p1 + QPointF(50,0);
    QPointF ctrl2 = p2 - QPointF(50,0);
    path.cubicTo(ctrl1, ctrl2, p2);
    setPath(path);
}

Node::Node(const QString &name, QGraphicsItem *parent) : QGraphicsRectItem(parent)
{
    setRect(0,0,200,140);
    setBrush(Qt::lightGray);
    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    m_label = new QGraphicsTextItem(name, this);
    m_label->setPos(5,5);
}

Port* Node::addPort(const QString &name, Port::PortDirection dir, Port::PortType type, int maxConnections)
{
    Port *port = new Port(name, dir, type, this, maxConnections);
    const qreal spacing = 8;
    qreal yOffset;
    if (dir == Port::Input) {
        yOffset = m_label->boundingRect().height() + spacing +
                  m_inputPorts.size() * (port->rect().height() + spacing);
        port->setPos(rect().left() - port->rect().width() + 4, yOffset);
        m_inputPorts.append(port);
    } else {
        yOffset = m_label->boundingRect().height() + spacing +
                  m_outputPorts.size() * (port->rect().height() + spacing);
        port->setPos(rect().right() - 4, yOffset);
        m_outputPorts.append(port);
    }
    adjustSize();
    return port;
}

QGraphicsTextItem* Node::addChildText(const QString &text, const QColor &color, const qreal textScaleFactor)
{
    QGraphicsTextItem *child = new QGraphicsTextItem(text, this);
    child->setDefaultTextColor(color);
    child->setScale(textScaleFactor);
    m_children.append(child);
    layoutChildren();
    adjustSize();
    return child;
}

void Node::layoutChildren()
{
    const qreal spacing = 0;
    qreal y = 0;

    m_label->setPos(3, y);
    m_label->setScale(1.1);
    y += m_label->boundingRect().height() + spacing;

    for (QGraphicsTextItem *child : m_children) {
        child->setPos(3, y);
        y += child->boundingRect().height() + spacing;
    }

    qreal yBeforeOutputPorts = y;

    for (Port *p : m_outputPorts) {
        p->setPos(rect().width() - p->rect().width() + 10, y);
        y += p->boundingRect().height() + 1;
        for (Connection *conn : p->connections()) {
            conn->updatePath();
        }
    }

    y = yBeforeOutputPorts;

    for (Port *p : m_inputPorts) {
        p->setPos(-10, y);
        y += p->boundingRect().height() + 1;
        for (Connection *conn : p->connections()) {
            conn->updatePath();
        }
    }
}

void Node::adjustSize()
{
    qreal minWidth = 200;
    qreal minHeight = 100;
    qreal textWidth = m_label->boundingRect().width() + 20;

    for (Port *p : m_inputPorts)
        textWidth = std::max(textWidth, p->boundingRect().width() + 40);
    for (Port *p : m_outputPorts)
        textWidth = std::max(textWidth, p->boundingRect().width() + 40);
    for (QGraphicsTextItem *child : m_children)
        textWidth = std::max(textWidth, child->boundingRect().width() + 20);

    qreal portsHeight = 0;
    if (!m_inputPorts.isEmpty())
        portsHeight = std::max(portsHeight,
            m_inputPorts.last()->pos().y() + m_inputPorts.last()->rect().height() + 10);
    if (!m_outputPorts.isEmpty())
        portsHeight = std::max(portsHeight,
            m_outputPorts.last()->pos().y() + m_outputPorts.last()->rect().height() + 10);

    qreal childrenHeight = 0;
    if (!m_children.isEmpty()) {
        QGraphicsTextItem *last = m_children.last();
        childrenHeight = last->pos().y() + last->boundingRect().height() + 10;
    }

    qreal requiredHeight = std::max({minHeight, portsHeight + 3, childrenHeight + 3});
    QRectF r = rect();
    r.setWidth(std::max(minWidth, textWidth + 20));
    r.setHeight(requiredHeight);
    setRect(r);
    layoutChildren();
}

Canvas::Canvas(QWidget *parent) : QGraphicsView(parent)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
}

Node* Canvas::addNode(const QString &name, const QPointF &pos)
{
    Node *node = new Node(name);
    node->setPos(pos);
    m_scene->addItem(node);
    m_nodes.append(node);
    return node;
}

Connection* Canvas::connectPorts(Port *from, Port *to)
{
    if(!from || !to) return nullptr;
    if(from->direction() == to->direction()) return nullptr;
    if(from->portType() != to->portType()) return nullptr;
    Connection *conn = new Connection(
        (from->direction() == Port::Output) ? from : to,
        (from->direction() == Port::Output) ? to : from
    );
    m_scene->addItem(conn);
    m_connections.append(conn);
    from->connections().push_back(conn);
    to->connections().push_back(conn);
    return conn;
}

Port* Canvas::findPortAt(const QPointF &scenePos)
{
    QList<QGraphicsItem*> itemsAt = m_scene->items(QRectF(scenePos-QPointF(5,5), QSizeF(10,10)));
    for(auto *item : itemsAt)
        if(auto *port = dynamic_cast<Port*>(item))
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
    } else {
        setCursor(Qt::SizeAllCursor);
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
    setCursor(Qt::ArrowCursor);
    if(m_draggingPort && m_tempPath) {
        QPointF scenePos = mapToScene(event->pos());
        Port *targetPort = findPortAt(scenePos);
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

QList<Node*> Canvas::nodes() { return m_nodes; }
