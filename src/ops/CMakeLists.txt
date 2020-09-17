# SPDX-FileCopyrightText: 2008,2012 Volker Lanz <vl@fidra.de>
# SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
# SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>

# SPDX-License-Identifier: GPL-3.0-or-later

set(OPS_SRC
    ops/operation.cpp
    ops/deleteoperation.cpp
    ops/restoreoperation.cpp
    ops/resizeoperation.cpp
    ops/newoperation.cpp
    ops/createfilesystemoperation.cpp
    ops/createpartitiontableoperation.cpp
    ops/createvolumegroupoperation.cpp
    ops/removevolumegroupoperation.cpp
    ops/deactivatevolumegroupoperation.cpp
    ops/resizevolumegroupoperation.cpp
    ops/setfilesystemlabeloperation.cpp
    ops/setpartflagsoperation.cpp
    ops/checkoperation.cpp
    ops/backupoperation.cpp
    ops/copyoperation.cpp
)

set(OPS_LIB_HDRS
    ops/backupoperation.h
    ops/checkoperation.h
    ops/copyoperation.h
    ops/createfilesystemoperation.h
    ops/createpartitiontableoperation.h
    ops/createvolumegroupoperation.h
    ops/removevolumegroupoperation.h
    ops/deactivatevolumegroupoperation.h
    ops/resizevolumegroupoperation.h
    ops/deleteoperation.h
    ops/newoperation.h
    ops/operation.h
    ops/resizeoperation.h
    ops/restoreoperation.h
    ops/setfilesystemlabeloperation.h
    ops/setpartflagsoperation.h
)
