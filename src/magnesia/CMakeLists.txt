qt_add_executable(magnesia main.cpp)

target_link_libraries(magnesia PRIVATE magnesia::project_options Qt::Widgets open62541pp::open62541pp)

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

    qt_generate_deploy_app_script(
        TARGET magnesia
        ${OUTPUT_VAR} deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif()
