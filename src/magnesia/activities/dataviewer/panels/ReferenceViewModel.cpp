#include "ReferenceViewModel.hpp"

#include "../../../opcua_qt/Connection.hpp"
#include "../../../opcua_qt/abstraction/node/Node.hpp"

#include <cstddef>
#include <utility>

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QObject>
#include <QVariant>
#include <Qt>

namespace magnesia::activities::dataviewer::panels::reference_view_panel {
    ReferenceViewModel::ReferenceViewModel(opcua_qt::Connection* connection, QObject* parent)
        : QAbstractTableModel(parent), m_connection(connection) {}

    int ReferenceViewModel::rowCount(const QModelIndex& /*parent*/) const {
        return static_cast<int>(m_references.size());
    }

    int ReferenceViewModel::columnCount(const QModelIndex& /*parent*/) const {
        return 2;
    }

    QVariant ReferenceViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
        if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
            return {};
        }

        switch (section) {
            case 0:
                return "Reference";
            case 1:
                return "Target Displayname";
            default:
                return {};
        }
    }

    QVariant ReferenceViewModel::data(const QModelIndex& index, int role) const {
        if (!checkIndex(index) || role != Qt::DisplayRole) {
            return {};
        }

        auto reference = m_references[static_cast<std::size_t>(index.row())];
        switch (index.column()) {
            case 0:
                return reference.first;
            case 1:
                return reference.second;
            default:
                return {};
        }
    }

    void ReferenceViewModel::nodeSelected(opcua_qt::abstraction::Node* node) {
        beginResetModel();
        m_references.clear();
        auto references = node->getReferences();
        m_references.clear();

        for (const auto& reference : references) {
            auto  node_id        = reference.getReferenceType();
            auto  is_forward     = reference.isForward();
            auto* reference_type = m_connection->getNode(node_id);

            QString reference_name;

            if (!is_forward) {
                auto inverse_name = reference_type->getInverseName();
                if (inverse_name.has_value()) {
                    reference_name = inverse_name->getText();
                }
            }

            if (reference_name.isNull()) {
                reference_name = reference_type->getDisplayName().getText();
            }

            m_references.emplace_back(reference_name, reference.getDisplayName().getText());
        }
        endResetModel();
    }
} // namespace magnesia::activities::dataviewer::panels::reference_view_panel
