# SPDX-FileCopyrightText: 2008 Volker Lanz <vl@fidra.de>
# SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
# SPDX-FileCopyrightText: 2014-2022 Andrius Štikonas <andrius@stikonas.eu>
# SPDX-FileCopyrightText: 2020 David Edmundson <kde@davidedmundson.co.uk>

# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "22")
set (RELEASE_SERVICE_VERSION_MINOR "12")
set (RELEASE_SERVICE_VERSION_MICRO "3")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
project(kpmcore VERSION ${RELEASE_SERVICE_VERSION})

set(SOVERSION "12")
add_definitions(-D'VERSION="${RELEASE_SERVICE_VERSION}"') #"

set(CMAKE_USE_RELATIVE_PATHS OFF)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

# Note that KPMcore is restricted to only run binaries installed into trusted prefixes
# See src/util/trustedprefixes
# By default this is set to / and /usr which is good for majority of distros

# Dependencies
set(QT_MIN_VERSION "5.15.2")
set(KF5_MIN_VERSION "5.90")
set(BLKID_MIN_VERSION "2.33.2")
# PolkitQt5-1

# Runtime
# smartmontools 7.0

set(KDE_COMPILERSETTINGS_LEVEL "5.85")

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

if (NOT QT_MAJOR_VERSION)
    set(QT_MAJOR_VERSION "5")
endif()

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} COMPONENTS REQUIRED
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

find_package(PolkitQt${QT_MAJOR_VERSION}-1 REQUIRED)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f00)
kde_enable_exceptions()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(BLKID REQUIRED blkid>=${BLKID_MIN_VERSION})
endif()

add_subdirectory(src)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KPMcore")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KPMcoreConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KPMcoreConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
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
