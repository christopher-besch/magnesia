#pragma once

#include "../../../opcua_qt/Connection.hpp"
#include "../../../opcua_qt/abstraction/AccessLevelBitmask.hpp"
#include "../../../opcua_qt/abstraction/AttributeId.hpp"
#include "../../../opcua_qt/abstraction/DataValue.hpp"
#include "../../../opcua_qt/abstraction/EventNotifierBitmask.hpp"
#include "../../../opcua_qt/abstraction/LocalizedText.hpp"
#include "../../../opcua_qt/abstraction/NodeClass.hpp"
#include "../../../opcua_qt/abstraction/NodeId.hpp"
#include "../../../opcua_qt/abstraction/QualifiedName.hpp"
#include "../../../opcua_qt/abstraction/ValueRank.hpp"
#include "../../../opcua_qt/abstraction/WriteMaskBitmask.hpp"
#include "../../../opcua_qt/abstraction/node/Node.hpp"

#include <cstdint>
#include <optional>
#include <vector>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QObject>
#include <QVariant>
#include <Qt>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::attribute_view_panel {

    /**
     * @class NodeProxy
     * @brief Proxy for a node.
     *
     * Is used to cache node data for performance enhancements.
     */
    struct NodeProxy {
        /// Id of the node.
        std::optional<opcua_qt::abstraction::NodeId> node_id;

        /// Class of the node.
        std::optional<opcua_qt::abstraction::NodeClass> node_class;

        /// Actual name of the node.
        std::optional<opcua_qt::abstraction::QualifiedName> browse_name;

        /// Display name of the node.
        std::optional<opcua_qt::abstraction::LocalizedText> display_name;

        /// Description of the node.
        std::optional<opcua_qt::abstraction::LocalizedText> description;

        /// Stores the write mask of the node.
        std::optional<opcua_qt::abstraction::WriteMaskBitmask> write_mask;

        /// Stores the write mask of the user.
        std::optional<opcua_qt::abstraction::WriteMaskBitmask> user_write_mask;

        /// Marks if the node is abstract.
        std::optional<bool> is_abstract;

        /// Marks if the node is symmetric.
        std::optional<bool> is_symmetric;

        /// Stores the inverse name of a node.
        std::optional<opcua_qt::abstraction::LocalizedText> inverse_name;

        /// Marks if the node contains no loops.
        std::optional<bool> contains_no_loops;

        /// Stores the event notifier of the node.
        std::optional<opcua_qt::abstraction::EventNotifierBitmask> event_notifier;

        /// Stores the value of the node.
        std::optional<opcua_qt::abstraction::DataValue> data_value;

        /// Stores which datatype the node has.
        std::optional<opcua_qt::abstraction::Node*> data_type;

        /// Stores the rank of the node value.
        std::optional<opcua_qt::abstraction::ValueRank> value_rank;

        /// Stores the array dimensions of the node.
        std::optional<std::vector<std::uint32_t>> array_dimensions;

        /// Stores the access level for the node.
        std::optional<opcua_qt::abstraction::AccessLevelBitmask> access_level;

        /// Stores the access level from the user.
        std::optional<opcua_qt::abstraction::AccessLevelBitmask> user_access_level;

        /// Stores the minimum sampling interval.
        std::optional<double> minimum_sampling_interval;

        /// Marks if the node keeps history.
        std::optional<bool> is_historizing;

        /// Marks if the node is executable.
        std::optional<bool> is_executable;

        /// Marks if the user can execute methods from the node.
        std::optional<bool> is_user_executable;
    };

    /**
     * @class AttributeViewModel
     * @brief Model for AttributeViewPanel.
     */
    class AttributeViewModel : public QAbstractItemModel {
        Q_OBJECT

      public:
        /**
         * @param parent Parent of the AttributeViewModel.
         */
        explicit AttributeViewModel(QObject* parent = nullptr);

        [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QModelIndex parent(const QModelIndex& index) const override;
        [[nodiscard]] int         rowCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] int         columnCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QVariant    data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        [[nodiscard]] QVariant    headerData(int section, Qt::Orientation orientation,
                                             int role = Qt::DisplayRole) const override;

        /**
         * Set the view to a specific Node.
         *
         * @param node The node.
         * @param connection Connection to the node.
         */
        void setNode(opcua_qt::abstraction::Node* node, opcua_qt::Connection* connection);

      private:
        NodeProxy                                       m_node;
        std::vector<opcua_qt::abstraction::AttributeId> m_available_attributes;
    };
} // namespace magnesia::activities::dataviewer::panels::attribute_view_panel
