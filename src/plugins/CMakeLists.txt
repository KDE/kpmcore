# SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
# SPDX-FileCopyrightText: 2018 Andrius Štikonas <andrius@stikonas.eu>

# SPDX-License-Identifier: GPL-3.0-or-later

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    option(PARTMAN_SFDISKBACKEND "Build the sfdisk backend plugin." ON)

    if (PARTMAN_SFDISKBACKEND)
        add_subdirectory(sfdisk)
    endif (PARTMAN_SFDISKBACKEND)
endif()

option(PARTMAN_DUMMYBACKEND "Build the dummy backend plugin." ON)

if (PARTMAN_DUMMYBACKEND)
    add_subdirectory(dummy)
endif (PARTMAN_DUMMYBACKEND)
