/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2016-2017 Andrius Štikonas <andrius@stikonas.eu

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_PARTWIDGETBASE_H
#define KPMCORE_PARTWIDGETBASE_H

#include "util/libpartitionmanagerexport.h"

#include "core/partitionnode.h"

#include <QList>
#include <QWidget>

class Partition;
class PartWidget;
class QWidget;

bool distributeLostPixels(QList<qint32>& childrenWidth, qint32 lostPixels);
bool levelChildrenWidths(QList<qint32>& childrenWidth, const QList<qint32>& minChildrenWidth, const qint32 destWidgetWidth);

/** Base class for all widgets that need to position Partitions.
    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT PartWidgetBase : public QWidget
{
    Q_DISABLE_COPY(PartWidgetBase)

protected:
    PartWidgetBase(QWidget* parent) : QWidget(parent) {}
    ~PartWidgetBase() override {}

public:
    virtual qint32 borderWidth() const {
        return m_BorderWidth;    /**< @return border width */
    }
    virtual qint32 borderHeight() const {
        return m_BorderHeight;    /**< @return border height */
    }
    static qint32 spacing() {
        return m_Spacing;    /**< @return spacing between Partitions */
    }
    static qint32 minWidth() {
        return m_MinWidth;    /**< @return minimum width for a Partition widget */
    }

    virtual const QList<PartWidget*> childWidgets() const;

protected:
    virtual void positionChildren(const QWidget* destWidget, const PartitionNode::Partitions& partitions, QList<PartWidget*> widgets) const;

private:
    static const qint32 m_Spacing;
    static const qint32 m_BorderWidth;
    static const qint32 m_BorderHeight;
    static const qint32 m_MinWidth;
};

#endif

