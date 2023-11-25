# SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
# SPDX-FileCopyrightText: 2018 Andrius Štikonas <andrius@stikonas.eu>

# SPDX-License-Identifier: GPL-3.0-or-later

function(kpmcore_add_plugin name)
    kcoreaddons_add_plugin(${name} INSTALL_NAMESPACE "kpmcore")
endfunction()


if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    option(PARTMAN_SFDISKBACKEND "Build the sfdisk backend plugin." ON)

    if (PARTMAN_SFDISKBACKEND)
        add_subdirectory(sfdisk)
    endif (PARTMAN_SFDISKBACKEND)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    option(PARTMAN_GPARTBACKEND "Build the gpart backend plugin." ON)
endif()

option(PARTMAN_DUMMYBACKEND "Build the dummy backend plugin." ON)

if (PARTMAN_GPARTBACKEND)
    add_subdirectory(gpart)
endif (PARTMAN_GPARTBACKEND)

if (PARTMAN_DUMMYBACKEND)
    add_subdirectory(dummy)
endif (PARTMAN_DUMMYBACKEND)
