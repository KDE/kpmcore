# SPDX-FileCopyrightText: 2023 Er2 <er2@dismail.de>

# SPDX-License-Identifier: GPL-3.0-or-later

kpmcore_add_plugin(pmgpartbackendplugin)

target_sources(pmgpartbackendplugin PRIVATE
    gpartbackend.cpp
    gpartdevice.cpp
    gpartpartitiontable.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/corebackenddevice.cpp
)

target_link_libraries(pmgpartbackendplugin kpmcore KF${KF_MAJOR_VERSION}::I18n KF${KF_MAJOR_VERSION}::CoreAddons geom)
