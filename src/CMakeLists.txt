# Copyright (C) 2008, 2012 by Volker Lanz <vl@fidra.de>
# Copyright (C) 2015       by Teo Mrnjavac <teo@kde.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

############################################

add_definitions(-DTRANSLATION_DOMAIN=\"kpmcore\")

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
    qca-qt5
    KF5::I18n
    KF5::CoreAddons
    KF5::WidgetsAddons
    KF5::Auth
)

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
