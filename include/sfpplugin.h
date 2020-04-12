/*
** SFP (SysFootPrint) Hollywood plugin
** Copyright (C) 2020 Christophe Gouiran <bechris13250@gmail.com>
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifdef HW_AMIGA
int initamigastuff(void);
void freeamigastuff(void);
#endif

void fill_systable(void *state);

#define hw_AddPart hwcl->DOSBase->hw_AddPart
#define hw_BeginDirScan hwcl->DOSBase->hw_BeginDirScan
#define hw_NextDirEntry hwcl->DOSBase->hw_NextDirEntry
#define hw_EndDirScan hwcl->DOSBase->hw_EndDirScan
#define hw_Lock hwcl->DOSBase->hw_Lock
#define hw_UnLock hwcl->DOSBase->hw_UnLock
#define hw_FOpen hwcl->DOSBase->hw_FOpen
#define hw_FClose hwcl->DOSBase->hw_FClose
#define hw_FRead hwcl->DOSBase->hw_FRead
#define hw_FStat hwcl->DOSBase->hw_FStat
#define hw_SetErrorString hwcl->SysBase->hw_SetErrorString
#define luaL_checkfilename hwcl->LuaBase->luaL_checkfilename
#define lua_newtable hwcl->LuaBase->lua_newtable
#define lua_pushboolean hwcl->LuaBase->lua_pushboolean 
#define lua_pushnumber hwcl->LuaBase->lua_pushnumber
#define lua_pushstring hwcl->LuaBase->lua_pushstring
#define lua_rawset hwcl->LuaBase->lua_rawset
