#pragma once

#include "qt_version_check.hpp"

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtLogging>
#else
#include <QtDebug>
#endif

/**
 * Termiante the application.
 *
 * Print any custom error message prior to this using `qWarning() << "some error";`
 *
 * @note Qt 6.5 introduced `qFatal() << "some error";` to terminate the application.
 * This doesn't work with earlier versions, therefore this syntax can't be used.
 */
[[noreturn]] inline void terminate() {
    qFatal("Fatal error, terminating");
}
