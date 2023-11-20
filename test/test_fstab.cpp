/*
    SPDX-FileCopyrightText: 2023 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <QObject>

#include <QtTest>

#include "core/fstab.h"

using namespace Qt::StringLiterals;

struct ExpectedEntry {
    bool comment = false;
    QString device = QString();
    QString mountPoint = QString();
    QString type = QString();
};

class FsTabTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void initTestCase()
    {

    }

    void testReadFstab()
    {
        auto fstabFile = QFINDTESTDATA("test_fstab_data/fstab");
        auto entries = readFstabEntries(fstabFile);

        std::array<ExpectedEntry, 13> expected_entries = {{
            {.comment = true},
            {.comment = true},
            {.comment = true},
            {.comment = true},
            {.device = u"/dev/sda"_s, .mountPoint = u"/"_s, .type = u"ext4"_s },
            {.comment = true},
            {.device = u""_s, .mountPoint = u"/test"_s, .type = u"btrfs"_s},
            {.comment = true},
            {.device = u"127.0.0.1:/nfs/export"_s, .mountPoint = u"/nfs"_s, .type = u"nfs"_s},
            {.comment = true},
            {.device = u"none"_s, .mountPoint = u"/tmp"_s, .type = u"tmpfs"_s},
            {.comment = true},
            {.device = u"/dev/mapper/swap"_s, .mountPoint = u"none"_s, .type = u"swap"_s},
        }};

        QCOMPARE(entries.size(), expected_entries.size());

        for (int i = 0; i < entries.size(); ++i) {
            auto entry = entries.at(i);
            auto expected = expected_entries.at(i);

            if (expected.comment) {
                QVERIFY(!entry.comment().isNull());
                continue;
            }

            QCOMPARE(entry.deviceNode(), expected.device);
            QCOMPARE(entry.mountPoint(), expected.mountPoint);
            QCOMPARE(entry.type(), expected.type);
        }
    }

    void testWriteFstab()
    {
        auto entries = FstabEntryList{{
            {{}, {}, {}, {}, {}, {}, u"# Static information about the filesystems."_s},
            {{}, {}, {}, {}, {}, {}, u"# See fstab(5) for details."_s},
            {{}, {}, {}, {}, {}, {}, {}},
            {{}, {}, {}, {}, {}, {}, u"# <file system> <dir> <type> <options> <dump> <pass>"_s},
            {u"/dev/sda"_s, u"/"_s, u"ext4"_s, u"rw,relatime,discard"_s, 0, 1},
            {{}, {}, {}, {}, {}, {}, {}},
            {u"UUID=0491f5bc-487c-4797-b118-78add1e9cfb0"_s, u"/test"_s, u"btrfs"_s, u"defaults"_s, 0, 2},
            {{}, {}, {}, {}, {}, {}, {}},
            {u"127.0.0.1:/nfs/export"_s, u"/nfs"_s, u"nfs"_s, u"defaults"_s, 0, 2},
            {{}, {}, {}, {}, {}, {}, {}},
            {u"none"_s, u"/tmp"_s, u"tmpfs"_s, u"defaults"_s, 0, 0},
            {{}, {}, {}, {}, {}, {}, {}},
            {u"/dev/mapper/swap"_s, u"none"_s, u"swap"_s, u"defaults"_s, 0, 0},
        }};

        auto fstab = generateFstab(entries).toUtf8().split('\n');

        auto fstabFilePath = QFINDTESTDATA("test_fstab_data/fstab");
        QFile fstabFile(fstabFilePath);
        QVERIFY(fstabFile.open(QIODevice::OpenModeFlag::ReadOnly));
        auto fileData = fstabFile.readAll().split('\n');
        fstabFile.close();

        QCOMPARE(fstab, fileData);

        // Test whether an empty mount point for not-swap is properly ignored.
        entries.append({u"/dev/sda"_s, u"none"_s, u"ext4"_s, u"defaults"_s});
        entries.append({u"/dev/sda"_s, QString(), u"ext4"_s, u"defaults"_s});
        fstab = generateFstab(entries).toUtf8().split('\n');

        QCOMPARE(fstab, fileData);
    }
};

QTEST_GUILESS_MAIN(FsTabTest)


#include "test_fstab.moc"
