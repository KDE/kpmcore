/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2019 Albert Astals Cid <aacid@kde.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KPMCORE_PARTWIDGET_H
#define KPMCORE_PARTWIDGET_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"
#include "partwidgetbase.h"

#include <QColor>

class Partition;

class QPaintEvent;
class QResizeEvent;

/** Widget that represents a Partition.

    Represents a single Partition (possibly with its children, in case of an extended Partition) in the GUI.

    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT PartWidget : public PartWidgetBase
{
    Q_OBJECT

public:
    explicit PartWidget(QWidget* parent, Partition* p = nullptr);

    void init(Partition* p);
    void setActive(bool b) {
        m_Active = b;
    }
    bool isActive() const {
        return m_Active;    /**< @return true if this is the currently active widget */
    }
    void updateChildren();

    Partition* partition() {
        return m_Partition;    /**< @return the widget's Partition */
    }

    const Partition* partition() const {
        return m_Partition;    /**< @return the widget's Partition */
    }

    void setFileSystemColorCode( const std::vector<QColor>& colorCode );

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    QColor activeColor(const QColor& col) const;

    void drawGradient(QPainter* painter, const QColor& color, const QRect& rect, bool active = false) const;

private:
    Partition* m_Partition;
    bool m_Active;
    std::vector<QColor> m_fileSystemColorCode;
};

#endif

