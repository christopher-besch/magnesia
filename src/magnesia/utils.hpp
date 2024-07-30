#pragma once

#include "qt_version_check.hpp"

#include <cstddef>
#include <cstdint>
#include <ranges>
#include <span>
#include <vector>

#include <open62541pp/Span.h>
#include <open62541pp/types/Builtin.h>

#include <qcontainerfwd.h>
#include <qlist.h>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtLogging>
#include <QtTypes>
#else
#include <QtDebug>
#include <QtGlobal>
#endif

/**
 * Terminate the application.
 *
 * Print any custom error message prior to this using `qWarning() << "some error";`
 *
 * @note Qt 6.5 introduced `qFatal() << "some error";` to terminate the application.
 * This doesn't work with earlier versions, therefore this syntax can't be used.
 */
[[noreturn]] inline void terminate() {
    qFatal("Fatal error, terminating");
}
