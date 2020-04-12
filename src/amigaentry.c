#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <exec/exec.h>
#include <exec/types.h>
#include <proto/exec.h>

#include "version.h"
#include "purefuncs.h"

#include <hollywood/plugin.h>

#if defined(HW_MORPHOS) || defined(HW_AMIGAOS3)
struct ExecBase *SysBase = NULL;
#endif

#ifdef HW_AMIGAOS3

#ifdef __VBCC__
#ifdef HW_FPU
extern ULONG _fpu_init(VOID);
#else
struct Library *MathIeeeSingBasBase = NULL;
struct Library *MathIeeeDoubBasBase = NULL;
struct Library *MathIeeeDoubTransBase = NULL;
#endif
#else
struct Library *__MathIeeeSingBasBase = NULL;
struct Library *__MathIeeeDoubBasBase = NULL;
struct Library *__MathIeeeDoubTransBase = NULL;
struct Library *__UtilityBase = NULL;
#endif

#endif

#ifdef HW_AMIGAOS4
void _start(void)
{
}
#endif

static const struct
{
	STRPTR name;
	void *func;
} funcs[] =
{
	{"InitPlugin", (void *) InitPlugin},
	{"ClosePlugin", (void *) ClosePlugin},
	{"GetBaseTable", (void *) GetBaseTable},
	{"GetCommands", (void *) GetCommands},
	{"GetConstants", (void *) GetConstants},
	{"InitLibrary", (void *) InitLibrary},
	{"FreeLibrary", (void *) FreeLibrary},
	{NULL, NULL}
};

HW_EXPORT void *GetProcAddress(STRPTR name)
{
	int k;

	for(k = 0; funcs[k].name; k++) {
		if(!pure_strcmp(name, funcs[k].name)) return funcs[k].func;
	}

	return NULL;
}

const hwAmigaEntry entry = {
	{HWPLUG_COOKIE1, HWPLUG_COOKIE2},
	PLUGIN_ARCH,
	GetProcAddress,
};

int initamigastuff(void)
{
#if defined(HW_MORPHOS) || defined(HW_AMIGAOS3)
	// on MorphOS we require SysBase for memcpy()
	SysBase = *((struct ExecBase **) 4);
#endif

#ifdef HW_AMIGAOS3

#ifdef __VBCC__

#ifdef HW_FPU
	if(!(SysBase->AttnFlags & AFF_68881) && !(SysBase->AttnFlags & AFF_FPU40)) return 0;
//	  Supervisor(_fpu_init);
#else
	MathIeeeSingBasBase = OpenLibrary("mathieeesingbas.library", 0);
	MathIeeeDoubBasBase = OpenLibrary("mathieeedoubbas.library", 0);
	MathIeeeDoubTransBase = OpenLibrary("mathieeedoubtrans.library", 0);

	if(!(MathIeeeSingBasBase && MathIeeeDoubBasBase && MathIeeeDoubTransBase)) {
		if(MathIeeeDoubTransBase) CloseLibrary(MathIeeeDoubTransBase);
		if(MathIeeeSingBasBase) CloseLibrary(MathIeeeSingBasBase);
		if(MathIeeeDoubBasBase) CloseLibrary(MathIeeeDoubBasBase);
		return 0;
	}
#endif

#else
	__MathIeeeSingBasBase = OpenLibrary("mathieeesingbas.library", 0);
	__MathIeeeDoubBasBase = OpenLibrary("mathieeedoubbas.library", 0);
	__MathIeeeDoubTransBase = OpenLibrary("mathieeedoubtrans.library", 0);
	__UtilityBase = OpenLibrary("utility.library", 0);

	if(!(__MathIeeeSingBasBase && __MathIeeeDoubBasBase && __MathIeeeDoubTransBase && __UtilityBase)) {
		if(__MathIeeeDoubTransBase) CloseLibrary(__MathIeeeDoubTransBase);
		if(__MathIeeeSingBasBase) CloseLibrary(__MathIeeeSingBasBase);
		if(__MathIeeeDoubBasBase) CloseLibrary(__MathIeeeDoubBasBase);
		if(__UtilityBase) CloseLibrary(__UtilityBase);
		return 0;
	}
#endif

#endif

	return 1;
}

void freeamigastuff(void)
{
#ifdef HW_AMIGAOS3

#ifdef __VBCC__
#ifndef HW_FPU
	if(MathIeeeSingBasBase) CloseLibrary(MathIeeeSingBasBase);
	if(MathIeeeDoubBasBase) CloseLibrary(MathIeeeDoubBasBase);
	if(MathIeeeDoubTransBase) CloseLibrary(MathIeeeDoubTransBase);
#endif
#else
	if(__MathIeeeSingBasBase) CloseLibrary(__MathIeeeSingBasBase);
	if(__MathIeeeDoubBasBase) CloseLibrary(__MathIeeeDoubBasBase);
	if(__MathIeeeDoubTransBase) CloseLibrary(__MathIeeeDoubTransBase);
	if(__UtilityBase) CloseLibrary(__UtilityBase);
#endif

#endif
}

