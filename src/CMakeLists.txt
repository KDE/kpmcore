# SPDX-FileCopyrightText: 2008,2012 Volker Lanz <vl@fidra.de>
# SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
# SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>

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
    Qt5::Core
    PRIVATE
    ${BLKID_LIBRARIES}
    Qt5::DBus
    Qt5::Gui
    KF5::I18n
    KF5::CoreAddons
    KF5::WidgetsAddons
)
generate_export_header(kpmcore
    BASE_NAME LIBKPMCORE
    EXPORT_FILE_NAME util/libpartitionmanagerexport.h
)
list(APPEND UTIL_LIB_HDRS ${CMAKE_CURRENT_BINARY_DIR}/util/libpartitionmanagerexport.h)

install(TARGETS kpmcore EXPORT KPMcoreTargets  ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${CORE_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/kpmcore/core/ COMPONENT Devel)
install(FILES ${BACKEND_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/kpmcore/backend/ COMPONENT Devel)
install(FILES ${FS_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/kpmcore/fs/ COMPONENT Devel)
install(FILES ${JOBS_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/kpmcore/jobs/ COMPONENT Devel)
install(FILES ${OPS_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/kpmcore/ops/ COMPONENT Devel)
install(FILES ${UTIL_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/kpmcore/util/ COMPONENT Devel)
install(FILES ${GUI_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/kpmcore/gui/ COMPONENT Devel)

############################################

add_subdirectory(plugins)
