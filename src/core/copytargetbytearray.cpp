/*
    SPDX-FileCopyrightText: 2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core/copytargetbytearray.h"

/** This class is for reading disk data into QByteArray
    It is only suitable for reading small amount of data such as GPT header or
    FAT boot sector. DBus is too slow for copying data of the whole partition.
    @param QByteArray to write to
*/
CopyTargetByteArray::CopyTargetByteArray(QByteArray& array) :
    CopyTarget(),
    m_Array(array)
{
}
