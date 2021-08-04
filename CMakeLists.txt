# SPDX-FileCopyrightText: 2008 Volker Lanz <vl@fidra.de>
# SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
# SPDX-FileCopyrightText: 2014-2020 Andrius Štikonas <andrius@stikonas.eu>
# SPDX-FileCopyrightText: 2020 David Edmundson <kde@davidedmundson.co.uk>

# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "21")
set (RELEASE_SERVICE_VERSION_MINOR "08")
set (RELEASE_SERVICE_VERSION_MICRO "0")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
project(kpmcore VERSION ${RELEASE_SERVICE_VERSION})

set(SOVERSION "11")
add_definitions(-D'VERSION="${RELEASE_SERVICE_VERSION}"') #"


set(CMAKE_USE_RELATIVE_PATHS OFF)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

# Dependencies
set(QT_MIN_VERSION "5.14.0")
set(KF5_MIN_VERSION "5.73")
set(BLKID_MIN_VERSION "2.33.2")
# PolkitQt5-1

# Runtime
# smartmontools 7.0

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)
include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMConfiguredInstall)

ecm_setup_version(${RELEASE_SERVICE_VERSION} VARIABLE_PREFIX KPMCORE
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
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED
  CoreAddons
  I18n
  WidgetsAddons
)

find_package(PolkitQt5-1 REQUIRED)

# use sane compile flags
add_definitions(
  -DQT_USE_QSTRINGBUILDER
  -DQT_NO_CAST_TO_ASCII
  -DQT_NO_CAST_FROM_ASCII
  -DQT_STRICT_ITERATORS
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_NO_CAST_FROM_BYTEARRAY
  -DQT_NO_CAST_TO_BYTEARRAY
  -DQT_USE_FAST_OPERATOR_PLUS
  -DQT_NO_KEYWORDS
)
kde_enable_exceptions()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(BLKID REQUIRED blkid>=${BLKID_MIN_VERSION})
endif()

add_subdirectory(src)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(INCLUDE_INSTALL_DIR "include/kpmcore/")
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KPMcore")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KPMcoreConfig.cmake.in"
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
  PROPERTIES VERSION ${RELEASE_SERVICE_VERSION} SOVERSION ${SOVERSION}
)

message(STATUS "kpmcore ${RELEASE_SERVICE_VERSION} will be built for install into ${CMAKE_INSTALL_PREFIX}")

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

enable_testing()
add_subdirectory(test)
