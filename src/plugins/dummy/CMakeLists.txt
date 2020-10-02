# SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
# SPDX-FileCopyrightText: 2016-2018 Andrius Štikonas <andrius@stikonas.eu>

# SPDX-License-Identifier: GPL-3.0-or-later

set (pmdummybackendplugin_SRCS
    dummybackend.cpp
    dummydevice.cpp
    dummypartitiontable.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/corebackenddevice.cpp
)

add_library(pmdummybackendplugin SHARED ${pmdummybackendplugin_SRCS})

target_link_libraries(pmdummybackendplugin kpmcore KF5::I18n KF5::CoreAddons)

install(TARGETS pmdummybackendplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})
