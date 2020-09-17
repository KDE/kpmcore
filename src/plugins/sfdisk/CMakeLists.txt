# SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>
# SPDX-FileCopyrightText: 2018 Andrius Štikonas <andrius@stikonas.eu>

# SPDX-License-Identifier: GPL-3.0-or-later

set (pmsfdiskbackendplugin_SRCS
    sfdiskbackend.cpp
    sfdiskdevice.cpp
    sfdiskgptattributes.cpp
    sfdiskpartitiontable.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/corebackenddevice.cpp
    ${CMAKE_SOURCE_DIR}/src/core/copysourcedevice.cpp
    ${CMAKE_SOURCE_DIR}/src/core/copytargetdevice.cpp
    ${CMAKE_SOURCE_DIR}/src/core/copytargetbytearray.cpp
)

add_library(pmsfdiskbackendplugin SHARED ${pmsfdiskbackendplugin_SRCS})

target_link_libraries(pmsfdiskbackendplugin kpmcore KF5::I18n KF5::CoreAddons)

install(TARGETS pmsfdiskbackendplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})
