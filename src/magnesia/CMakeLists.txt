qt_add_executable(magnesia main.cpp)

target_link_libraries(magnesia PRIVATE Qt::Widgets open62541pp::open62541pp)

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
    # TODO rename FILENAME_VARIABLE to OUTPUT_SCRIPT when using Qt6.5 or newer
    qt_generate_deploy_app_script(
        TARGET magnesia
        FILENAME_VARIABLE deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif()
