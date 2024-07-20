#include "Application.hpp"

#include <QApplication>

int main(int argc, char** argv) {
    const QApplication qapp(argc, argv);

    const magnesia::Application app;
    magnesia::Application::initializeActivities();

    return QApplication::exec();
}
