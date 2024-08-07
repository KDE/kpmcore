/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
    SPDX-FileCopyrightText: 2012-2020 Andrius Štikonas <andrius@stikonas.eu
    SPDX-FileCopyrightText: 2015 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2020 David Faure <faure@kde.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/partwidget.h"

#include "core/partition.h"
#include "fs/filesystem.h"
#include "util/capacity.h"

#include <QApplication>
#include <QFontDatabase>
#include <QPainter>
#include <QStyleOptionButton>

/** Creates a new PartWidget
    @param parent pointer to the parent widget
    @param p pointer to the Partition this widget will show. must not be nullptr.
*/
PartWidget::PartWidget(QWidget* parent, Partition* p) :
    PartWidgetBase(parent),
    m_Partition(nullptr),
    m_Active(false)
{
    setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));
    init(p);
    m_fileSystemColorCode = FileSystem::defaultColorCode;
}

void PartWidget::init(Partition* p)
{
    m_Partition = p;

    if (partition())
        setToolTip(partition()->deviceNode() + QStringLiteral("\n") + partition()->fileSystem().name() + QStringLiteral(" ") + QString(Capacity::formatByteSize(partition()->capacity())));
    else
        setToolTip(QString());

    updateChildren();
}

/** Updates the widget's children */
void PartWidget::updateChildren()
{
    if (partition()) {
        for (const auto &w : childWidgets()) {
            w->setVisible(false);
            w->deleteLater();
            w->setParent(nullptr);
        }

        for (const auto &child : partition()->children()) {
            QWidget* w = new PartWidget(this, child);
            w->setVisible(true);
        }

        positionChildren(this, partition()->children(), childWidgets());
    }
}

void PartWidget::setFileSystemColorCode(const std::vector<QColor>& colorCode)
{
    m_fileSystemColorCode = colorCode;
    repaint();
}

void PartWidget::resizeEvent(QResizeEvent*)
{
    if (partition())
        positionChildren(this, partition()->children(), childWidgets());
}

QColor PartWidget::activeColor(const QColor& col) const
{
    return isActive() ? col.darker(190) : col;
}

void PartWidget::paintEvent(QPaintEvent*)
{
    if (partition() == nullptr || partition()->isFileSystemNullptr())
        return;

    auto partitionCapacity = partition()->capacity();
    if (partitionCapacity <= 0)
        return;

    const int usedPercentage = static_cast<int>(partition()->used() * 100 / partitionCapacity);
    const int w = width() * usedPercentage / 100;

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    const QColor base = activeColor(m_fileSystemColorCode[ static_cast<int>(partition()->fileSystem().type()) ]);
    if (partition()->roles().has(PartitionRole::Extended)) {
        drawGradient(&painter, base, QRect(0, 0, width(), height()));
        return;
    }

    if (!partition()->roles().has(PartitionRole::Unallocated)) {
        const QColor dark = base.darker(105);
        const QColor light = base.lighter(120);

        // draw free space background
        drawGradient(&painter, light, QRect(0, 0, width(), height()), isActive());

        // draw used space in front of that
        drawGradient(&painter, dark, QRect(0, 0, w, height() - 1));
    } else
        drawGradient(&painter, base, QRect(0, 0, width(), height()), isActive());

    // draw name and size
    QString text = partition()->deviceNode().remove(QStringLiteral("/dev/")) + QStringLiteral("\n") + QString(Capacity::formatByteSize(partition()->capacity()));

    const QRect textRect(0, 0, width() - 1, height() - 1);
    const QRect boundingRect = painter.boundingRect(textRect, Qt::AlignCenter, text);
    if (boundingRect.x() > PartWidgetBase::borderWidth() && boundingRect.y() > PartWidgetBase::borderHeight()) {
        if (isActive())
            painter.setPen(Qt::white);
        painter.drawText(textRect, Qt::AlignCenter, text);
    }
}

void PartWidget::drawGradient(QPainter* painter, const QColor& color, const QRect& rect, bool active) const
{
    if (rect.width() < 8)
        return;

    QStyleOptionButton option;
    option.initFrom(this);
    option.rect = rect;
    option.palette.setColor(QPalette::Button, color);
    option.palette.setColor(QPalette::Window, color);
    option.state |= QStyle::State_Raised;
    if (!active)
        option.state &= ~QStyle::State_MouseOver;
    else
        option.state |= QStyle::State_MouseOver;

    style()->drawControl(QStyle::CE_PushButtonBevel, &option, painter, this);
}

#include "moc_partwidget.cpp"
