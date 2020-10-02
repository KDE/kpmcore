/*
    SPDX-FileCopyrightText: 2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_OPERATION_P_H
#define KPMCORE_OPERATION_P_H

#include "ops/operation.h"

class OperationPrivate
{
public:
    Operation::OperationStatus m_Status;
    QList<Job*> m_Jobs;
    qint32 m_ProgressBase;
};

#endif
