#pragma once

#include "../../qt_version_check.hpp"
#include "dataviewer_fwd.hpp"
#include "panels/AttributeViewPanel.hpp"
#include "panels/LogViewPanel.hpp"
#include "panels/ReferenceViewPanel.hpp"
#include "panels/TreeViewPanel.hpp"

#include <array>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtTypeTraits>
#else
#include <QtGlobal>
#endif

/**
 * Creates a binary operator and its assigning version for the enum class `type`.
 *
 * @param type the type to generate the operator for
 * @param op the operator to define
 */
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MAGNESIA_BIN_OPERATOR(type, op)                                     \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses) */                        \
    inline constexpr type operator op(type lhs, type rhs) {                 \
        /* NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange) */  \
        return static_cast<type>(qToUnderlying(lhs) op qToUnderlying(rhs)); \
    }                                                                       \
                                                                            \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses) */                        \
    inline constexpr type operator op##=(type& lhs, type rhs) {             \
        return lhs = lhs op rhs;                                            \
    }

namespace magnesia::activities::dataviewer::panels {
    /**
     * List of registered panels used to fill the UI. Add your panels metadata here and an entry to `panels::Panels`.
     */
    inline constexpr std::array all{
        log_view_panel::metadata,
        treeview_panel::metadata,
        attribute_view_panel::metadata,
        reference_view_panel::metadata,
    };

    /**
     * Bitset used internally for identifying panel types at runtime. Add your panels here and it's metadata to
     * `panels::all`.
     */
    enum class Panels : unsigned {
        treeview       = 0x1 << 1,
        attribute      = 0x1 << 2,
        log_view       = 0x1 << 3,
        reference_view = 0x1 << 4,
    };

    /**
     * `operator&` and `operator&=` implementation
     *
     * @see MAGNESIA_BIN_OPERATOR(type, op)
     */
    MAGNESIA_BIN_OPERATOR(Panels, &);
    /**
     * `operator|` and `operator|=` implementation
     *
     * @see MAGNESIA_BIN_OPERATOR(type, op)
     */
    MAGNESIA_BIN_OPERATOR(Panels, |);
    /**
     * `operator^` and `operator^=` implementation
     *
     * @see MAGNESIA_BIN_OPERATOR(type, op)
     */
    MAGNESIA_BIN_OPERATOR(Panels, ^);
} // namespace magnesia::activities::dataviewer::panels

#undef MAGNESIA_BIN_OPERATOR
