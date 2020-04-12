sfp - Get system infos from Hollywood

Hollywood is a commercial multimedia-oriented programming language that can be used to create applications and games very easily (https://hollywood-mal.com/)

This plugin exposes one new function to Hollywood scripts : sfp.SysInfo()

/* This function returns a table containing following subtables:
** 1)cpu table : everything about CPU model identification, capabilities (MMX, SSE, ...), caches size, frequencies
** 2)sys table : depends on operating system and can returns informations such as computer brand, bios version, motherboard and bios serial number, ...
** (under Linux motherboard and bios serial number are fetched only if application is launched with root privileges)
*/


How to compile:
==============
1)Eventually edit build.ini
2)Execute python genbuildfiles.py (generates all the build.* files)
3)Execute one of ./compile_(linux|linux64|mos|...).sh of ./compile_all.sh (invoke all compile_*.sh)
(It uses Ninja build tool instead of Make : https://ninja-build.org/)


Now I explain how I cross compile all plugin.hwp from a Linux Manjaro 64 bits system.

For AROS i386:
=============
git clone https://github.com/aros-development-team/AROS.git
cd AROS
mkdir contrib
mkdir ports
git clone https://github.com/aros-development-team/contrib.git
git clone https://github.com/aros-development-team/ports.git
git submodule update --init --recursive
./configure --prefix=/opt/i386-aros --target=pc-i386
make
cd bin/linux-x86_64/tools/crosstools
mkdir /opt/i386-aros/bin
cp -r * /opt/i386-aros/bin/


For Windows 32 bits:
===================
wine must be installed (currently I have the very last wine 5.4)
winetricks must also be installed

then (to be done only once):
WINEPREFIX=<prefix> winetricks -q psdkwin7

Then execute compile_windows.sh script everytime to compile all source files and generate the final plugin.hwp in build/win32/


For Windows 64 bits:
===================
TODO


For MacOS 64 and 32 bits (in a VM):
==================================
Install virtualbox at least 6.1.4

Then, in a VM, install MacOS following guide at : https://github.com/myspaghetti/macos-guest-virtualbox

Note : To install HighSierra instead of Catalina, I modified in macos-guest-virtualbox.sh:
macOS_release_name="Catalina"
by:
macOS_release_name="HighSierra"

Then, after having launched the script:
Upon "Press enter when the Terminal command prompt is ready." prompt appears I pressed CTRL+C to interrupt it (the installation continue without issues in the vm).

Then within the vm, open a Terminal and enter:
gcc

The update manager will propose to install XCode or developper command line tools : select installation for developper command line tools.

Then, for all compilations, change to the plugin source directory and enter:
make -f makefile.macos64 and/or make -f makefile.macos
=> plugin.hwp will be generated in build/macos64 and/or in build/macos.


For MacOS 64 and 32 bits (cross compiler in Linux):
==================================================
1)Log in to a MacOS system where XCode is installed
2)git clone https://github.com/tpoechtrager/osxcross.git
3)cd osxcross
4)./tools/gen_sdk_package.sh
5)=> copy generated MacOSX*.tar.bz2 to linux
6)Log in to the Linux host system
7)git clone https://github.com/tpoechtrager/osxcross.git
8)cd osxcross
10)mkdir /opt/i386-macos
9)TARGET_DIR=/opt/i386-macos ./build.sh
