# SPDX-FileCopyrightText: 2008,2012 Volker Lanz <vl@fidra.de>
# SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
# SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
# SPDX-FileCopyrightText: 2015 Andrius Štikonas <andrius@stikonas.eu>
# SPDX-FileCopyrightText: 2018 Huzaifa Faruqui <huzaifafaruqui@gmail.com>
# SPDX-FileCopyrightText: 2019 Albert Astals Cid <aacid@kde.org>
# SPDX-FileCopyrightText: 2019 Antonio Rojas <arojas@archlinux.org>
# SPDX-FileCopyrightText: 2020 David Edmundson <kde@davidedmundson.co.uk>

# SPDX-License-Identifier: GPL-3.0-or-later

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

add_executable(kpmcore_mdadmupdateconf
    util/mdadmupdateconf.cpp
)

add_definitions(
    -DLIBEXECDIRPATH="${LIBEXEC_INSTALL_DIR}"
)

target_link_libraries(kpmcore_externalcommand
    qca-qt5
    Qt5::Core
    Qt5::DBus
    KF5::AuthCore
    KF5::I18n
    PolkitQt5-1::Core
)

install(TARGETS kpmcore_mdadmupdateconf DESTINATION ${LIBEXEC_INSTALL_DIR})
install(TARGETS kpmcore_externalcommand DESTINATION ${KDE_INSTALL_LIBEXECDIR})
install( FILES util/org.kde.kpmcore.helperinterface.conf DESTINATION ${KDE_INSTALL_DBUSDIR}/system.d )
install( FILES util/org.kde.kpmcore.applicationinterface.conf DESTINATION ${KDE_INSTALL_DBUSDIR}/system.d )

kauth_install_actions(org.kde.kpmcore.externalcommand util/org.kde.kpmcore.externalcommand.actions)
ecm_install_configured_files(
    INPUT util/org.kde.kpmcore.helperinterface.service.in
    DESTINATION ${KDE_INSTALL_DBUSDIR}/system-services
)
