/*************************************************************************
 *  Copyright (C) 2017 by Andrius Štikonas <andrius@stikonas.eu>         *
 *                                                                       *
 *  This program is free software; you can redistribute it and/or        *
 *  modify it under the terms of the GNU General Public License as       *
 *  published by the Free Software Foundation; either version 3 of       *
 *  the License, or (at your option) any later version.                  *
 *                                                                       *
 *  This program is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 *************************************************************************/

#include "plugins/sfdisk/sfdiskpartition.h"
#include "plugins/sfdisk/sfdiskbackend.h"

#include "util/report.h"

SfdiskPartition::SfdiskPartition() :
    CoreBackendPartition()
{
}

bool SfdiskPartition::setFlag(Report& report, PartitionTable::Flag partitionManagerFlag, bool state)
{
    Q_UNUSED(report);
    Q_UNUSED(partitionManagerFlag);
    Q_UNUSED(state);

    return true;
}
