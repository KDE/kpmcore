# SPDX-FileCopyrightText: 2023 Er2 <er2@dismail.de>

# SPDX-License-Identifier: GPL-3.0-or-later

kpmcore_add_plugin(pmgeombackendplugin)

target_sources(pmgeombackendplugin PRIVATE
    geombackend.cpp
    geomdevice.cpp
    geompartitiontable.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/corebackenddevice.cpp
)

target_link_libraries(pmgeombackendplugin kpmcore KF6::I18n KF6::CoreAddons geom)
