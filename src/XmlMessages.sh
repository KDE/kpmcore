# SPDX-FileCopyrightText: 2020 Andrius Štikonas <andrius@stikonas.eu>
# SPDX-License-Identifier: MIT

function get_files
{
    echo util/org.kde.kpmcore.externalcommand.policy
}

function po_for_file
{
    case "$1" in
       util/org.kde.kpmcore.externalcommand.policy)
           echo kpmcore._policy_.po
       ;;
    esac
}

function tags_for_file
{
    case "$1" in
       util/org.kde.kpmcore.externalcommand.policy)
           echo description
           echo message
       ;;
    esac
}
