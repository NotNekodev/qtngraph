#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QGraphicsTextItem>
#include <QPen>
#include <QBrush>
#include <QMouseEvent>
#include <QList>
#include <functional>
#include <any>

class Port;
class Connection;
class Node;

class Port : public QGraphicsRectItem {
public:
    enum PortDirection { Input, Output };
    enum PortType { Type_Generic, Type_IP, Type_MAC, Type_SUBNET };

    Port(const QString &name, PortDirection dir, PortType type, QGraphicsItem *parent = nullptr, int maxconnections = -1);

    PortDirection direction() const { return m_dir; }
    PortType portType() const { return m_type; }
    QString name() const { return m_name; }
    int maxConnections() const { return m_maxConnections; };
    QList<Connection *> connections() const { return m_connections; };
    QColor color() const { return m_color; };

    void addConnection(Connection *conn);
    void removeConnection(Connection *conn);

    std::any privateData;
    template<typename T>
    void setData(const T& value) { privateData = value; }
    template<typename T>
    T getData() const { return std::any_cast<T>(privateData); }

    void setOnConnected(std::function<void(Port*)> callback) { m_onConnected = callback; }
    void triggerConnected(Port* other) { if(m_onConnected) m_onConnected(other); }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    PortDirection m_dir;
    PortType m_type;
    QString m_name;
    QColor m_color;
    int m_maxConnections;
    QGraphicsTextItem *m_label;
    QList<Connection *> m_connections;
    std::function<void(Port*)> m_onConnected;
};

class Connection : public QGraphicsPathItem {
public:
    Connection(Port *from, Port *to);
    ~Connection();
    void updatePath();
private:
    Port *m_from;
    Port *m_to;
};

class Node : public QGraphicsRectItem {
public:
    Node(const QString &name, QGraphicsItem *parent = nullptr);
    Port* addPort(const QString &name, Port::PortDirection dir, Port::PortType type, int maxConnections = -1);
    QList<Port*> inputPorts() const { return m_inputPorts; }
    QList<Port*> outputPorts() const { return m_outputPorts; }
private:
    QGraphicsTextItem *m_label;
    QList<Port*> m_inputPorts;
    QList<Port*> m_outputPorts;
};

class Canvas : public QGraphicsView {
    Q_OBJECT
public:
    Canvas(QWidget *parent = nullptr);
    Node* addNode(const QString &name, const QPointF &pos);
    Connection* connectPorts(Port *from, Port *to);
    QList<Node*> nodes();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    Port* findPortAt(const QPointF &scenePos);
    QGraphicsScene *m_scene;
    QList<Node*> m_nodes;
    QList<Connection*> m_connections;

    Port *m_draggingPort = nullptr;
    QGraphicsPathItem *m_tempPath = nullptr;

    bool m_panning = false;
    QPoint m_lastPanPoint;
};
