<!-- SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
     SPDX-FileCopyrightText: 2015-2019 Andrius Štikonas <andrius@stikonas.eu>
     SPDX-License-Identifier: CC-BY-4.0
-->

Building and installing KDE Partition Manager Core Library from source
=========================================================

## Dependencies

* [util-linux](https://github.com/karelzak/util-linux) 2.34

* [Qt](https://www.qt.io/) 5.10

* Tier 2 [KDE Frameworks](https://www.kde.org/products/frameworks/) 5.56

## Configure

KPMcore is built with [cmake](https://cmake.org/). It is recommended to build out of tree:
After unpacking the source, create a separate build directory and run cmake there:

```bash
$ tar xf kpmcore-x.y.z.tar.xz
$ cd kpmcore-x.y.z
$ mkdir build
$ cd build
$ cmake ..
```

If all dependencies are met, cmake configures the build directory.


## Build and install

Just run make and make install in the build directory. The default install path
is `/usr/local`, so installing will need write privileges there. You can
configure a different install path by passing
`-DCMAKE_INSTALL_PREFIX=<your_path>` to cmake when configuring. To change the
install path after configuring and building, run

```bash
$ ccmake .
```

in the build directory and modify `CMAKE_INSTALL_PREFIX` there.
