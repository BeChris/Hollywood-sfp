#!/bin/sh
#This script can be run under Linux to launch (using wine) msvc compiler from Windows 7 SDK
#To install the Windows 7 SDK run following command:
#WINEPREFIX=$HOME/.local/share/wineprefixes/psdkwin7 winetricks -q psdkwin71

WINE=${WINE:-wine}
WINEPREFIX=$HOME/.local/share/wineprefixes/psdkwin7
export WINEPREFIX
PROGRAMFILES="c:\Program Files"
PROGRAMFILESX86="c:\Program Files (x86)"
WSDK="$PROGRAMFILESX86\Microsoft Visual Studio 10.0"
WPSDK="$PROGRAMFILES\Microsoft SDKs\Windows\v7.1"
export WINEPATH="c:\windows;c:\windows\system32;$WSDK\Common7\IDE;$WSDK\VC\bin"
export INCLUDE="$WSDK\VC\include;$WPSDK\Include"
export LIB="$WSDK\VC\lib;$WPSDK\Lib"
$WINE cl.exe $@

