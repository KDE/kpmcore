/*
    SPDX-FileCopyrightText: 2019 Caio Jordão Carvalho <caiojcarvalho@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

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
