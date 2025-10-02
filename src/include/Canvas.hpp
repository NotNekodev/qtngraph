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

class NetworkPort;
class Connection;
class NetworkNode;

class NetworkPort : public QGraphicsRectItem {
public:
    enum PortDirection { Input, Output };
    enum PortType { Type_Generic, Type_IP, Type_MAC };

    NetworkPort(const QString &name, PortDirection dir, PortType type, QGraphicsItem *parent = nullptr, int maxconnections = -1);

    PortDirection direction() const { return m_dir; }
    PortType portType() const { return m_type; }
    QString name() const { return m_name; }
    int maxConnections() const { return m_maxConnections; };
    QList<Connection *> connections() const { return m_connections; };

    void addConnection(Connection *conn);
    void removeConnection(Connection *conn);

    std::any privateData;
    template<typename T>
    void setData(const T& value) { privateData = value; }
    template<typename T>
    T getData() const { return std::any_cast<T>(privateData); }

    void setOnConnected(std::function<void(NetworkPort*)> callback) { m_onConnected = callback; }
    void triggerConnected(NetworkPort* other) { if(m_onConnected) m_onConnected(other); }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    PortDirection m_dir;
    PortType m_type;
    QString m_name;
    int m_maxConnections;
    QGraphicsTextItem *m_label;
    QList<Connection *> m_connections;
    std::function<void(NetworkPort*)> m_onConnected;
};

class Connection : public QGraphicsPathItem {
public:
    Connection(NetworkPort *from, NetworkPort *to);
    ~Connection();
    void updatePath();
private:
    NetworkPort *m_from;
    NetworkPort *m_to;
};

class NetworkNode : public QGraphicsRectItem {
public:
    NetworkNode(const QString &name, QGraphicsItem *parent = nullptr);
    NetworkPort* addPort(const QString &name, NetworkPort::PortDirection dir, NetworkPort::PortType type, int maxConnections = -1);
    QList<NetworkPort*> inputPorts() const { return m_inputPorts; }
    QList<NetworkPort*> outputPorts() const { return m_outputPorts; }
private:
    QGraphicsTextItem *m_label;
    QList<NetworkPort*> m_inputPorts;
    QList<NetworkPort*> m_outputPorts;
};

class Canvas : public QGraphicsView {
    Q_OBJECT
public:
    Canvas(QWidget *parent = nullptr);
    NetworkNode* addNode(const QString &name, const QPointF &pos);
    Connection* connectPorts(NetworkPort *from, NetworkPort *to);
    QList<NetworkNode*> nodes();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    NetworkPort* findPortAt(const QPointF &scenePos);
    QGraphicsScene *m_scene;
    QList<NetworkNode*> m_nodes;
    QList<Connection*> m_connections;

    NetworkPort *m_draggingPort = nullptr;
    QGraphicsPathItem *m_tempPath = nullptr;

    bool m_panning = false;
    QPoint m_lastPanPoint;
};
