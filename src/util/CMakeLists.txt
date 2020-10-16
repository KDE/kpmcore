set(application_interface_xml org.kde.kpmcore.applicationinterface.xml)
set(helper_interface_xml org.kde.kpmcore.helperinterface.xml)

qt5_generate_dbus_interface(
    util/externalcommand.h
    ${application_interface_xml}
    OPTIONS -a
)

qt5_generate_dbus_interface(
    util/externalcommandhelper.h
    ${helper_interface_xml}
    OPTIONS -a
)


find_package(PolkitQt5-1 REQUIRED)

qt5_add_dbus_interface(ApplicationInterface_SRCS ${CMAKE_CURRENT_BINARY_DIR}/${application_interface_xml} externalcommand_interface)
qt5_add_dbus_interface(HelperInterface_SRCS ${CMAKE_CURRENT_BINARY_DIR}/${helper_interface_xml} externalcommandhelper_interface)

set(UTIL_SRC
    ${HelperInterface_SRCS}
    util/capacity.cpp
    util/externalcommand.cpp
    util/globallog.cpp
    util/helpers.cpp
    util/htmlreport.cpp
    util/report.cpp
)

set(UTIL_LIB_HDRS
    util/libpartitionmanagerexport.h
    util/capacity.h
    util/externalcommand.h
    util/globallog.h
    util/helpers.h
    util/htmlreport.h
    util/report.h
)

add_executable(kpmcore_externalcommand
    ${ApplicationInterface_SRCS}
    util/externalcommandhelper.cpp
)

target_link_libraries(kpmcore_externalcommand
    qca-qt5
    Qt5::Core
    Qt5::DBus
    KF5::AuthCore
    KF5::I18n
    PolkitQt5-1::Core
)

install(TARGETS kpmcore_externalcommand DESTINATION ${KDE_INSTALL_LIBEXECDIR})
install( FILES util/org.kde.kpmcore.helperinterface.conf DESTINATION ${KDE_INSTALL_DBUSDIR}/system.d )
install( FILES util/org.kde.kpmcore.applicationinterface.conf DESTINATION ${KDE_INSTALL_DBUSDIR}/system.d )

kauth_install_actions(org.kde.kpmcore.externalcommand util/org.kde.kpmcore.externalcommand.actions)


function(ecm_install_configured_files)
    set(options COPYONLY ESCAPE_QUOTES @ONLY)
    set(oneValueArgs DESTINATION COMPONENT)
    set(multiValueArgs INPUT)

    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN})


    if(ARGS_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown arguments given to ecm_install_configured_file(): \"${ARGS_UNPARSED_ARGUMENTS}\"")
    endif()

    if (NOT ARGS_DESTINATION)
        message(FATAL_ERROR "missing DESTINATION argument to ECMConfiguredInstall")
    endif()

    foreach(_input ${ARGS_INPUT})
        # convert absolute paths
        get_filename_component(_name ${_input} NAME)

        string(REGEX REPLACE "\\.in$"  "" _name ${_name})

        set(_out_file ${CMAKE_CURRENT_BINARY_DIR}/${_name})

        set(_configure_args)
        if (ARGS_COPY_ONLY)
                list(APPEND _configure_args COPY_ONLY)
        endif()
        if (ARGS_ESCAPE_QUOTES)
                list(APPEND _configure_args  ESCAPE_QUOTES)
        endif()
        if (ARGS_@ONLY)
                list(APPEND _configure_args @ONLY)
        endif()

        configure_file(${_input} ${_out_file} ${_configure_args})

        if (DEFINED ARGS_COMPONENT)
            set(_component COMPONENT ${ARGS_COMPONENT})
        else()
            set(_component)
        endif()

        install(FILES ${_out_file} DESTINATION ${ARGS_DESTINATION} ${_component})
    endforeach()
endfunction()

ecm_install_configured_files(
    INPUT util/org.kde.kpmcore.helperinterface.service.in
    DESTINATION ${KDE_INSTALL_DBUSDIR}/system-services
)
