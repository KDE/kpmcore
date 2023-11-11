# SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>
# SPDX-FileCopyrightText: 2021 Andrius Štikonas <andrius@stikonas.eu>

# SPDX-License-Identifier: GPL-3.0-or-later

kpmcore_add_plugin(pmsfdiskbackendplugin)

target_sources(pmsfdiskbackendplugin PRIVATE
    sfdiskbackend.cpp
    sfdiskdevice.cpp
    sfdiskgptattributes.cpp
    sfdiskpartitiontable.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/corebackenddevice.cpp
    ${CMAKE_SOURCE_DIR}/src/core/copysourcedevice.cpp
    ${CMAKE_SOURCE_DIR}/src/core/copytargetdevice.cpp
    ${CMAKE_SOURCE_DIR}/src/core/copytargetbytearray.cpp
)

target_link_libraries(pmsfdiskbackendplugin kpmcore KF6::I18n KF6::CoreAddons)
