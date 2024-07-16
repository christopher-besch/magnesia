set(SOURCES
    # this comment doesn't seem to work in qt_add_executable
    # cmake-format: sortable
    activities/activities.cpp
    activities/add_activity/AddActivity.cpp
    Activity.cpp
    ActivityMetadata.cpp
    Application.cpp
    ConfigWidget.cpp
    database_types.cpp
    Router.cpp
    settings.cpp
    SettingsManager.cpp
    SQLStorageManager.cpp
    StorageManager.cpp
    terminate.cpp
)
qt_add_executable(magnesia main.cpp ${SOURCES})

target_include_directories(magnesia PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    magnesia
    PRIVATE magnesia::project_options
            Qt::Widgets
            Qt::Sql
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
    set(DOXYGEN_WARN_AS_ERROR YES)
    set(DOXYGEN_VERBATIM_HEADERS NO)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_UML_LOOK YES)
    doxygen_add_docs(magnesia-docs ${CMAKE_CURRENT_SOURCE_DIR} ALL)
endif()
