#include <open62541pp/Server.h>

#include <QApplication>
#include <QLabel>
#include <qnamespace.h>

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    opcua::Server server;
    server.run();

    QLabel label("Hello, World!");
    label.setAlignment(Qt::AlignCenter);
    label.show();

    return QApplication::exec();
}
