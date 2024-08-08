set(SOURCES
    # this comment doesn't seem to work in qt_add_executable
    # cmake-format: sortable
    activities/about/About.cpp
    activities/activities.cpp
    activities/add_activity/AddActivity.cpp
    activities/dataviewer/ConfigWidget.cpp
    activities/dataviewer/DataViewer.cpp
    activities/dataviewer/dataviewer_fwd.cpp
    activities/dataviewer/layout.cpp
    activities/dataviewer/metadata.cpp
    activities/dataviewer/Panel.cpp
    activities/dataviewer/PanelMetadata.cpp
    activities/dataviewer/panels.cpp
    activities/dataviewer/panels/AttributeViewModel.cpp
    activities/dataviewer/panels/AttributeViewPanel.cpp
    activities/dataviewer/panels/LogViewModel.cpp
    activities/dataviewer/panels/LogViewPanel.cpp
    activities/dataviewer/panels/Node.cpp
    activities/dataviewer/panels/ReferenceViewModel.cpp
    activities/dataviewer/panels/ReferenceViewPanel.cpp
    activities/dataviewer/panels/TreeViewModel.cpp
    activities/dataviewer/panels/TreeViewPanel.cpp
    Activity.cpp
    ActivityMetadata.cpp
    Application.cpp
    ConfigWidget.cpp
    database_types.cpp
    HistoricServerConnection.cpp
    Layout.cpp
    opcua_qt/abstraction/AccessLevel.cpp
    opcua_qt/abstraction/AccessLevelBitmask.cpp
    opcua_qt/abstraction/AttributeId.cpp
    opcua_qt/abstraction/DataValue.cpp
    opcua_qt/abstraction/Endpoint.cpp
    opcua_qt/abstraction/EventNotifier.cpp
    opcua_qt/abstraction/EventNotifierBitmask.cpp
    opcua_qt/abstraction/LocalizedText.cpp
    opcua_qt/abstraction/LogCategory.cpp
    opcua_qt/abstraction/LogLevel.cpp
    opcua_qt/abstraction/MessageSecurityMode.cpp
    opcua_qt/abstraction/MonitoredItem.cpp
    opcua_qt/abstraction/MonitoringMode.cpp
    opcua_qt/abstraction/MonitoringParameters.cpp
    opcua_qt/abstraction/node/DataTypeNode.cpp
    opcua_qt/abstraction/node/MethodNode.cpp
    opcua_qt/abstraction/node/Node.cpp
    opcua_qt/abstraction/node/ObjectNode.cpp
    opcua_qt/abstraction/node/ObjectTypeNode.cpp
    opcua_qt/abstraction/node/ReferenceTypeNode.cpp
    opcua_qt/abstraction/node/VariableNode.cpp
    opcua_qt/abstraction/node/VariableTypeNode.cpp
    opcua_qt/abstraction/node/ViewNode.cpp
    opcua_qt/abstraction/NodeClass.cpp
    opcua_qt/abstraction/NodeId.cpp
    opcua_qt/abstraction/QualifiedName.cpp
    opcua_qt/abstraction/ReferenceDescription.cpp
    opcua_qt/abstraction/StatusCode.cpp
    opcua_qt/abstraction/Subscription.cpp
    opcua_qt/abstraction/SubscriptionParameters.cpp
    opcua_qt/abstraction/ValueRank.cpp
    opcua_qt/abstraction/Variant.cpp
    opcua_qt/abstraction/WriteMask.cpp
    opcua_qt/abstraction/WriteMaskBitmask.cpp
    opcua_qt/ApplicationCertificate.cpp
    opcua_qt/Connection.cpp
    opcua_qt/ConnectionBuilder.cpp
    opcua_qt/ConnectionManager.cpp
    opcua_qt/LogEntry.cpp
    opcua_qt/Logger.cpp
    Router.cpp
    settings.cpp
    SettingsManager.cpp
    SQLStorageManager.cpp
    StorageManager.cpp
    terminate.cpp
)
qt_add_executable(magnesia main.cpp ${SOURCES})

target_link_libraries(
    magnesia
    PRIVATE magnesia::project_options
            Qt::Core
            Qt::Widgets
            Qt::Sql
            Qt::Network
            open62541pp::open62541pp
)

set_target_properties(
    magnesia
    PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

install(
    TARGETS magnesia
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(APPLE OR WIN32)
    if(Qt6_VERSION VERSION_GREATER_EQUAL 6.5)
        set(OUTPUT_VAR OUTPUT_SCRIPT)
    else()
        set(OUTPUT_VAR FILENAME_VARIABLE)
    endif()

    # cmake-format doesn't handle the ${OUTPUT_VAR} well
    # cmake-format: off
    qt_generate_deploy_app_script(
        TARGET magnesia
        ${OUTPUT_VAR} deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    # cmake-format: on
    install(SCRIPT ${deploy_script})
endif()

if(MAGNESIA_BUILD_DOCS)
    set(DOXYGEN_CREATE_SUBDIRS YES)
    set(DOXYGEN_EXTRACT_ALL YES)
    if(MAGNESIA_WARNINGS_AS_ERRORS)
        set(DOXYGEN_WARN_AS_ERROR YES)
    else()
        set(DOXYGEN_WARN_AS_ERROR NO)
    endif()
    set(DOXYGEN_VERBATIM_HEADERS NO)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_UML_LOOK YES)
    doxygen_add_docs(magnesia-docs ${CMAKE_CURRENT_SOURCE_DIR} ALL)
endif()
