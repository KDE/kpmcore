# SPDX-FileCopyrightText: 2008,2012 Volker Lanz <vl@fidra.de>
# SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
# SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>
# SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

# SPDX-License-Identifier: GPL-3.0-or-later

add_definitions(-DTRANSLATION_DOMAIN=\"kpmcore\")

include_directories(
    ${BLKID_INCLUDE_DIRS}
)

include(backend/CMakeLists.txt)
include(core/CMakeLists.txt)
include(util/CMakeLists.txt)
include(ops/CMakeLists.txt)
include(jobs/CMakeLists.txt)
include(fs/CMakeLists.txt)
include(gui/CMakeLists.txt)

set(kpmcore_SRCS
    ${BACKEND_SRC}
    ${FS_SRC}
    ${CORE_SRC}
    ${OPS_SRC}
    ${JOBS_SRC}
    ${UTIL_SRC}
    ${GUI_SRC}
)

ki18n_wrap_ui(kpmcore_SRCS ${gui_UIFILES})

add_library(kpmcore SHARED ${kpmcore_SRCS})
target_link_libraries( kpmcore PUBLIC
    Qt6::Core
    PRIVATE
    ${BLKID_LIBRARIES}
    Qt6::DBus
    Qt6::Gui
    KF6::I18n
    KF6::CoreAddons
    KF6::WidgetsAddons
)
generate_export_header(kpmcore
    BASE_NAME LIBKPMCORE
    EXPORT_FILE_NAME util/libpartitionmanagerexport.h
)
list(APPEND UTIL_LIB_HDRS ${CMAKE_CURRENT_BINARY_DIR}/util/libpartitionmanagerexport.h)

target_include_directories(kpmcore INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/kpmcore>")

install(TARGETS kpmcore EXPORT KPMcoreTargets  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${CORE_LIB_HDRS} DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kpmcore/core/ COMPONENT Devel)
install(FILES ${BACKEND_LIB_HDRS} DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kpmcore/backend/ COMPONENT Devel)
install(FILES ${FS_LIB_HDRS} DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kpmcore/fs/ COMPONENT Devel)
install(FILES ${JOBS_LIB_HDRS} DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kpmcore/jobs/ COMPONENT Devel)
install(FILES ${OPS_LIB_HDRS} DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kpmcore/ops/ COMPONENT Devel)
install(FILES ${UTIL_LIB_HDRS} DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kpmcore/util/ COMPONENT Devel)
install(FILES ${GUI_LIB_HDRS} DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kpmcore/gui/ COMPONENT Devel)

############################################

add_subdirectory(plugins)
