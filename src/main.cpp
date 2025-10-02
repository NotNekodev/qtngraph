#include <QApplication>
#include "include/Canvas.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Canvas canvas;
    canvas.setWindowTitle(":3");
    canvas.resize(1000, 700);
    canvas.show();

    // Example nodes
    NetworkNode* nodeA = canvas.addNode("Router", QPointF(100, 100));
    NetworkNode* nodeB = canvas.addNode("Server", QPointF(400, 200));

    nodeA->addPort("IP Out", NetworkPort::Output, NetworkPort::Type_IP);
    nodeA->addPort("MAC Out", NetworkPort::Output, NetworkPort::Type_MAC);

    nodeB->addPort("IP In", NetworkPort::Input, NetworkPort::Type_IP);
    nodeB->addPort("MAC In", NetworkPort::Input, NetworkPort::Type_MAC);

    return a.exec();
}
