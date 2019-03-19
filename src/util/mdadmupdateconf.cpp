/*************************************************************************
 *  Copyright (C) 2019 by Caio Carvalho <caiojcarvalho@gmail.com>        *
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

#include <cstdlib>
#include <string>

using std::string;
using std::system;

/* 
 *  Use this helper application to append content in the software raid configuration file or to rewrite it.
 *  Only rewrite if you want to replace *ALL* data.
 *
 *  HOWTO:
 *      kpmcore_mdadmupdateconf --append [path] [raid configuration file path]
 *      kpmcore_mdadmupdateconf --write [path] [raid configuration file path]
 * 
*/ 

int main(int argc, char *argv[])
{
    if (argc != 4)
        return 1;
    
    string command = "";

    if (string(argv[1]) == "--append")
        command = "mdadm --detail --scan " + string(argv[2]) + " >> " + string(argv[3]);
    else if (string(argv[1]) == "--write")
        command = "echo " + string(argv[2]) + " > " + string(argv[3]);
    else
        return 1;

    return command != "" ? system(command.c_str()) : 1;
}
