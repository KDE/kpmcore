# Copyright (C) 2008 by Volker Lanz <vl@fidra.de>
# Copyright (C) 2014-2017 by Andrius Štikonas <andrius@stikonas.eu>
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

project(kpmcore)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(CMAKE_USE_RELATIVE_PATHS OFF)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

set(QT_MIN_VERSION "5.7.0")
set(VERSION_MAJOR "3")
set(VERSION_MINOR "3")
set(VERSION_RELEASE "0")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_RELEASE})
set(SOVERSION "8")
add_definitions(-D'VERSION="${VERSION}"') #"

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)
include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMPackageConfigHelpers)

ecm_setup_version(${VERSION} VARIABLE_PREFIX KPMCORE
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kpmcore_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPMcoreConfigVersion.cmake"
                        SOVERSION ${SOVERSION})

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
  Core
  DBus
  Gui
  Widgets
)

# Load the frameworks we need
find_package(KF5 REQUIRED
  I18n
  CoreAddons
  WidgetsAddons
)

# use sane compile flags
add_definitions(
  -DQT_USE_QSTRINGBUILDER
  -DQT_NO_CAST_TO_ASCII
  -DQT_NO_CAST_FROM_ASCII
  -DQT_STRICT_ITERATORS
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_NO_CAST_FROM_BYTEARRAY
  -DQT_NO_CAST_TO_BYTEARRAY
  -DQT_NO_SIGNALS_SLOTS_KEYWORDS
  -DQT_USE_FAST_OPERATOR_PLUS
)
kde_enable_exceptions()


find_package(PkgConfig REQUIRED)
pkg_check_modules(BLKID REQUIRED blkid>=2.30)
pkg_check_modules(LIBATASMART REQUIRED libatasmart)

include_directories(${Qt5Core_INCLUDE_DIRS} ${UUID_INCLUDE_DIRS} ${BLKID_INCLUDE_DIRS} lib/ src/)

add_subdirectory(src)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(INCLUDE_INSTALL_DIR "include/kpmcore/")
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KPMcore")

ecm_configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KPMcoreConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KPMcoreConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS INCLUDE_INSTALL_DIR
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KPMcoreConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KPMcoreConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KPMcoreTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KPMcoreTargets.cmake )

ki18n_install(po)

set_target_properties( kpmcore
  PROPERTIES VERSION ${VERSION} SOVERSION ${SOVERSION}
)

message(STATUS "kpmcore ${VERSION} will be built for install into ${CMAKE_INSTALL_PREFIX}")

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

enable_testing()
add_subdirectory(test)
