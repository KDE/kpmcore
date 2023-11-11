# SPDX-FileCopyrightText: 2008,2012 Volker Lanz <vl@fidra.de>
# SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
# SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
# SPDX-FileCopyrightText: 2015-2020 Andrius Štikonas <andrius@stikonas.eu>
# SPDX-FileCopyrightText: 2018 Huzaifa Faruqui <huzaifafaruqui@gmail.com>
# SPDX-FileCopyrightText: 2019 Albert Astals Cid <aacid@kde.org>
# SPDX-FileCopyrightText: 2019 Antonio Rojas <arojas@archlinux.org>
# SPDX-FileCopyrightText: 2020 David Edmundson <kde@davidedmundson.co.uk>

# SPDX-License-Identifier: GPL-3.0-or-later

set(helper_interface_xml org.kde.kpmcore.helperinterface.xml)

file(READ "util/trustedprefixes" TRUSTED_PREFIXES)
string(REGEX REPLACE ";" "\\\\;" TRUSTED_PREFIXES "${TRUSTED_PREFIXES}")
string(REGEX REPLACE "\n" ";" TRUSTED_PREFIXES "${TRUSTED_PREFIXES}")
foreach(TRUSTED_PREFIX ${TRUSTED_PREFIXES})
    list(APPEND TRUSTED_PREFIXES_LIST "    QStringLiteral(\"${TRUSTED_PREFIX}\")")
endforeach()
string(REPLACE ";    QStringLiteral(" ",\n    QStringLiteral(" TRUSTED_PREFIXES_LIST "${TRUSTED_PREFIXES_LIST}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS util/trustedprefixes)
configure_file(util/externalcommand_trustedprefixes.h.in util/externalcommand_trustedprefixes.h)

qt_generate_dbus_interface(
    util/externalcommand.h
    ${application_interface_xml}
    OPTIONS -a
)

qt_generate_dbus_interface(
    util/externalcommandhelper.h
    ${helper_interface_xml}
    OPTIONS -a
)

qt_add_dbus_interface(HelperInterface_SRCS ${CMAKE_CURRENT_BINARY_DIR}/${helper_interface_xml} externalcommandhelper_interface)

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
    util/capacity.h
    util/externalcommand.h
    util/globallog.h
    util/helpers.h
    util/htmlreport.h
    util/report.h
)

add_executable(kpmcore_externalcommand
    util/externalcommandhelper.cpp
)

target_link_libraries(kpmcore_externalcommand
    Qt6::Core
    Qt6::DBus
    KF6::I18n
    PolkitQt6-1::Core
)

install(TARGETS kpmcore_externalcommand DESTINATION ${KDE_INSTALL_LIBEXECDIR})
install( FILES util/org.kde.kpmcore.helperinterface.conf DESTINATION ${KDE_INSTALL_DBUSDIR}/system.d )

install( FILES util/org.kde.kpmcore.externalcommand.policy DESTINATION ${POLKITQT-1_POLICY_FILES_INSTALL_DIR})
ecm_install_configured_files(
    INPUT util/org.kde.kpmcore.helperinterface.service.in
    DESTINATION ${KDE_INSTALL_DBUSDIR}/system-services
)
