# SPDX-FileCopyrightText: 2008,2012 Volker Lanz <vl@fidra.de>
# SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
# SPDX-FileCopyrightText: 2017 Andrius Štikonas <andrius@stikonas.eu>
# SPDX-FileCopyrightText: 2018 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

# SPDX-License-Identifier: GPL-3.0-or-later

include(core/raid/CMakeLists.txt)

set(CORE_SRC
    core/copysource.cpp
    core/copysourcedevice.cpp
    core/copysourcefile.cpp
    core/copysourceshred.cpp
    core/copytarget.cpp
    core/copytargetbytearray.cpp
    core/copytargetdevice.cpp
    core/copytargetfile.cpp
    core/device.cpp
    core/devicescanner.cpp
    core/diskdevice.cpp
    core/fstab.cpp
    core/lvmdevice.cpp
    core/operationrunner.cpp
    core/operationstack.cpp
    core/partition.cpp
    core/partitionalignment.cpp
    core/partitionnode.cpp
    core/partitionrole.cpp
    core/partitiontable.cpp
    core/smartstatus.cpp
    core/smartattribute.cpp
    core/smartparser.cpp
    core/smartattributeparseddata.cpp
    core/smartdiskinformation.cpp
    core/volumemanagerdevice.cpp
    ${RAID_SRC}
)

set(CORE_LIB_HDRS
    core/device.h
    core/devicescanner.h
    core/diskdevice.h
    core/fstab.h
    core/lvmdevice.h
    core/operationrunner.h
    core/operationstack.h
    core/partition.h
    core/partitionalignment.h
    core/partitionnode.h
    core/partitionrole.h
    core/partitiontable.h
    core/smartattribute.h
    core/smartstatus.h
    core/volumemanagerdevice.h
    ${RAID_LIB_HDRS}
)
