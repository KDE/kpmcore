# SPDX-FileCopyrightText: 2017 Adriaan de Groot <groot@kde.org>
# SPDX-FileCopyrightText: 2017-2019 Andrius Štikonas <andrius@stikonas.eu>
# SPDX-FileCopyrightText: 2018 Caio Carvalho <caiojcarvalho@gmail.com>
# SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>

# SPDX-License-Identifier: GPL-3.0-or-later

# Tests for KPMcore
#
# These are not so much "tests" as "small example programs". They illustrate
# how to use the library, how to perform common tasks.

set(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_compile_options(-fPIC)

###
#
# Helper macro to link to the helper (for initialization of kpmcore)
# and to add a test with the given name.
#
add_library(testhelpers STATIC helpers.cpp)
target_link_libraries(testhelpers PRIVATE kpmcore)

macro (kpm_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} testhelpers kpmcore Qt${QT_MAJOR_VERSION}::Core)
endmacro()

###
#
# Tests of initialization: try explicitly loading some backends
kpm_test(testinit testinit.cpp)  # Default backend
if(TARGET pmdummybackendplugin)
    add_test(NAME testinit-dummy COMMAND testinit $<TARGET_FILE_NAME:pmdummybackendplugin>)
endif()
if(TARGET pmsfdiskbackendplugin)
    add_test(NAME testinit-sfdisk COMMAND testinit $<TARGET_FILE_NAME:pmsfdiskbackendplugin>)
else()
    return()  # All the rest really needs a working backend
endif()

set(BACKEND $<TARGET_FILE_NAME:pmsfdiskbackendplugin>)

###
#
# Listing devices, partitions
kpm_test(testlist testlist.cpp)
add_test(NAME testlist COMMAND testlist ${BACKEND})

kpm_test(testdevicescanner testdevicescanner.cpp)
add_test(NAME testdevicescanner COMMAND testdevicescanner ${BACKEND})

find_package (Threads)
###
#
# Execute external commands as root
kpm_test(testexternalcommand testexternalcommand.cpp)
add_test(NAME testexternalcommand COMMAND testexternalcommand ${BACKEND})


# Test Device
kpm_test(testdevice testdevice.cpp)
add_test(NAME testdevice COMMAND testdevice ${BACKEND})
