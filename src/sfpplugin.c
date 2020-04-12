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

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <hollywood/plugin.h>

#include "sfpplugin.h"
#include "version.h"

#ifdef _MSC_VER
#define MSVC_COMPILER 1
#else
#define MSVC_COMPILER 0
#endif

#if MSVC_COMPILER
#  include <intrin.h>
#else
#  include <cpuid.h>
#  include <x86intrin.h>
#endif

#include <stdint.h>

// pointer to the Hollywood plugin API
hwPluginAPI *hwcl = NULL;

static hwPluginBase *hwpb = NULL;

// information about our plugin for InitPlugin()
// (NB: we store the version string after the plugin's name; this is not required by Hollywood;
// it is just a trick to prevent the linker from optimizing our version string away)
static const char plugin_name[] = PLUGIN_NAME "\0$VER: " PLUGIN_MODULENAME ".hwp " PLUGIN_VER_STR " (" PLUGIN_DATE ") [" PLUGIN_PLAT "]";
static const char plugin_modulename[] = PLUGIN_MODULENAME;
static const char plugin_author[] = PLUGIN_AUTHOR;
static const char plugin_description[] = PLUGIN_DESCRIPTION;
static const char plugin_copyright[] = PLUGIN_COPYRIGHT;
static const char plugin_url[] = PLUGIN_URL;
static const char plugin_date[] = PLUGIN_DATE;

// all functions will be added to this table
static const char *basetable = "sfp";

/*
** WARNING: InitPlugin() will be called by *any* Hollywood version >= 5.0. Thus, you must
** check the Hollywood version that called your InitPlugin() implementation before calling
** functions from the hwPluginAPI pointer or accessing certain structure members. Your
** InitPlugin() implementation must be compatible with *any* Hollywood version >= 5.0. If
** you call Hollywood 6.0 functions here without checking first that Hollywood 6.0 or higher
** has called your InitPlugin() implementation, *all* programs compiled with Hollywood
** versions < 6.0 *will* crash when they try to open your plugin! 
*/

HW_EXPORT int InitPlugin(hwPluginBase *self, hwPluginAPI *cl, STRPTR path)
{
	// open Amiga libraries needed by this plugin		
#ifdef HW_AMIGA
	if(!initamigastuff()) return FALSE;
#endif

	// identify as a file plugin to Hollywood
	self->CapsMask = HWPLUG_CAPS_LIBRARY;
	self->Version = PLUGIN_VER;
	self->Revision = PLUGIN_REV;

	// we want to be compatible with Hollywood 6.0
	// **WARNING**: when compiling with newer SDK versions you have to be very
	// careful which functions you call and which structure members you access
	// because not all of them are present in earlier versions. Thus, if you
	// target versions older than your SDK version you have to check the hollywood.h
	// header file very carefully to check whether the older version you want to
	// target has the respective feature or not
	self->hwVersion = 6;
	self->hwRevision = 0;
	
	// set plugin information; note that these string pointers need to stay
	// valid until Hollywood calls ClosePlugin()		
	self->Name = (STRPTR) plugin_name;
	self->ModuleName = (STRPTR) plugin_modulename;	
	self->Author = (STRPTR) plugin_author;
	self->Description = (STRPTR) plugin_description;
	self->Copyright = (STRPTR) plugin_copyright;
	self->URL = (STRPTR) plugin_url;
	self->Date = (STRPTR) plugin_date;
	self->Settings = NULL;
	self->HelpFile = NULL;

	hwpb = self;

	// NB: "cl" can be NULL in case Hollywood or Designer just wants to obtain information
	// about our plugin
	if(cl) {
			
		hwcl = cl;

	}

	return TRUE;
}

/*
** WARNING: ClosePlugin() will be called by *any* Hollywood version >= 5.0.
** --> see the note above in InitPlugin() for information on how to implement this function
*/
HW_EXPORT void ClosePlugin(void)
{
#ifdef HW_AMIGA
	freeamigastuff();
#endif
}

#define EDX_FEATURES_SIZE     29
#define ECX_FEATURES_SIZE     30
#define EBX_EXT_FEATURES_SIZE 30
#define ECX_EXT_FEATURES_SIZE  6
#define ECX_EXT_FUNCTIONS_SIZE  10
#define EDX_EXT_FUNCTIONS_SIZE  8

typedef struct
{
	const char* name;
	int mask;

} feature_t;


const feature_t EdxFeatures[EDX_FEATURES_SIZE] =
{
	{ "FPU",           (1 <<  0) },
	{ "VME",           (1 <<  1) },
	{ "DE",            (1 <<  2) },
	{ "PSE",           (1 <<  3) },
	{ "TSC",           (1 <<  4) },
	{ "MSR",           (1 <<  5) },
	{ "PAE",           (1 <<  6) },
	{ "MCE",           (1 <<  7) },
	{ "CX8",           (1 <<  8) },
	{ "APIC",          (1 <<  9) },
	{ "SEP",           (1 << 11) },
	{ "MTRR",          (1 << 12) },
	{ "PGE",           (1 << 13) },
	{ "MCA",           (1 << 14) },
	{ "CMOV",          (1 << 15) },
	{ "PAT",           (1 << 16) },
	{ "PSE-36",        (1 << 17) },
	{ "PSN",           (1 << 18) },
	{ "CLFSH",         (1 << 19) },
	{ "DS",            (1 << 21) },
	{ "ACPI",          (1 << 22) },
	{ "MMX",           (1 << 23) },
	{ "FXSR",          (1 << 24) },
	{ "SSE",           (1 << 25) },
	{ "SSE2",          (1 << 26) },
	{ "SS",            (1 << 27) },
	{ "HTT",           (1 << 28) },
	{ "TM",            (1 << 29) },
	{ "PBE",           (1 << 31) }
};

const feature_t EcxFeatures[ECX_FEATURES_SIZE] =
{
	{ "SSE3",          (1 <<  0) },
	{ "PCLMULQDQ",     (1 <<  1) },
	{ "DTES64",        (1 <<  2) },
	{ "MONITOR",       (1 <<  3) },
	{ "DS-CPL",        (1 <<  4) },
	{ "VMX",           (1 <<  5) },
	{ "SMX",           (1 <<  6) },
	{ "EIST",          (1 <<  7) },
	{ "TM2",           (1 <<  8) },
	{ "SSSE3",         (1 <<  9) },
	{ "CNXT-ID",       (1 << 10) },
	{ "SDBG",          (1 << 11) },
	{ "FMA",           (1 << 12) },
	{ "CMPXCHG16B",    (1 << 13) },
	{ "XTPR",          (1 << 14) },
	{ "PDCM",          (1 << 15) },
	{ "PCID",          (1 << 17) },
	{ "DCA",           (1 << 18) },
	{ "SSE4.1",        (1 << 19) },
	{ "SSE4.2",        (1 << 20) },
	{ "X2APIC",        (1 << 21) },
	{ "MOVBE",         (1 << 22) },
	{ "POPCNT",        (1 << 23) },
	{ "TSC-DEADLINE",  (1 << 24) },
	{ "AESNI",         (1 << 25) },
	{ "XSAVE",         (1 << 26) },
	{ "OSXSAVE",       (1 << 27) },
	{ "AVX",           (1 << 28) },
	{ "F16C",          (1 << 29) },
	{ "RDRND",         (1 << 30) }
};

const feature_t EbxExtFeatures[EBX_EXT_FEATURES_SIZE] =
{
	{ "FSGSBASE",      (1 <<  0) },
	{ "IA32-TSC-ADJ",  (1 <<  1) },
	{ "SGX",           (1 <<  2) },
	{ "BMI1",          (1 <<  3) },
	{ "HLE",           (1 <<  4) },
	{ "AVX2",          (1 <<  5) },
	{ "FDP-EXCP",      (1 <<  6) },
	{ "SMEP",          (1 <<  7) },
	{ "BMI2",          (1 <<  8) },
	{ "ENHANCED-RMS",  (1 <<  9) },
	{ "INVPCID",       (1 << 10) },
	{ "RTM",           (1 << 11) },
	{ "RDT-M",         (1 << 12) },
	{ "DEPR-FCSDS",    (1 << 13) },
	{ "MPX",           (1 << 14) },
	{ "RDT-A",         (1 << 15) },
	{ "AVX512F",       (1 << 16) },
	{ "AVX512DQ",      (1 << 17) },
	{ "RDSEED",        (1 << 18) },
	{ "ADX",           (1 << 19) },
	{ "SMAP",          (1 << 20) },
	{ "CLFLUSHOPT",    (1 << 23) },
	{ "CLWB",          (1 << 24) },
	{ "INTEL-PTRACE",  (1 << 25) },
	{ "AVX512PF",      (1 << 26) },
	{ "AVX512ER",      (1 << 27) },
	{ "AVX512CD",      (1 << 28) },
	{ "SHA",           (1 << 29) },
	{ "AVX512BW",      (1 << 30) },
	{ "AVX512VL",      (1 << 31) }
};

const feature_t EcxExtFeatures[ECX_EXT_FEATURES_SIZE] =
{
	{ "PREFETCHWT1",   (1 <<  0) },
	{ "UMIP",          (1 <<  2) },
	{ "PKU",           (1 <<  3) },
	{ "OSPKE",         (1 <<  4) },
	{ "RPID",          (1 << 22) },
	{ "SGX-LC",        (1 << 30) }
};

const feature_t EcxExtFunctions[ECX_EXT_FUNCTIONS_SIZE] =
{
	{ "LAHF-SAHF",     (1 <<  0) },
	{ "SVM",           (1 <<  2) },
	{ "LZCNT",         (1 <<  5) },
	{ "SSE4A",         (1 <<  6) },
	{ "PREFETCHW",     (1 <<  8) },
	{ "XOP",           (1 << 11) },
	{ "SKINIT",        (1 << 12) },
	{ "FMA4",          (1 << 16) },
	{ "TBM",           (1 << 21) },
	{ "MONITORX",      (1 << 29) }
};

const feature_t EdxExtFunctions[EDX_EXT_FUNCTIONS_SIZE] =
{
	{ "SYSCALL-SYSRET",(1 << 11) },
	{ "EXD",           (1 << 20) },
	{ "MMX_ext",       (1 << 22) },
	{ "1GB_pages",     (1 << 26) },
	{ "RDTSCP",        (1 << 27) },
	{ "64_bit_mode",   (1 << 29) },
	{ "3DNow_ext",     (1 << 30) },
	{ "3DNow",         (1 << 31) }
};

const char* CacheTlbDescriptors[256] =
{
// 0x00
	NULL,
	"Instruction TLB: 4 KByte pages, 4-way set associative, 32 entries",
	"Instruction TLB: 4 MByte pages, fully associative, 2 entries",
	"Data TLB: 4 KByte pages, 4-way set associative, 64 entries",
	"Data TLB: 4 MByte pages, 4-way set associative, 8 entries",
	"Data TLB1: 4 MByte pages, 4-way set associative, 32 entries",
	"1st-level instruction cache: 8 KBytes, 4-way set associative, 32 byte line size",
	"1st-level instruction cache: 16 KBytes, 4-way set associative, 32 byte line size",
	"1st-level instruction cache: 32KBytes, 4-way set associative, 64 byte line size",
	"1st-level data cache: 8 KBytes, 2-way set associative, 32 byte line size",
	"Instruction TLB: 4 MByte pages, 4-way set associative, 4 entries",
	"1st-level data cache: 16 KBytes, 4-way set associative, 32 byte line size",
	"1st-level data cache: 16 KBytes, 4-way set associative, 64 byte line size",
	"1st-level data cache: 24 KBytes, 6-way set associative, 64 byte line size",
	NULL, NULL,
// 0x10
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	"2nd-level cache: 128 KBytes, 2-way set associative, 64 byte line size",
	NULL, NULL,
// 0x20
	NULL,
	"2nd-level cache: 256 KBytes, 8-way set associative, 64 byte line size",
	"3rd-level cache: 512 KBytes, 4-way set associative, 64 byte line size, 2 lines per sector",
	"3rd-level cache: 1 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector",
	"2nd-level cache: 1 MBytes, 16-way set associative, 64 byte line size",
	"3rd-level cache: 2 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector",
	NULL, NULL, NULL,
	"3rd-level cache: 4 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector",
	NULL, NULL,
	"1st-level data cache: 32 KBytes, 8-way set associative, 64 byte line size",
	NULL, NULL, NULL,
// 0x30
	"1st-level instruction cache: 32 KBytes, 8-way set associative, 64 byte line size",
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
// 0x40
	"2nd-level cache or, if processor contains a valid 2nd-level cache, no 3rd-level cache",
	"2nd-level cache: 128 KBytes, 4-way set associative, 32 byte line size",
	"2nd-level cache: 256 KBytes, 4-way set associative, 32 byte line size",
	"2nd-level cache: 512 KBytes, 4-way set associative, 32 byte line size",
	"2nd-level cache: 1 MByte, 4-way set associative, 32 byte line size",
	"2nd-level cache: 2 MByte, 4-way set associative, 32 byte line size",
	"3rd-level cache: 4 MByte, 4-way set associative, 64 byte line size",
	"3rd-level cache: 8 MByte, 8-way set associative, 64 byte line size",
	"2nd-level cache: 3MByte, 12-way set associative, 64 byte line size",
	"3rd-level cache: 4MB, 16-way set associative, 64-byte line size (Intel Xeon processor MP, Family 0FH, Model 06H); 2nd-level cache: 4 MByte, 16-way set associative, 64 byte line size",
	"3rd-level cache: 6MByte, 12-way set associative, 64 byte line size",
	"3rd-level cache: 8MByte, 16-way set associative, 64 byte line size",
	"3rd-level cache: 12MByte, 12-way set associative, 64 byte line size",
	"3rd-level cache: 16MByte, 16-way set associative, 64 byte line size",
	"2nd-level cache: 6MByte, 24-way set associative, 64 byte line size",
	"Instruction TLB: 4 KByte pages, 32 entries",
// 0x50
	"Instruction TLB: 4 KByte and 2-MByte or 4-MByte pages, 64 entries",
	"Instruction TLB: 4 KByte and 2-MByte or 4-MByte pages, 128 entries",
	"Instruction TLB: 4 KByte and 2-MByte or 4-MByte pages, 256 entries",
	NULL, NULL,
	"Instruction TLB: 2-MByte or 4-MByte pages, fully associative, 7 entries",
	"Data TLB0: 4 MByte pages, 4-way set associative, 16 entries",
	"Data TLB0: 4 KByte pages, 4-way associative, 16 entries",
	NULL,
	"Data TLB0: 4 KByte pages, fully associative, 16 entries",
	"Data TLB0: 2 MByte or 4 MByte pages, 4-way set associative, 32 entries",
	"Data TLB: 4 KByte and 4 MByte pages, 64 entries",
	"Data TLB: 4 KByte and 4 MByte pages,128 entries",
	"Data TLB: 4 KByte and 4 MByte pages,256 entries",
	NULL, NULL,
// 0x60
	"1st-level data cache: 16 KByte, 8-way set associative, 64 byte line size",
	"Instruction TLB: 4 KByte pages, fully associative, 48 entries",
	NULL,
	"Data TLB: 2 MByte or 4 MByte pages, 4-way set associative, 32 entries and a separate array with 1 GByte pages, 4-way set associative, 4 entries",
	"Data TLB: 4 KByte pages, 4-way set associative, 512 entries",
	NULL,
	"1st-level data cache: 8 KByte, 4-way set associative, 64 byte line size",
	"1st-level data cache: 16 KByte, 4-way set associative, 64 byte line size",
	"1st-level data cache: 32 KByte, 4-way set associative, 64 byte line size",
	NULL,
	"uTLB: 4 KByte pages, 8-way set associative, 64 entries",
	"DTLB: 4 KByte pages, 8-way set associative, 256 entries",
	"DTLB: 2M/4M pages, 8-way set associative, 128 entries",
	"DTLB: 1 GByte pages, fully associative, 16 entries",
	NULL, NULL,
// 0x70
	"Trace cache: 12 K-μop, 8-way set associative",
	"Trace cache: 16 K-μop, 8-way set associative",
	"Trace cache: 32 K-μop, 8-way set associative",
	NULL, NULL, NULL,
	"Instruction TLB: 2M/4M pages, fully associative, 8 entries",
	NULL,
	"2nd-level cache: 1 MByte, 4-way set associative, 64byte line size",
	"2nd-level cache: 128 KByte, 8-way set associative, 64 byte line size, 2 lines per sector",
	"2nd-level cache: 256 KByte, 8-way set associative, 64 byte line size, 2 lines per sector",
	"2nd-level cache: 512 KByte, 8-way set associative, 64 byte line size, 2 lines per sector",
	"2nd-level cache: 1 MByte, 8-way set associative, 64 byte line size, 2 lines per sector",
	"2nd-level cache: 2 MByte, 8-way set associative, 64byte line size",
	NULL,
	"2nd-level cache: 512 KByte, 2-way set associative, 64-byte line size",
// 0x80
	"2nd-level cache: 512 KByte, 8-way set associative, 64-byte line size",
	NULL,
	"2nd-level cache: 256 KByte, 8-way set associative, 32 byte line size",
	"2nd-level cache: 512 KByte, 8-way set associative, 32 byte line size",
	"2nd-level cache: 1 MByte, 8-way set associative, 32 byte line size",
	"2nd-level cache: 2 MByte, 8-way set associative, 32 byte line size",
	"2nd-level cache: 512 KByte, 4-way set associative, 64 byte line size",
	"2nd-level cache: 1 MByte, 8-way set associative, 64 byte line size",
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
// 0x90
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
// 0xA0
	"DTLB: 4k pages, fully associative, 32 entries",
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
// 0xB0
	"Instruction TLB: 4 KByte pages, 4-way set associative, 128 entries",
	"Instruction TLB: 2M pages, 4-way, 8 entries or 4M pages, 4-way, 4 entries",
	"Instruction TLB: 4KByte pages, 4-way set associative, 64 entries",
	"Data TLB: 4 KByte pages, 4-way set associative, 128 entries",
	"Data TLB1: 4 KByte pages, 4-way associative, 256 entries",
	"Instruction TLB: 4KByte pages, 8-way set associative, 64 entries",
	"Instruction TLB: 4KByte pages, 8-way set associative, 128 entries",
	NULL, NULL, NULL,
	"Data TLB1: 4 KByte pages, 4-way associative, 64 entries",
	NULL, NULL, NULL, NULL, NULL,
// 0xC0
	"Data TLB: 4 KByte and 4 MByte pages, 4-way associative, 8 entries",
	"Shared 2nd-Level TLB: 4 KByte/2MByte pages, 8-way associative, 1024 entries",
	"DTLB: 4 KByte/2 MByte pages, 4-way associative, 16 entries",
	"Shared 2nd-Level TLB: 4 KByte /2 MByte pages, 6-way associative, 1536 entries. Also 1GBbyte pages, 4-way, 16 entries.",
	"DTLB: 2M/4M Byte pages, 4-way associative, 32 entries",
	NULL, NULL, NULL, NULL, NULL,
	"Shared 2nd-Level TLB: 4 KByte pages, 4-way associative, 512 entries",
	NULL, NULL, NULL, NULL, NULL,
// 0xD0
	"3rd-level cache: 512 KByte, 4-way set associative, 64 byte line size",
	"3rd-level cache: 1 MByte, 4-way set associative, 64 byte line size",
	"3rd-level cache: 2 MByte, 4-way set associative, 64 byte line size",
	NULL, NULL, NULL,
	"3rd-level cache: 1 MByte, 8-way set associative, 64 byte line size",
	"3rd-level cache: 2 MByte, 8-way set associative, 64 byte line size",
	"3rd-level cache: 4 MByte, 8-way set associative, 64 byte line size",
	NULL, NULL, NULL,
	"3rd-level cache: 1.5 MByte, 12-way set associative, 64 byte line size",
	"3rd-level cache: 3 MByte, 12-way set associative, 64 byte line size",
	"3rd-level cache: 6 MByte, 12-way set associative, 64 byte line size",
	NULL,
// 0xE0
	NULL, NULL,
	"3rd-level cache: 2 MByte, 16-way set associative, 64 byte line size",
	"3rd-level cache: 4 MByte, 16-way set associative, 64 byte line size",
	"3rd-level cache: 8 MByte, 16-way set associative, 64 byte line size",
	NULL, NULL, NULL, NULL, NULL,
	"3rd-level cache: 12MByte, 24-way set associative, 64 byte line size",
	"3rd-level cache: 18MByte, 24-way set associative, 64 byte line size",
	"3rd-level cache: 24MByte, 24-way set associative, 64 byte line size",
	NULL, NULL, NULL,
// 0xF0
	"64-Byte prefetching",
	"128-Byte prefetching",
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

const char* microarch_info(uint32_t model_num)
{
	// https://software.intel.com/en-us/articles/intel-architecture-and-processor-identification-with-cpuid-model-and-family-numbers
	// https://en.wikipedia.org/wiki/List_of_Intel_CPU_microarchitectures
	// http://instlatx64.atw.hu/
	//
	// TODO: Older microarchitectures identification

	switch(model_num)
	{
	// atom microarchitectures
	case 0x1C:
	case 0x26:
		return "Atom - 45 nm";
	case 0x36:
		return "Atom - 32 nm";

	// mainline microarchitectures
	case 0x03:
	case 0x04:
		return "Prescott - 90 nm";
	case 0x06:
		return "Presler - 65 nm";
	case 0x0D:
		return "Dothan - 90 nm";
	case 0x0F:
	case 0x16:
		return "Merom - 65 nm";
	case 0x17:
	case 0x1D:
		return "Penryn - 45 nm";
	case 0x1A:
	case 0x1E:
	case 0x2E:
		return "Nehalem - 45 nm";
	case 0x25:
	case 0x2C:
	case 0x2F:
		return "Westmere - 32 nm";
	case 0x2A:
	case 0x2D:
		return "SandyBridge - 32 nm";
	case 0x3A:
	case 0x3E:
		return "IvyBridge - 22 nm";
	case 0x3C:
	case 0x3F:
		return "Haswell - 22 nm";
	case 0x3D:
	case 0x4F:
		return "Broadwell - 14 nm";
	case 0x55:
	case 0x5E:
		return "Skylake - 14 nm";
	case 0x8E:
	case 0x9E:
		return "KabyLake - 14 nm";

	default:
		return "<Unknow>";
	}
}

/*
static uint64_t xgetbv(uint32_t xsr)
{
#if MSVC_COMPILER
    uint32_t eax_, edx_;
    __asm {
        mov ecx, [xsr]
        __asm _emit 0x0f __asm _emit 0x01 __asm _emit 0xd0
        mov eax_, eax
        mov edx_, edx
    }
    return ((uint64_t)edx_ << 32) | eax_;
#else
    uint32_t hi, lo;
    asm ("xgetbv"
            : "=d" (hi), "=a" (lo)
            : "c" (xsr));

    return (uint64_t(hi) << 32) | lo;
#endif
}
*/

uint32_t info[4];
uint32_t functionID;
uint32_t subfunctionID;
int valid;

int get2(uint32_t function, uint32_t subfunction)
{
#if MSVC_COMPILER
        uint32_t tmp_info[4];

        if (function > 0)
        {
            __cpuid(tmp_info, function & 0x80000000);

            if (tmp_info[0] < function)
            {
                memset(info, 0, 4 * sizeof(*info));
                return valid = 0;
            }
        }

        __cpuidex(info, functionID, subfunctionID);

        return valid = 1;
#else
        unsigned int _eax, _ebx, _ecx, _edx;

        if (function > 0)
        {
            __cpuid(function & 0x80000000, _eax, _ebx, _ecx, _edx);

            if (_eax < function)
            {
                memset(info, 0, 4 * sizeof(*info));
                return valid = 0;
            }
        }

        __cpuid_count(function, subfunction, info[0], info[1], info[2], info[3]);

        return valid = 1;
#endif
}

int get(uint32_t function)
{
	return get2(function, 0);
}

uint32_t eax()  {return valid * info[0];}
uint32_t ebx()  {return valid * info[1];}
uint32_t ecx()  {return valid * info[2];}
uint32_t edx()  {return valid * info[3];}

uint32_t eax2(unsigned bit, unsigned length)  {return valid * ((info[0] >> bit) & ((1ul << length) - 1));}
uint32_t ebx2(unsigned bit, unsigned length)  {return valid * ((info[1] >> bit) & ((1ul << length) - 1));}
uint32_t ecx2(unsigned bit, unsigned length)  {return valid * ((info[2] >> bit) & ((1ul << length) - 1));}
uint32_t edx2(unsigned bit, unsigned length)  {return valid * ((info[3] >> bit) & ((1ul << length) - 1));}

uint32_t eax1(unsigned bit)  {return eax2(bit, 1);}
uint32_t ebx1(unsigned bit)  {return ebx2(bit, 1);}
uint32_t ecx1(unsigned bit)  {return ecx2(bit, 1);}
uint32_t edx1(unsigned bit)  {return edx2(bit, 1);}

#define I(f) { lua_pushstring(L, #f); lua_pushnumber(L, f()); lua_rawset(L, -3); }
#define S(f) { lua_pushstring(L, #f); lua_pushstring(L, f()); lua_rawset(L, -3); }
#define T(f) { lua_pushstring(L, #f); lua_pushboolean(L, thermal_##f()); lua_rawset(L, -3); }

void add_cache_tlb_info(lua_State *L, uint32_t reg, int *array_index)
{
	const char* info0 = CacheTlbDescriptors[(reg      ) & 0xFF];
	const char* info1 = CacheTlbDescriptors[(reg >>  8) & 0xFF];
	const char* info2 = CacheTlbDescriptors[(reg >> 16) & 0xFF];
	const char* info3 = CacheTlbDescriptors[(reg >> 24) & 0xFF];

	if(info0 != NULL) { lua_pushnumber(L, *array_index); lua_pushstring(L, info0); lua_rawset(L, -3); *array_index = *array_index + 1; }
	if(info1 != NULL) { lua_pushnumber(L, *array_index); lua_pushstring(L, info1); lua_rawset(L, -3); *array_index = *array_index + 1; }
	if(info2 != NULL) { lua_pushnumber(L, *array_index); lua_pushstring(L, info2); lua_rawset(L, -3); *array_index = *array_index + 1; }
	if(info3 != NULL) { lua_pushnumber(L, *array_index); lua_pushstring(L, info3); lua_rawset(L, -3); *array_index = *array_index + 1; }
}

uint32_t max_standard_cpuid_leaf() {
	get(0);
	return eax();
}
uint32_t max_extended_cpuid_leaf() {
	get(0x80000000);
	return eax();
}
uint8_t processor_stepping() {
	get(1);
	return eax2(0, 4);
}
uint16_t processor_family() {
	uint16_t temp;
	get(1);
	temp = eax2(8, 4);
	if (temp != 0x0F)
		return temp;
	else
		return temp + eax2(20, 8);
}
uint8_t processor_model() {
	uint16_t family;
	get(1);
	family = processor_family();
	if (family == 0x06 || family == 0x0F)
		return (eax2(16, 4) << 4) + eax2(4, 4);
	return eax2(4, 4);
}
uint8_t processor_type() {
	get(1);
	return eax2(12, 2);
}
uint8_t processor_brand_index() {
	get(1);
	return ebx(0, 8);
}
uint16_t processor_cache_line_size() {
	get(1);
	return ebx(8, 8) * 8;
}
uint32_t max_SOCID_index() {
	get(0x17);
	return eax();
}
uint16_t SOC_vendor_ID() {
	get(0x17);
	return ebx(0, 16);
}
uint8_t cache_line_size() {
	get(0x80000006);
	return ecx(0, 8);
}
uint32_t cache_size() {
	get(0x80000006);
	return ecx(16, 16) * 1024;
}

uint8_t physical_address_bits() {
	get(0x80000008);
	return eax2(0, 8);
}
uint8_t linear_address_bits() {
	get(0x80000008);
	return eax2(8, 8);
}

uint16_t processor_base_frequency_MHz() {
	return eax(0, 16);
}
uint16_t processor_max_frequency_MHz() {
	return ebx(0, 16);
}
uint16_t processor_bus_reference_frequency_MHz() {
	return ecx(0, 16);
}

const char *vendor()
{
    char vendor_string[16] = {0};

    get(0);

	*(uint32_t *)(&vendor_string[0]) = ebx();
	*(uint32_t *)(&vendor_string[4]) = edx();
	*(uint32_t *)(&vendor_string[8]) = ecx();
	vendor_string[12] = 0;

    return strdup(vendor_string);
}

const char *processor_brand_string()
{
	uint32_t idx = 0;
	uint32_t brand_idx = 0;
	char *ns = 0;
	uint32_t brand[12] = {0};

	for(idx = 0 ; idx < 3 ; ++idx)
	{
		get(0x80000002 + idx);
		*(uint32_t *)&brand[brand_idx] = eax();
		*(uint32_t *)&brand[brand_idx+1] = ebx();
		*(uint32_t *)&brand[brand_idx+2] = ecx();
		*(uint32_t *)&brand[brand_idx+3] = edx();

		brand_idx += 4;
	}

	for(ns = (char *)brand ;  ; ++ns)
	{
		if (*ns != ' ')
		{
			break;
		}
	}

    return strdup(ns);
}

const char *processor_brand_name()
{
    get(1);

    switch (ebx() & 0xff) {
        default: return "";
        case 0x01: // Fallthrough
        case 0x0A: // Fallthrough
        case 0x14: return "Intel(R) Celeron(R) processor";
        case 0x02: // Fallthrough
        case 0x04: return "Intel(R) Pentium(R) III processor";
        case 0x03: return eax() == 0x000006B1? "Intel(R) Celeron(R) processor": "Intel(R) Pentium(R) III Xeon(R) processor";
        case 0x06: return "Mobile Intel(R) Pentium(R) III processor-M";
        case 0x07: return "Mobile Intel(R) Celeron(R) processor";
        case 0x08: // Fallthrough
        case 0x09: return "Intel(R) Pentium(R) 4 processor";
        case 0x0B: return eax() == 0x00000F13? "Intel(R) Xeon(R) processor MP": "Intel(R) Xeon(R) processor";
        case 0x0C: return "Intel(R) Xeon(R) processor MP";
        case 0x0E: return eax() == 0x00000F13? "Intel(R) Xeon(R) processor": "Mobile Intel(R) Pentium(R) 4 processor-M";
        case 0x0F: // Fallthrough
        case 0x17: return "Mobile Intel(R) Celeron(R) processor";
        case 0x11: // Fallthrough
        case 0x15: return "Mobile Genuine Intel(R) processor";
        case 0x12: return "Intel(R) Celeron(R) M processor";
        case 0x13: return "Mobile Intel(R) Celeron(R) processor";
        case 0x16: return "Intel(R) Pentium(R) M processor";
    }
}

void new_table(void *state, const char *name)
{
	lua_State *L = (lua_State *)state;
	lua_pushstring(L, name);
	lua_newtable(L);
}

void close_table(void *state)
{
	lua_State *L = (lua_State *)state;
	lua_rawset(L, -3);
}

void add_entry(void *state, const char *key, const char *value)
{
	lua_State *L = (lua_State *)state;
	lua_pushstring(L, key);
	lua_pushstring(L, value);
	lua_rawset(L, -3);
}

/* Returns a table containing informations about processor and system */
static SAVEDS int hw_SysInfo(lua_State *L)
{
	uint32_t i = 0;
	int array_index = 0;

	lua_newtable(L);

	lua_pushstring(L, "cpu");
	lua_newtable(L);

	lua_pushstring(L, "ident");
	lua_newtable(L);

	S(vendor)
	S(processor_brand_string)
	I(max_standard_cpuid_leaf)
	I(processor_stepping)
	I(processor_model)
	I(processor_family)
	I(processor_type)
	I(processor_brand_index)
	I(SOC_vendor_ID)
	//I(processor_serial_number_lo_bits)

	lua_pushstring(L, "Microarchitecture");
	lua_pushstring(L, microarch_info(processor_model()));
	lua_rawset(L, -3);

	lua_rawset(L, -3);

	lua_pushstring(L, "features");

	lua_newtable(L);

	get(1);

	// Get the features encoded in edx
	for(i = 0; i < EDX_FEATURES_SIZE; ++i)
	{
		if(edx() & EdxFeatures[i].mask)
		{
			lua_pushnumber(L, array_index);
			lua_pushstring(L, EdxFeatures[i].name);
			lua_rawset(L, -3);
			++array_index;
		}
	}

	// Get the features encoded in ecx
	for(i = 0; i < ECX_FEATURES_SIZE; ++i)
	{
		if(ecx() & EcxFeatures[i].mask)
		{
			lua_pushnumber(L, array_index);
			lua_pushstring(L, EcxFeatures[i].name);
			lua_rawset(L, -3);
			++array_index;
		}
	}

	lua_rawset(L, -3);

	lua_pushstring(L, "extended_features");

	lua_newtable(L);

	get(7);

	array_index = 0;

	// Get the extended features encoded in ebx
	for(i = 0; i < EBX_EXT_FEATURES_SIZE; ++i)
	{
		if(ebx() & EbxExtFeatures[i].mask)
		{
			lua_pushnumber(L, array_index);
			lua_pushstring(L, EbxExtFeatures[i].name);
			lua_rawset(L, -3);
			++array_index;
		}
	}

	// Get the extended features encoded in ecx
	for(i = 0; i < ECX_EXT_FEATURES_SIZE; ++i)
	{
		if(ecx() & EcxExtFeatures[i].mask)
		{
			lua_pushnumber(L, array_index);
			lua_pushstring(L, EcxExtFeatures[i].name);
			lua_rawset(L, -3);
			++array_index;
		}
	}

	get(0x80000001);

	// Get the extended functions encoded in ecx
	for(i = 0; i < ECX_EXT_FUNCTIONS_SIZE; ++i)
	{
		if(ecx() & EcxExtFunctions[i].mask)
		{
			lua_pushnumber(L, array_index);
			lua_pushstring(L, EcxExtFunctions[i].name);
			lua_rawset(L, -3);
			++array_index;
		}
	}

	// Get the extended functions encoded in edx
	for(i = 0; i < EDX_EXT_FUNCTIONS_SIZE; ++i)
	{
		if(edx() & EdxExtFunctions[i].mask)
		{
			lua_pushnumber(L, array_index);
			lua_pushstring(L, EdxExtFunctions[i].name);
			lua_rawset(L, -3);
			++array_index;
		}
	}

	lua_rawset(L, -3);

/*
	lua_pushstring(L, "thermal");
	lua_newtable(L);

	T(digital_sensor_supported)
	T(Intel_TurboBoost_supported)
	T(APIC_timer_always_running_supported)
	T(PLN_controls_supported)
	T(ECMD_supported)
	T(PTM_supported)
	T(HWP_supported)
	T(HWP_notification_supported)
	T(HWP_Activity_Window_supported)
	T(HWP_Energy_Performance_Preference_supported)
	T(HWP_Package_Level_Request_supported)
	T(HDC_supported)
	T(Intel_Turbo_Boost_3_supported)
	T(Highest_Performance_change_supported)
	T(HWP_PECI_override_supported)
	T(flexible_HWP_supported)
	T(HWP_request_fast_access_supported)
	T(ignoring_idle_logical_processor_supported)
	I(thermal_sensor_interrupt_thresholds)
	T(hardware_coordination_feedback_supported)
	T(performance_energy_bias_preference)

	lua_rawset(L, -3);
*/

	lua_pushstring(L, "caches");
	lua_newtable(L);

	I(cache_line_size)
	I(cache_size)
	I(processor_cache_line_size)

	get(2);

	if (valid)
	{
		array_index = 0;

		if(~eax() & 0x80000000)
		{
			add_cache_tlb_info(L, eax() >> 8, &array_index);
		}
		if(~ebx() & 0x80000000)
		{
			add_cache_tlb_info(L, ebx(), &array_index);
		}
		if(~ecx() & 0x80000000)
		{
			add_cache_tlb_info(L, ecx(), &array_index);
		}
		if(~edx() & 0x80000000)
		{
			add_cache_tlb_info(L, edx(), &array_index);
		}
	}

	lua_rawset(L, -3);

	get(0x16);

	if (valid)
	{
		lua_pushstring(L, "freqs");
		lua_newtable(L);
		I(processor_base_frequency_MHz)
		I(processor_max_frequency_MHz)
		I(processor_bus_reference_frequency_MHz)
		lua_rawset(L, -3);
	}

	lua_rawset(L, -3);

	lua_pushstring(L, "sys");
	lua_newtable(L);

	fill_systable((void *)L);

	lua_rawset(L, -3);

	return 1;

}

/* table containing all commands to be added by this plugin */
struct hwCmdStruct plug_commands[] = {
	{(STRPTR)"SysInfo", hw_SysInfo},
	{NULL, NULL}
};

/* table containing all constants to be added by this plugin */
struct hwCstStruct plug_constants[] = {
	{NULL, NULL, 0}
};

/* return base table's name */
HW_EXPORT STRPTR GetBaseTable(void)
{
	return (STRPTR) basetable;
}

/* return command table */
HW_EXPORT struct hwCmdStruct *GetCommands(void)
{
	return (struct hwCmdStruct *) plug_commands;
}

/* return constant table */
HW_EXPORT struct hwCstStruct *GetConstants(void)
{
	return (struct hwCstStruct *) plug_constants;
}

/* you may do additional initialization here */
HW_EXPORT int InitLibrary(lua_State *L)
{
	return 0;
}

/* you may do additional clean-up here */
#if defined(HW_WIN32) && defined(HW_64BIT)
HW_EXPORT void _FreeLibrary(lua_State *L)
#else
HW_EXPORT void FreeLibrary(lua_State *L)
#endif
{
}
