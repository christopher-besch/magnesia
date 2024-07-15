#pragma once

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
// This is a hack to stop include cleaner from complaining. The macros are actually provided by QtGlobal because
// QtVersionChecks doesn't exist yet in older qt versions.
#include <QtVersionChecks>
#define MAGNESIA_HAS_QT_6_5
#endif
