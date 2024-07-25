#pragma once

#include "opcua_qt/Connection.hpp"
#include "opcua_qt/abstraction/AccessLevelBitmask.hpp"
#include "opcua_qt/abstraction/AttributeId.hpp"
#include "opcua_qt/abstraction/DataValue.hpp"
#include "opcua_qt/abstraction/EventNotifierBitmask.hpp"
#include "opcua_qt/abstraction/LocalizedText.hpp"
#include "opcua_qt/abstraction/NodeClass.hpp"
#include "opcua_qt/abstraction/NodeId.hpp"
#include "opcua_qt/abstraction/QualifiedName.hpp"
#include "opcua_qt/abstraction/ValueRank.hpp"
#include "opcua_qt/abstraction/WriteMaskBitmask.hpp"
#include "opcua_qt/abstraction/node/Node.hpp"
#include "qt_version_check.hpp"

#include <cstdint>
#include <optional>

#include <QAbstractItemModel>
#include <QList>
#include <QObject>
#include <QVariant>
#include <Qt>
#include <qtmetamacros.h>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtTypes>
#else
#include <QtGlobal>
#endif

namespace magnesia::activities::dataviewer::panels::attribute_view_panel {
    struct NodeProxy {
        std::optional<opcua_qt::abstraction::NodeId>               node_id;
        std::optional<opcua_qt::abstraction::NodeClass>            node_class;
        std::optional<opcua_qt::abstraction::QualifiedName>        browse_name;
        std::optional<opcua_qt::abstraction::LocalizedText>        display_name;
        std::optional<opcua_qt::abstraction::LocalizedText>        description;
        std::optional<opcua_qt::abstraction::WriteMaskBitmask>     write_mask;
        std::optional<opcua_qt::abstraction::WriteMaskBitmask>     user_write_mask;
        std::optional<bool>                                        is_abstract;
        std::optional<bool>                                        is_symmetric;
        std::optional<opcua_qt::abstraction::LocalizedText>        inverse_name;
        std::optional<bool>                                        contains_no_loops;
        std::optional<opcua_qt::abstraction::EventNotifierBitmask> event_notifier;
        std::optional<opcua_qt::abstraction::DataValue>            data_value;
        std::optional<opcua_qt::abstraction::Node*>                data_type;
        std::optional<opcua_qt::abstraction::ValueRank>            value_rank;
        std::optional<QList<quint32>>                              array_dimensions;
        std::optional<opcua_qt::abstraction::AccessLevelBitmask>   access_level;
        std::optional<opcua_qt::abstraction::AccessLevelBitmask>   user_access_level;
        std::optional<double>                                      minimum_sampling_interval;
        std::optional<bool>                                        is_historizing;
        std::optional<bool>                                        is_executable;
        std::optional<bool>                                        is_user_executable;
    };

    class AttributeViewModel : public QAbstractItemModel {
        Q_OBJECT

      public:
        explicit AttributeViewModel(QObject* parent = nullptr);

        [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QModelIndex parent(const QModelIndex& index) const override;
        [[nodiscard]] int         rowCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] int         columnCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QVariant    data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

        [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                          int role = Qt::DisplayRole) const override;

        void setNode(opcua_qt::abstraction::Node* node, opcua_qt::Connection* connection);

      private:
        [[nodiscard]] static constexpr uint32_t itemId(opcua_qt::abstraction::AttributeId attribute,
                                                       uint8_t                            sub_item) noexcept;

        [[nodiscard]] static constexpr uint32_t itemId(uint32_t attribute, uint8_t sub_item) noexcept;

        [[nodiscard]] static constexpr opcua_qt::abstraction::AttributeId attributeId(uint32_t item_id) noexcept;
        [[nodiscard]] static constexpr uint8_t                            subId(uint32_t item_id) noexcept;
        [[nodiscard]] static uint32_t                                     itemId(QModelIndex index) noexcept;

      private:
        NodeProxy                                 m_node;
        QList<opcua_qt::abstraction::AttributeId> m_available_attributes;
    };
} // namespace magnesia::activities::dataviewer::panels::attribute_view_panel
