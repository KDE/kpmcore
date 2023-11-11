# SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
# SPDX-FileCopyrightText: 2016-2021 Andrius Štikonas <andrius@stikonas.eu>

# SPDX-License-Identifier: GPL-3.0-or-later

kpmcore_add_plugin(pmdummybackendplugin)

target_sources(pmdummybackendplugin PRIVATE
    dummybackend.cpp
    dummydevice.cpp
    dummypartitiontable.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/corebackenddevice.cpp
)

target_link_libraries(pmdummybackendplugin kpmcore KF6::I18n KF6::CoreAddons)
