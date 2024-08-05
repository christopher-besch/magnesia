#include "Application.hpp"
#include "qt_version_check.hpp"

#include <QApplication>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtLogging>
#else
#include <QtGlobal>
#endif

int main(int argc, char** argv) {
    qSetMessagePattern("%{time yyyy-MM-ddTHH:mm:ss.zzzttt} %{if-category}%{category} %{endif}%{type}: %{message}");

    const QApplication qapp(argc, argv);

    const magnesia::Application app;
    magnesia::Application::initializeActivities();

    return QApplication::exec();
}
