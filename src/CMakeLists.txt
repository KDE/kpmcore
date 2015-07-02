# Copyright (C) 2008, 2012 by Volker Lanz <vl@fidra.de>
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

include(backend/CMakeLists.txt)
include(core/CMakeLists.txt)
include(util/CMakeLists.txt)
include(ops/CMakeLists.txt)
include(jobs/CMakeLists.txt)
include(fs/CMakeLists.txt)

set(kpmcore_SRCS
    ${BACKEND_SRC}
    ${FS_SRC}
    ${CORE_SRC}
    ${OPS_SRC}
    ${JOBS_SRC}
    ${UTIL_SRC}
)

file(GLOB kpmcore_UIFILES config/*.ui)

ki18n_wrap_ui(kpmcore_SRCS ${kpmcore_UIFILES})

kconfig_add_kcfg_files(kpmcore_SRCS config.kcfgc)

add_library(kpmcore SHARED ${kpmcore_SRCS})
target_link_libraries( kpmcore
    ${UUID_LIBRARIES}
    ${BLKID_LIBRARIES}
    ${LIBATASMART_LIBRARIES}
    KF5::ConfigCore
    KF5::ConfigGui
    KF5::I18n
    KF5::IconThemes
    KF5::KIOCore
    KF5::Service
)

install(TARGETS kpmcore EXPORT KPMcoreTargets  ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${CORE_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/kpmcore/core/ COMPONENT Devel)
install(FILES ${BACKEND_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/kpmcore/backend/ COMPONENT Devel)
install(FILES ${FS_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/kpmcore/fs/ COMPONENT Devel)
install(FILES ${JOBS_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/kpmcore/jobs/ COMPONENT Devel)
install(FILES ${OPS_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/kpmcore/ops/ COMPONENT Devel)
install(FILES ${UTIL_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/kpmcore/util/ COMPONENT Devel)

############################################

add_subdirectory(plugins)
