# SPDX-FileCopyrightText: 2008,2012 Volker Lanz <vl@fidra.de>
# SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>
# SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
# SPDX-FileCopyrightText: 2020 Gaël PORTAY <gael.portay@collabora.com>

# SPDX-License-Identifier: GPL-3.0-or-later

set(JOBS_SRC
    jobs/resizefilesystemjob.cpp
    jobs/createfilesystemjob.cpp
    jobs/job.cpp
    jobs/checkfilesystemjob.cpp
    jobs/shredfilesystemjob.cpp
    jobs/createpartitionjob.cpp
    jobs/createpartitiontablejob.cpp
    jobs/setpartitionlabeljob.cpp
    jobs/setpartitionuuidjob.cpp
    jobs/setpartitionattributesjob.cpp
    jobs/createvolumegroupjob.cpp
    jobs/removevolumegroupjob.cpp
    jobs/deactivatevolumegroupjob.cpp
    jobs/deactivatelogicalvolumejob.cpp
    jobs/resizevolumegroupjob.cpp
    jobs/movephysicalvolumejob.cpp
    jobs/setfilesystemlabeljob.cpp
    jobs/deletepartitionjob.cpp
    jobs/restorefilesystemjob.cpp
    jobs/setpartgeometryjob.cpp
    jobs/deletefilesystemjob.cpp
    jobs/backupfilesystemjob.cpp
    jobs/setpartflagsjob.cpp
    jobs/copyfilesystemjob.cpp
    jobs/movefilesystemjob.cpp
    jobs/changepermissionsjob.cpp
)

set(JOBS_LIB_HDRS
    jobs/job.h
)
