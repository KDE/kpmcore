/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2015 Chris Campbell <c.j.campbell@ed.ac.uk>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_PARTITIONNODE_H
#define KPMCORE_PARTITIONNODE_H

#include "util/libpartitionmanagerexport.h"

#include <QObject>
#include <QList>
#include <QtGlobal>

class Partition;
class PartitionRole;

/** A node in the tree of partitions.

    The root in this tree is the PartitionTable. The primaries are the child nodes; extended partitions again
    have child nodes.

    @see Device, PartitionTable, Partition
    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT PartitionNode : public QObject
{
public:
    typedef QList<Partition*> Partitions;

protected:
    PartitionNode() {}
    ~PartitionNode() override {}

public:
    virtual bool insert(Partition* partNew);

    virtual Partition* predecessor(Partition& p);
    virtual const Partition* predecessor(const Partition& p) const;

    virtual Partition* successor(Partition& p);
    virtual const Partition* successor(const Partition& p) const;

    virtual bool remove(Partition* p);
    virtual Partition* findPartitionBySector(qint64 s, const PartitionRole& role);
    virtual const Partition* findPartitionBySector(qint64 s, const PartitionRole& role) const;
    virtual void reparent(Partition& p);

    virtual Partitions& children() = 0;
    virtual PartitionNode* parent() = 0;
    virtual bool isRoot() const = 0;
    virtual const PartitionNode* parent() const = 0;
    virtual const Partitions& children() const = 0;
    virtual void append(Partition* p) = 0;
    virtual qint32 highestMountedChild() const;
    virtual bool isChildMounted() const;

protected:
    virtual void clearChildren();
};

#endif
