# Copyright (C) 2010 by Volker Lanz <vl@fidra.de>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

set (pmsfdiskbackendplugin_SRCS
    sfdiskbackend.cpp
    sfdiskdevice.cpp
    sfdiskpartitiontable.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/corebackenddevice.cpp
)

add_library(pmsfdiskbackendplugin SHARED ${pmsfdiskbackendplugin_SRCS})

target_link_libraries(pmsfdiskbackendplugin kpmcore KF5::I18n KF5::CoreAddons)

install(TARGETS pmsfdiskbackendplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})
