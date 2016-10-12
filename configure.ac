AC_INIT([andrii-sevastianov-test], [0.1], [mustitz@gmail.com])
${CXXFLAGS=""}

AM_INIT_AUTOMAKE(dist-bzip2)
AC_CONFIG_MACRO_DIR([m4])

YOO_ENABLE_DEBUG
YOO_WITH_VALGRIND

AC_PROG_CXX
AC_PROG_RANLIB
AM_SILENT_RULES([yes])

AC_CONFIG_FILES([
    Makefile 
    source/Makefile 
    include/Makefile
    validation/Makefile
])

AC_CONFIG_FILES([
    validation/validation.sh
],[
    chmod +x validation/validation.sh
])

AC_OUTPUT
