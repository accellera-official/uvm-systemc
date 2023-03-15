##                            -*- Autoconf -*-
## --------------------------------------------------------------------------
##
##   Copyright 2022 NXP B.V.
##   Copyright 2014 Université Pierre et Marie Curie, Paris
##   Copyright 2014-2018 Fraunhofer-Gesellschaft zur Foerderung
##                  der angewandten Forschung e.V.
##   Copyright 2007-2011 Mentor Graphics Corporation
##   Copyright 2007-2011 Cadence Design Systems, Inc.
##   Copyright 2010-2011 Synopsys, Inc.
##   Copyright 2018 Robert Bosch GmbH
##   Copyright 2019 COSEDA Technologies GmbH
##   All Rights Reserved Worldwide
##
##   Licensed under the Apache License, Version 2.0 (the "License");
##   you may not use this file except in compliance with the License.
##   You may obtain a copy of the License at
##
##       http://www.apache.org/licenses/LICENSE-2.0
##
##   Unless required by applicable law or agreed to in writing, software
##   distributed under the License is distributed on an "AS IS" BASIS,
##   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##   See the License for the specific language governing permissions and
##   limitations under the License.
##
## --------------------------------------------------------------------------
##
##   configure.ac --
##   Process this file with autoconf to produce a configure script.
##
##   Original Author: Thomas Uhle, Fraunhofer IIS / EAS Dresden
##   Created on: 2014-03-10
##
## --------------------------------------------------------------------------
##
##   SVN Version       :  $Revision: 510 $
##   SVN last checkin  :  $Date: 2015-01-08 19:31:34 +0100 (Thu, 08 Jan 2015) $
##   SVN checkin by    :  $Author: barnasconi $
##   SVN Id            :  $Id: configure.ac 510 2015-01-08 18:31:34Z barnasconi $
##
## --------------------------------------------------------------------------

AC_PREREQ(2.61)

AC_INIT([UVM-SystemC],
        [1.0-beta5],
        [uvm-systemc-feedback@lists.accellera.org],
        [uvm-systemc],
        [Download-URL],
)

AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([config])
AC_CONFIG_SRCDIR([src/uvm.h])
AC_CONFIG_HEADERS([src/config.h])

AC_DEFINE_UNQUOTED(CONFIGURE_DATE, "`sh -c date`", [Date of configure])
AC_SUBST(CONFIGURE_DATE)

AC_DEFINE_UNQUOTED(UVM_VERSION, "${PACKAGE_VERSION}", [UVM SystemC Version])
AC_DEFINE_UNQUOTED(UVM_RELEASE_DATE, "2023-03-15", [UVM SystemC Release Date])
AC_SUBST(UVM_VERSION)
AC_SUBST(UVM_RELEASE_DATE)

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([1.9.4 tar-pax -Werror -Wno-portability no-define subdir-objects foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES(yes)])

# ignore GNU defaults for CFLAGS and CXXFLAGS
: ${CFLAGS=""}
: ${CXXFLAGS=""}

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S


# figure out architecture dependent variables

CXX_COMP=`set $CXX; basename $1`

EXTRA_CXXFLAGS="-Wall -pedantic -Wno-long-long -Wno-variadic-macros"
DEBUG_CXXFLAGS="-g"
OPT_CXXFLAGS="-O3"
LDFLAG_RPATH="-Wl,-rpath="

case "$depmode" in
    gcc3)
        EXTRA_CXXFLAGS="${EXTRA_CXXFLAGS} -fmessage-length=0"
        ;;
    *)
        ;;
esac

case "$target" in
    *-apple-*)
        case "$target_cpu" in
            x86_64)
                TARGET_ARCH="macosx64"
                CPU_ARCH="x86_64"
                ;;
            x386 | i386)
                TARGET_ARCH="macosx"
                CPU_ARCH="i386"
                ;;
            powerpc)
                TARGET_ARCH="macosxppc"
                CPU_ARCH="ppc"
                ;;
            powerpc64)
                TARGET_ARCH="macosxppc64"
                CPU_ARCH="ppc64"
                ;;
            *)
                AC_MSG_ERROR("sorry... architecture not supported.")
                ;;
        esac
        if test "$GXX" = "yes"; then
                EXTRA_CXXFLAGS="${EXTRA_CXXFLAGS} -arch ${CPU_ARCH}"
                LDFLAG_RPATH="-Wl,-rpath -Wl,"
        else
                AC_MSG_ERROR("sorry... compiler not supported.")
        fi
        ;;
    sparc-sun-solaris*)
        if test "$GXX" = "yes"; then
                TARGET_ARCH="gccsparcOS5"
                LDFLAG_RPATH="-Wl,-R"
        else
            if test "$CXX_COMP" = "CC"; then
                EXTRA_CXXFLAGS=""
                OPT_CXXFLAGS="-fast -xO4 -xnolibmil -mc"
                TARGET_ARCH="sparcOS5"
                LDFLAG_RPATH="-Wl,-R"
            else
                AC_MSG_ERROR("sorry... compiler not supported.")
            fi
        fi
        ;;
    *linux*)
        case "$target_cpu" in
            x86_64 | amd64)
                TARGET_ARCH="linux64"
                CPU_ARCH=64
                ;;
            i*86)
                TARGET_ARCH="linux"
                CPU_ARCH=32
                ;;
            *)
                AC_MSG_ERROR("sorry... architecture not supported.")
            ;;
        esac
        if test "$GXX" = "yes"; then
                EXTRA_CXXFLAGS="${EXTRA_CXXFLAGS} -m${CPU_ARCH}"
        else
                AC_MSG_ERROR("sorry... compiler not supported.")
        fi
        ;;
    *freebsd* | dragonfly* | *netbsd* | openbsd*)
        case "$target_cpu" in
            x86_64 | amd64)
                TARGET_ARCH="bsd64"
                CPU_ARCH=64
                ;;
            i*86)
                TARGET_ARCH="bsd"
                CPU_ARCH=32
                ;;
            *)
                AC_MSG_ERROR("sorry... architecture not supported.")
            ;;
        esac
        if test "$GXX" = "yes"; then
                EXTRA_CXXFLAGS="${EXTRA_CXXFLAGS} -m${CPU_ARCH}"
        else
                AC_MSG_ERROR("sorry... compiler not supported.")
        fi
        ;;
    *cygwin*)
        case "$target_cpu" in
            x86_64 | amd64)
                TARGET_ARCH="cygwin64"
                CPU_ARCH=64
                ;;
            i*86)
                TARGET_ARCH="cygwin"
                CPU_ARCH=32
                ;;
            *)
                AC_MSG_ERROR("sorry... architecture not supported.")
            ;;
        esac
        if test "$GXX" = "yes"; then
                EXTRA_CXXFLAGS="${EXTRA_CXXFLAGS} -m${CPU_ARCH}"
        else
                AC_MSG_ERROR("sorry... compiler not supported.")
        fi
        ;;
    *mingw*)
        case "$target_cpu" in
            x86_64 | amd64)
                TARGET_ARCH="mingw64"
                CPU_ARCH=64
                ;;
            i*86)
                TARGET_ARCH="mingw"
                CPU_ARCH=32
                ;;
            *)
                AC_MSG_ERROR("sorry... architecture not supported.")
            ;;
        esac
        if test "$GXX" = "yes"; then
                EXTRA_CXXFLAGS="${EXTRA_CXXFLAGS} -m${CPU_ARCH}"
        else
                AC_MSG_ERROR("sorry... compiler not supported.")
        fi
        ;;
esac

AC_SUBST(LDFLAG_RPATH)
AC_SUBST(TARGET_ARCH)


# append additional flags to configure script

AC_MSG_CHECKING(whether to include debug symbols)
AC_ARG_ENABLE([debug],
  AS_HELP_STRING([--enable-debug],
                 [include debug symbols]),
  [case "$enableval" in
    "no" | "yes")  enable_debug=$enableval;;
    *)             AC_MSG_ERROR(bad value "$enableval" for --enable-debug);;
  esac],
  [enable_debug="not defined"])
AC_MSG_RESULT($enable_debug)


AC_MSG_CHECKING(whether to enable code profiling)
AC_ARG_ENABLE([profiling],
  [AS_HELP_STRING([--enable-profiling@<:@=PROGRAM@:>@],
                  [generate instrumentation calls for code
                   profiling, supported profilers: prof,
                   gprof, instrument (-finstrument-functions)
                   @<:@default=no@:>@])],
  [case "$enableval" in
    "no" | "prof" | "gprof" | "instrument")  enable_profiling=$enableval;;
    "yes")  AC_MSG_ERROR(no profiler defined with --enable-profiling);;
    *)      AC_MSG_ERROR(bad value "$enableval" for --enable-profiling);;
  esac],
  [enable_profiling="no"]
)
AC_MSG_RESULT($enable_profiling)

if test "$enable_profiling" = "no"; then
  if test "$enable_debug" = "not defined"; then
    enable_debug="no"
  fi
else
  case "$enable_debug" in
    "not defined")  enable_debug="yes";;
    "no")           AC_MSG_ERROR(contradicting options: 
           Cannot have --disable-debug with --enable-profiling.
           Please re-run configure with only one of these options 
           at the same time.);;
  esac
  case "$enable_profiling" in
    "prof")        DEBUG_CXXFLAGS="${DEBUG_CXXFLAGS} -p";;
    "gprof")       DEBUG_CXXFLAGS="${DEBUG_CXXFLAGS} -pg";;
    "instrument")  DEBUG_CXXFLAGS="${DEBUG_CXXFLAGS} -finstrument-functions";;
  esac
fi
AM_CONDITIONAL([WANT_DEBUG], [test "$enable_debug" = "yes"])


AC_MSG_CHECKING(whether to enable compiler optimizations)
AC_ARG_ENABLE([optimize],
  AS_HELP_STRING([--disable-optimize],
                 [disable compiler optimizations]),
  [case "$enableval" in
    "no" | "yes")  enable_optimize=$enableval;;
    *)             AC_MSG_ERROR(bad value "$enableval" for --enable-optimize);;
  esac],
  [enable_optimize="yes"])
AC_MSG_RESULT($enable_optimize)
AM_CONDITIONAL([WANT_OPTIMIZE], [test "$enable_optimize" = "yes"])

AC_SUBST(EXTRA_CXXFLAGS)
AC_SUBST(DEBUG_CXXFLAGS)
AC_SUBST(OPT_CXXFLAGS)


AC_MSG_CHECKING(how installed files will be laid out)
AC_ARG_WITH([layout],
  [AS_HELP_STRING([--with-layout@<:@=TYPE@:>@],
                  [sets how installed files will be laid out,
                   possible directory layouts: systemc, unix
                   @<:@default=systemc@:>@])],
  [case "$withval" in
    "systemc")  want_install_layout="SystemC";;
    "unix")     want_install_layout="UNIX";;
    *)          want_install_layout="$withval";;
  esac],
  [want_install_layout="SystemC"]
)

abs_builddir=`pwd`
abs_srcdir=`cd "$srcdir"; pwd`

# set default prefix: $srcdir
# preset SystemC directory layout if installing to $srcdir
if test "$prefix" = "NONE"; then
  prefix="$abs_srcdir"
  want_install_layout="SystemC"
fi
AC_MSG_RESULT($want_install_layout)

# strip trailing slash unless $prefix="/"
if test "$prefix" != "/"; then
  prefix=`echo "$prefix" | $AWK '{sub(/\/$/, "")};1'`
fi

if test "$abs_srcdir" != "$abs_builddir"; then
  if test "$prefix" = "$abs_builddir"; then
    AC_MSG_ERROR(Installation to build directory not supported.)
  fi
fi

case "$want_install_layout" in
  "SystemC")
    if test "$datarootdir" = '${prefix}/share'; then
      datarootdir="$prefix"
    fi
    if test "$docdir" = '${datarootdir}/doc/${PACKAGE_TARNAME}'; then
      docdir="$prefix/docs"
      rootdocdir="$prefix"
    else
      rootdocdir="$docdir"
    fi
    have_arch_suffix="default";
    LIB_ARCH_SUFFIX="-${TARGET_ARCH}"
    ;;
  "UNIX")
    # keep Automake/GNU/UNIX defaults
    rootdocdir="$docdir"
    have_arch_suffix="ignored";
    LIB_ARCH_SUFFIX=""
    ;;
  *)
    AC_MSG_ERROR(Unknown directory layout requested.)
    ;;
esac


AC_MSG_CHECKING([for SystemC library arch suffix])
AC_ARG_WITH([arch-suffix],
  [AS_HELP_STRING([--with-arch-suffix@<:@=SUFFIX@:>@],
                  [append SUFFIX to library installation directory
                   @<:@default=-$TARGET_ARCH@:>@])],
  [case "$withval" in
     "no" | "")  have_arch_suffix="no";  LIB_ARCH_SUFFIX="";;
     "yes")      have_arch_suffix="yes"; LIB_ARCH_SUFFIX="-${TARGET_ARCH}";;
     *)          have_arch_suffix="yes"; LIB_ARCH_SUFFIX="$withval";;
  esac],
  [ac_dummy=""]
)
AC_MSG_RESULT($have_arch_suffix (TARGET_ARCH=${TARGET_ARCH}))
AC_SUBST(rootdocdir)
AC_SUBST(examplesdir, [${rootdocdir}/examples])
AC_SUBST([libarchdir],  ['${libdir}${LIB_ARCH_SUFFIX}'] )

AM_CONDITIONAL([SEPARATE_INSTALL_TREE],dnl
  [ test "x${prefix}" != "x${abs_srcdir}" ])
dnl Check for out-of-tree build
AM_CONDITIONAL([SEPARATE_BUILD_TREE],dnl
  [ test "x${abs_srcdir}" != "x${abs_builddir}" ])

#AM_COND_IF([SEPARATE_BUILD_TREE],[
if test -z "${SEPARATE_BUILD_TREE_TRUE}" ; then
  if test "x${prefix}" = "x${abs_builddir}" ; then
    AC_MSG_ERROR([Installation to build directory not supported.])
  fi
fi
#])

AC_ARG_WITH([systemc],
  [AS_HELP_STRING([--with-systemc=DIR],
                  [define path to Accellera SystemC library home])],
  [case "$withval" in
    "no")   have_systemc="no";    systemc_home="";;
    "yes")  have_systemc="yes";   systemc_home="";;
    "")     have_systemc="maybe"; systemc_home="";;
    *)      have_systemc="yes";   systemc_home="$withval";;
  esac],
  [have_systemc="maybe"; systemc_home=""]
)

PKG_CHECK_MODULES(SYSTEMC, [systemc >= 2.3.0],
  [have_systemc="yes"],
  [if test -z "$systemc_home"; then
    AC_MSG_CHECKING(for SystemC library home)
    if test -d "${SYSTEMC_HOME}"; then
      builddir=`pwd`;
      systemc_home=`cd ${SYSTEMC_HOME}; pwd`
      AC_MSG_RESULT(found in \$SYSTEMC_HOME)
    elif test -d "`pwd`/systemc-2.3.1"; then
      builddir=`pwd`;
      systemc_home=`cd $builddir/systemc-2.3.1; pwd`
    elif test -d "`pwd`/systemc-2.3.0"; then
      builddir=`pwd`;
      systemc_home=`cd $builddir/systemc-2.3.0; pwd`
    elif test -d "`pwd`/../systemc-2.3.1"; then
      builddir=`pwd`;
      systemc_home=`cd $builddir/../systemc-2.3.1; pwd`
    elif test -d "`pwd`/../systemc-2.3.0"; then
      builddir=`pwd`;
      systemc_home=`cd $builddir/../systemc-2.3.0; pwd`
    fi
    if test -z "$systemc_home"; then
      have_systemc="no"
      AC_MSG_RESULT(not found)
      AC_MSG_ERROR(SystemC library home not found.
           Specify --with-systemc=/path/to/systemc_home pointing to the installed location.)
    else
      have_systemc="yes"
      AC_MSG_RESULT($systemc_home)
    fi
  fi
  if test "$have_systemc" = "yes"; then
    if test -d "$systemc_home"; then
      systemc_home=`cd $systemc_home; pwd`;
    else
      AC_MSG_ERROR(SystemC library home $systemc_home does not exist.
           SystemC might not have been installed correctly.)
    fi
    SYSTEMC_INC_PREFIX="$systemc_home/include"
    SYSTEMC_LIB_PREFIX="$systemc_home/lib${LIB_ARCH_SUFFIX}"
    AC_MSG_CHECKING(for $SYSTEMC_INC_PREFIX/systemc.h)
    if test -r "$SYSTEMC_INC_PREFIX/systemc.h"; then
      AC_MSG_RESULT(yes)
      SYSTEMC_CFLAGS="-I$SYSTEMC_INC_PREFIX"
      SYSTEMC_LIBS="-L$SYSTEMC_LIB_PREFIX -lsystemc"
    else
      AC_MSG_RESULT(no)
      AC_MSG_ERROR(SystemC header files missing.
           SystemC might not have been installed correctly.)
    fi
  fi]
)


AC_ARG_WITH([regex],
  [AS_HELP_STRING([--with-regex@<:@=ARG@:>@],
                  [select a regular expression library out of {c++11, pcre, posix}
                   @<:@default=posix@:>@])],
  [case "$withval" in
    "c++11")  want_regex="C++11";;
    "pcre")   want_regex="PCRE";;
    "posix")  want_regex="POSIX";;
    *)        want_regex="$withval";;
  esac],
  [want_regex="POSIX"]
)

case "$want_regex" in
  "C++11")
    AX_CXX_COMPILE_STDCXX_11(noext)
    AC_MSG_CHECKING(whether $CXX has a working C++11 regex implementation)
    AC_LANG_PUSH([C++])
    AC_TRY_RUN([#include<regex>
      int main()
      {
        std::regex rx("[(ab)]+", std::regex::extended);
        if( !std::regex_match("abababab", rx) ) exit(1);
        exit(0);
        return 0;
      }],
      [have_regex="yes"],
      [have_regex="no"],
      [have_regex="maybe"]
    )
    AC_LANG_POP([C++])
    AC_MSG_RESULT($have_regex)
    if test "${have_regex}" != "no"; then
      AC_DEFINE(HAVE_CXX11_REGEX, 1, [Define to 1 if you have C++11 regex.])
      AC_SUBST(HAVE_CXX11_REGEX)
    fi
    ;;
  "PCRE")
    PKG_CHECK_MODULES(PCRE, [libpcreposix libpcre],
      [have_regex="yes"
      CPPFLAGS="$PCRE_CFLAGS $CPPFLAGS"; LIBS="$PCRE_LIBS $LIBS"],
      [AC_CHECK_LIB(pcre, pcre_compile,
        [LIBS="-lpcre $LIBS"
        AC_CHECK_LIB(pcreposix, regcomp,
          [have_regex="yes"; LIBS="-lpcreposix $LIBS"],
          [have_regex="no"])],
        [have_regex="no"])]
    )
    if test "${have_regex}" != "no"; then
      AC_DEFINE(HAVE_PCRE_REGEX, 1, [Define to 1 if you have PCRE regex.])
      AC_SUBST(HAVE_PCRE_REGEX)
    fi
    ;;
  "POSIX")
    # Functions regcomp() and regexec() are also in the old V8 regex package.
    # So let us find out who has regfree(). First check -lregex for older
    # UNIX platforms as well as MinGW and Cygwin.
    AC_CHECK_LIB(regex, regfree,
      [have_regex="yes"; LIBS="-lregex $LIBS"],
      [AC_SEARCH_LIBS(regfree, [rx posix],
        [have_regex="yes"
        if test -n "$ac_lib" && test "$ac_lib" != "no"; then
          LIBS="-l$ac_lib $LIBS"
        fi],
        [have_regex="no"])]
    )
    if test "${have_regex}" != "no"; then
      AC_DEFINE(HAVE_POSIX_REGEX, 1, [Define to 1 if you have POSIX regex.])
      AC_SUBST(HAVE_POSIX_REGEX)
    fi
    ;;
  *)
    AC_MSG_ERROR(bad value "${want_regex}" for --with-regex)
    ;;
esac

if test "${have_regex}" = "no"; then
  AC_MSG_ERROR(Library for regular expressions not found.);
fi


# Libtool initialisation (always AFTER all the CC options have been set)
# * explicitly use C++ for libtool initialization since we are always
#   compiling C++ sources
AC_LANG([C++])
# * need to pass EXTRA_CXXFLAGS to Libtool via CXXFLAGS to correctly
#   detect C++ library dependencies, esp. for cross-compilation
stored_CXXFLAGS="${CXXFLAGS}"
CXXFLAGS="${EXTRA_CXXFLAGS} ${CXXFLAGS}"
AC_PROG_LIBTOOL
# * restore CXXFLAGS
CXXFLAGS="${stored_CXXFLAGS}"

#check SystemC Version by building an example program
AC_ARG_ENABLE([systemc_version_check],
    AS_HELP_STRING([--disable-systemc-version-check], [Disable SystemC version check during configure, it is assummed that your SystemC version is IEEE 1666 compliant]))

AS_IF([test "x$enable_systemc_version_check" != "xno"], [
  stored_CXXFLAGS="${CXXFLAGS}"
  stored_LIBS="${LIBS}"
  CXXFLAGS="${SYSTEMC_CFLAGS} ${CXXFLAGS}"
  LIBS="${LIBS} ${SYSTEMC_LIBS}"
  AC_MSG_CHECKING( SystemC Version IEEE compliant)
  AC_COMPILE_IFELSE( 
    [AC_LANG_SOURCE([[#include <systemc>
    int sc_main(int argc, char* argv[]) {
    #if (IEEE_1666_SYSTEMC >= 201101L)
      return 0;
    #else
      #error "Non IEEE 1666 compatible SystemC version found"
    #endif
    return 1;
    }
    ]])]
    ,  
    [AC_MSG_RESULT([IEEE 1666 compatible SystemC version found ])],
    [AC_MSG_FAILURE([Non IEEE 1666 compatible SystemC version found, make sure that SystemC can be found and you build with the same compile options as building SystemC])]
    )
  CXXFLAGS="${stored_CXXFLAGS}"
  LIBS="${stored_LIBS}"
])

AC_SUBST(LIB_ARCH_SUFFIX)

AC_OUTPUT( Makefile
           src/Makefile
           src/uvmsc/Makefile
           src/uvmsc/uvm-systemc.pc
           examples/Makefile
           examples/uvmsc/Makefile
           docs/Makefile
           docs/uvmsc/doxygen/Doxyfile )

# resolve path variables
UVM_SYSTEMC_HOME=$prefix
eval "install_prefix=\"${UVM_SYSTEMC_HOME}\""
eval "install_includedir=\"${includedir}\""
eval "install_includedir=\"${install_includedir}\""
eval "install_libdir=\"${libdir}\""
eval "install_libdir=\"${install_libdir}\""
eval "install_docdir=\"${docdir}\""
eval "install_docdir=\"${install_docdir}\""
eval "install_examplesdir=\"${examplesdir}\""
eval "install_examplesdir=\"${install_examplesdir}\""
eval "install_examplesdir=\"${install_examplesdir}\""
eval "install_examplesdir=\"${install_examplesdir}\""
prefix=$UVM_SYSTEMC_HOME

cat <<EOF | tee -a config.log

------------------------------------------------------------------------------
Configuration summary of $PACKAGE_STRING for $target
------------------------------------------------------------------------------

 Directory setup (based on $want_install_layout layout):
   Installation prefix: ${install_prefix}
   Header files:        ${install_includedir}
   Libraries:           ${install_libdir}
   Documentation:       ${install_docdir}
   Examples:            ${install_examplesdir}

 Target architecture:   ${TARGET_ARCH}

 Build settings:
   Compiler (flags):    ${CXX}
                        ${CXXFLAGS}
   Preprocessor flags:  ${CPPFLAGS}
   SystemC CFLAGS:      ${SYSTEMC_CFLAGS}
   SystemC LIBS:        ${SYSTEMC_LIBS}
   Additional LIBS:     ${LIBS}
   Enable compiler optimizations:   $enable_optimize
   Include debug symbols:           $enable_debug
   Generate instrumentation calls:  $enable_profiling
   Regular expression library:      $want_regex
------------------------------------------------------------------------------
EOF


## Taf!
