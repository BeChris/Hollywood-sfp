import configparser
import os
import re
import stat
import sys

if sys.platform == 'linux':
    HOLLYWOOD = '$HOME/devel/Hollywood'
elif sys.platform == 'darwin':
    HOLLYWOOD = '/Applications/Hollywood.app/Contents/Resources/HollywoodInterpreter.app/Contents/Resources'
else:
    sys.exit("%s platform is not supported" % sys.platform)

HOME    = os.getenv('HOME')
PROJECT = 'project'
SOURCES = 'sources'
INCDIRS = 'incdirs'
DEFINES = 'defines'
LIBS    = 'libs'
TESTS   = 'tests'
NAME    = 'name'
PLATFORMS = 'platforms'
ALLOWED_PLATFORMS = ['aros', 'linux', 'linux64', 'linuxarm', 'macos', 'macos64', 'mos', 'os3', 'os3fpu', 'os4', 'win32', 'applet']

is_applet = {'applet'}
is_amiga = {'aros', 'mos', 'os3', 'os3fpu', 'os4'}

C_COMPILER = {
    'aros': 'i386-aros-gcc',
    'linux': 'gcc',
    'linux64': 'gcc',
    'linuxarm': 'arm-linux-gnueabihf-gcc',
    'macos': 'i386-apple-darwin17-cc',
    'macos64': 'x86_64-apple-darwin17-cc',
    'mos': 'ppc-morphos-gcc',
    'os3': 'vc',
    'os3fpu': 'vc',
    'os4': 'ppc-amigaos-gcc',
    'win32': './cl.sh'
}

SYSROOT = {
    'aros': '/opt/i386-aros',
    'linuxarm': '/opt/arm-raspbian',
    'macos': '/opt/i386-macos',
    'macos64': '/opt/i386-macos',
    'mos': '/opt/ppc-morphos',
    'os3': '/opt/m68k-amigaos',
    'os3fpu': '/opt/m68k-amigaos',
    'os4': '/opt/ppc-amigaos'
}

EXE_WRAPPER = {
    'win32': 'wine'
}

EXE_SUFFIX = {
    'macos': '.app',
    'macos64': '64.app',
    'win32': '.exe',
    'applet': '.hwa'
}

EXE_PLATFORM = {
    'aros': 'aros',
    'linux': 'linux',
    'linux64': 'linux64',
    'linuxarm': 'linuxarm',
    'macos': 'macos86',
    'macos64': 'macos64',
    'mos': 'morphos',
    'os3': 'classic',
    'os3fpu': 'classic881',
    'os4': 'amigaos4',
    'win32': 'win32console',
    'applet': 'applet'
}

EXE_DESTDIR = {
    'aros': '/opt/IcarosDesktop/C'
}

PLUGIN_DESTDIR = {
    'aros': HOLLYWOOD + '/LinkerPlugins/x86-aros',
    'linux': HOLLYWOOD + '/LinkerPlugins/x86-linux',
    'linux64': HOLLYWOOD + '/LinkerPlugins/x64-linux',
    'linuxarm': HOLLYWOOD + '/LinkerPlugins/arm-linux',
    'macos': HOLLYWOOD + '/LinkerPlugins/x86-macos',
    'macos64': HOLLYWOOD + '/LinkerPlugins/x64-macos',
    'mos': HOLLYWOOD + '/LinkerPlugins/ppc-morphos',
    'os3': HOLLYWOOD + '/LinkerPlugins/m68k-amigaos',
    'os3fpu': HOLLYWOOD + '/LinkerPlugins/m881-amigaos',
    'os4': HOLLYWOOD + '/LinkerPlugins/ppc-amigaos',
    'win32': HOLLYWOOD + '/LinkerPlugins/x86-windows-console',
}

is_gcc = {'aros', 'linux', 'linux64', 'linuxarm', 'macos', 'macos64', 'mos', 'os4'}
is_msvc = {'win32'}
is_vbcc = {'os3', 'os3fpu'}

C_COMPILER_OPTS = {
    'aros': '-c -DHW_AMIGA -DHW_AROS -DHW_LITTLE_ENDIAN -O2 -Wall -Wno-pointer-sign -MMD -MP',
    'linux': '-c -m32 -fPIC -DHW_LINUX -DHW_LITTLE_ENDIAN -O2 -Wall -Wno-pointer-sign -MMD -MP',
    'linux64': '-c -fPIC -DHW_LINUX -DHW_LITTLE_ENDIAN -O2 -Wall -Wno-pointer-sign -MMD -MP',
    'linuxarm': '-c -fPIC -DHW_LINUX -DHW_LITTLE_ENDIAN -O2 -Wall -Wno-pointer-sign -MMD -MP',
    'macos': '-c -fPIC -DHW_MACOS -DHW_LITTLE_ENDIAN -O2 -Wall -Wno-pointer-sign -MMD -MP',
    'macos64': '-c -fPIC -DHW_MACOS -DHW_LITTLE_ENDIAN -O2 -Wall -Wno-pointer-sign -MMD -MP',
    'mos': '-c -DHW_AMIGA -DHW_MORPHOS -O2 -Wall -Wno-pointer-sign -noixemul -MMD -MP',
    'os3': '-c -deps -c99 -O -cpu=68020 -DHW_AMIGA -DHW_AMIGAOS3',
    'os3fpu': '-c -deps -c99 -O -cpu=68020 -fpu=68881 -no-fp-return -D__NOINLINE__ -DHW_AMIGA -DHW_AMIGAOS3 -DHW_FPU',
    'os4': '-c -DHW_AMIGA -DHW_AMIGAOS4 -D__USE_INLINE__ -mcrt=clib2 -O2 -Wall -Wno-pointer-sign -MMD -MP',
    'win32': '/nologo /W3 /O2 /EHsc /DHW_WIN32 /DHW_LITTLE_ENDIAN /DNDEBUG /D_MBCS /D_CRT_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_DEPRECATE /MT /c'
}

LINKER = C_COMPILER.copy()
LINKER['win32'] = './link.sh'

LINKER_OPTS = {
    'aros': '-nostartfiles',
    'linux': '-m32 -shared -fPIC -Wl,--version-script=plugin_linux.def -Wl,-soname,$PROJECT',
    'linux64': '-shared -fPIC -Wl,--version-script=plugin_linux.def -Wl,-soname,$PROJECT',
    'linuxarm': '-shared -fPIC -Wl,--version-script=plugin_linux.def -Wl,-soname,$PROJECT',
    'macos': '-dynamiclib -fPIC -exported_symbols_list plugin_macos.def',
    'macos64': '-dynamiclib -fPIC -exported_symbols_list plugin_macos.def',
    'mos': '-nostartfiles -noixemul',
    'os3': '-nostdlib',
    'os3fpu': '-nostdlib',
    'os4': '-nostartfiles -mcrt=clib2',
    'win32': '/dll /nologo'
}

LINKER_LIBS = {
    'aros': '-lm',
    'mos': '-lc -lmath',
    'os3': '-lmieee -lvc',
    'os3fpu': '-lm881 -lvc',
    'os4': '-lm',
    'win32': 'kernel32.lib'
}

STRIP_COMMAND = {
    'aros': 'i386-aros-strip --remove-section .comment',
    'linux': 'strip --remove-section .comment',
    'linux64': 'strip --remove-section .comment',
    'linuxarm': 'arm-linux-gnueabihf-strip --remove-section .comment',
    'macos': 'i386-apple-darwin17-strip -x',
    'macos64': 'x86_64-apple-darwin17-strip -x',
    'mos': 'ppc-morphos-strip --remove-section .comment',
    'os4': 'ppc-amigaos-strip --remove-section .comment'
}

config = configparser.ConfigParser(allow_no_value=True)
if len(config.read('build.ini')) == 0:
    sys.exit("build.ini file unreadable ?")

# Check mandatory sections [project] and [sources] are here
for mandatory_section in [PROJECT, SOURCES]:
    if mandatory_section not in config.sections():
        sys.exit("Missing mandatory section [%s]" % mandatory_section)

# Check mandatory name and platforms key in [project]
for key in [NAME, PLATFORMS]:
    if key not in config[PROJECT]:
        sys.exit("Missing mandatory key '%s' in section [%s]" % (key, PROJECT))

# Check name and platforms are not empty
for key in [NAME, PLATFORMS]:
    if config[PROJECT][key] is None or len(config[PROJECT][key]) == 0:
        sys.exit("Value can't be empty for key '%s' in section [%s]" % (key, PROJECT))

# Check given platforms are recognized
platforms = config[PROJECT][PLATFORMS].split(' ')

for platform in platforms:
    if platform not in ALLOWED_PLATFORMS:
        sys.exit("Platform '%s' is not among autorized ones : [%s]" % (platform, ", ".join(ALLOWED_PLATFORMS)))

# Now generate build file for each platform
project = config[PROJECT][NAME]

compile_scripts = []

for platform in platforms:
    with open('build.%s' % platform, 'w') as wd:
        if platform in C_COMPILER_OPTS:
            c_compiler_opts = C_COMPILER_OPTS[platform]

        incdirs = []

        if INCDIRS in config:
            for incdir in config[INCDIRS]:
                incdirs.append(incdir)

        PLATFORM_INCDIRS = [platform + ":" + INCDIRS]
        if platform in is_amiga:
            PLATFORM_INCDIRS.append("amiga:" + INCDIRS)

        for PLATFORM_INCDIRS in PLATFORM_INCDIRS:
            if PLATFORM_INCDIRS in config:
                for incdir in config[PLATFORM_INCDIRS]:
                    incdirs.append(incdir)

        for incdir in incdirs:
            if platform in is_msvc:
                c_compiler_opts = c_compiler_opts + " /I%s" % incdir
            else:
                c_compiler_opts = c_compiler_opts + " -I%s" % incdir

        defines = []

        if DEFINES in config:
            for define in config[DEFINES]:
                defines.append(define)

        PLATFORM_DEFINES = [platform + ":" + DEFINES]
        if platform in is_amiga:
            PLATFORM_DEFINES.append("amiga:" + DEFINES)
        
        for PLATFORM_DEFINES in PLATFORM_DEFINES: 
            if PLATFORM_DEFINES in config:
                for define in config[PLATFORM_DEFINES]:
                    defines.append(define)

        for define in defines:
            if platform in is_msvc:
                c_compiler_opts = c_compiler_opts + " /D%s" % define
            else:
                c_compiler_opts = c_compiler_opts + " -D%s" % define

        libs = []

        if platform in LINKER_LIBS:
            libs = LINKER_LIBS[platform].split(' ')
        
        PLATFORM_LIBS = [platform + ":" + LIBS]
        if platform in is_amiga:
            PLATFORM_LIBS.append("amiga:" + LIBS)
        
        for PLATFORM_LIBS in PLATFORM_LIBS: 
            if PLATFORM_LIBS in config:
                for lib in config[PLATFORM_LIBS]:
                    libs.append(lib)

        objs = []
        wd.write("HOME = %s\n" % HOME)
        wd.write("PROJECT = %s.hwp\n" % project)
        if not platform in is_applet:
            wd.write("PLUGIN_DESTDIR = %s\n" % PLUGIN_DESTDIR[platform])
            wd.write("\n")
            wd.write("builddir = build/%s\n" % platform)
            wd.write("\n")
            wd.write("C_COMPILER      = %s\n" % C_COMPILER[platform])
            wd.write("C_COMPILER_OPTS = %s\n" % c_compiler_opts)
            wd.write("\n")
            wd.write("LINKER          = %s\n" % LINKER[platform])
            wd.write("LINKER_OPTS     = %s\n" % LINKER_OPTS[platform])
            wd.write("LINKER_LIBS     = %s\n" % " ".join(libs))
        wd.write("\n")
        wd.write("rule clean\n")
        wd.write("  command = /bin/rm -rf $to_clean\n")
        wd.write("  description = clean $platform\n")
        wd.write("\n")
        wd.write("rule cp\n")
        wd.write("  command = /bin/cp -f $in $out\n")
        wd.write("  description = copy $out\n")
        wd.write("\n")
        wd.write("rule exe\n")
        wd.write("  command = %s/Interpreter -compress -compile $out -exetype $platform -overwrite $in\n" % HOLLYWOOD)
        wd.write("  description = generate $out\n")
        wd.write("\n")
        if not platform in is_applet:
            wd.write("rule cc\n")
            if platform in is_gcc or platform in is_vbcc:
                wd.write("  command = ${C_COMPILER} -o $out ${C_COMPILER_OPTS} $in\n")
                wd.write("  deps = gcc\n")
                wd.write("  depfile = $depfile\n")
                wd.write("  description = compile $in\n")
            else:
                wd.write("  command = ${C_COMPILER} ${C_COMPILER_OPTS} $in /Fo$out\n")
                wd.write("  description = compile $in\n")
            wd.write("\n")
            wd.write("rule link\n")
            if platform in is_gcc or platform in is_vbcc:
                link_command = "${LINKER} ${LINKER_OPTS} $in -o $out ${LINKER_LIBS}"
                if platform in STRIP_COMMAND:
                    link_command = link_command + " && " + STRIP_COMMAND[platform] + " $out"
                wd.write("  command = %s\n" % link_command)
                wd.write("  description = link $out\n")
            else:
                wd.write("  command = ${LINKER} ${LINKER_OPTS} /out:$out $in ${LINKER_LIBS}\n")
                wd.write("  description = link $out\n")
            wd.write("\n")

            srcs = []
            for src in config[SOURCES]:
                srcs.append(src)

            PLATFORM_SOURCES = [platform + ":" + SOURCES]
            if platform in is_amiga:
                PLATFORM_SOURCES.append("amiga:" + SOURCES)

            for PLATFORM_SOURCES in PLATFORM_SOURCES:
                if PLATFORM_SOURCES in config:
                    for src in config[PLATFORM_SOURCES]:
                        srcs.append(src)

            for src in srcs:
                obj = re.sub('\.c$', '.o', src)
                objs.append("$builddir/%s" % obj)
                wd.write("build $builddir/%s: cc src/%s\n" % (obj, src))
                if platform in is_gcc:
                    dep = re.sub('\.c$', '.d', src)
                    wd.write("  depfile = $builddir/%s\n" % dep)
                elif platform in is_vbcc:
                    dep = re.sub('\.c$', '.dep', src)
                    wd.write("  depfile = src/%s\n" % dep)
                wd.write("\n")
            wd.write("build ${builddir}/${PROJECT}: link %s\n" % " ".join(objs))
            wd.write("\n")
        wd.write("rule generate\n")
        wd.write("  command = %s %s\n" % (sys.executable, __file__))
        wd.write("  generator = 1\n")
        wd.write("  description = generates build files\n")
        wd.write("build build.%s: generate | %s build.ini\n" % (platform, __file__))
        wd.write("\n")
        if not platform in is_applet:
            wd.write("build ${PLUGIN_DESTDIR}/${PROJECT}: cp ${builddir}/${PROJECT}\n")
            wd.write("\n")
            wd.write("default ${PLUGIN_DESTDIR}/${PROJECT}\n")
            wd.write("\n")

        
        tests = []
        if TESTS in config:
            for test in config[TESTS]:
                tests.append(test)

        PLATFORM_TESTS = [platform + ":" + TESTS]
        if platform in is_amiga:
            PLATFORM_TESTS.append("amiga:" + TESTS)
        
        for PLATFORM_TESTS in PLATFORM_TESTS: 
            if PLATFORM_TESTS in config:
                for test in config[PLATFORM_TESTS]:
                    tests.append(test)

        tests_to_clean = []
        for test in tests:
            suffix = "." + platform
            if platform in EXE_SUFFIX:
                suffix = EXE_SUFFIX[platform]
            exe = re.sub('\.hws$', suffix, test)
            if platform in EXE_DESTDIR:
                exe = EXE_DESTDIR[platform] + "/" + exe

            tests_to_clean.append(exe)

            if platform in is_applet:
                wd.write("build %s: exe %s\n" % (exe, test))
                wd.write("  platform = %s\n" % EXE_PLATFORM[platform])
            else:
                wd.write("build %s: exe %s | ${PLUGIN_DESTDIR}/${PROJECT}\n" % (exe, test))
                wd.write("  platform = %s\n" % EXE_PLATFORM[platform])
            wd.write("\n")
            wd.write("default %s\n" % exe)

        wd.write("\n")
        if platform in is_applet:
            if len(tests_to_clean) > 0:
                wd.write("build clean: clean\n")
                wd.write("  to_clean = %s\n" % " ".join(tests_to_clean))
                wd.write("  platform = %s\n" % platform)
                wd.write("\n")
        else:
            wd.write("build clean: clean\n")
            wd.write("  to_clean = %s ${builddir}/${PROJECT} %s\n" % (" ".join(objs), " ".join(tests_to_clean)))
            wd.write("  platform = %s\n" % platform)
            wd.write("\n")

    compile_script = 'compile_%s.sh' % platform
    with open(compile_script, 'w') as wd:
        wd.write("#!/bin/sh\n")
        if platform in SYSROOT:
            wd.write("PATH=%s/bin:$PATH " % SYSROOT[platform])
        wd.write("ninja -f build.%s $@\n" % platform)

        compile_scripts.append(compile_script)

    os.chmod(compile_script, stat.S_IXUSR | stat.S_IRUSR | stat.S_IWUSR)

with open('compile_all.sh', 'w') as wd:
    wd.write("#!/bin/sh\n")
    for compile_script in compile_scripts:
        wd.write("./%s $@\n" % compile_script)

os.chmod('compile_all.sh', stat.S_IXUSR | stat.S_IRUSR | stat.S_IWUSR)

with open('clean_all.sh', 'w') as wd:
    wd.write("#!/bin/sh\n")
    for compile_script in compile_scripts:
        wd.write("./%s clean\n" % compile_script)

os.chmod('clean_all.sh', stat.S_IXUSR | stat.S_IRUSR | stat.S_IWUSR)
