AC_PREREQ([2.67])
AC_INIT( [superdiff], [0.1.0], [snyder.bradleyj@gmail.com])
AM_INIT_AUTOMAKE(superdiff, 0.1.0)
AC_OUTPUT(Makefile src/Makefile)

AC_CONFIG_HEADERS([config.h])
#AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([ncurses], [endwin], [], [AC_MSG_ERROR([Library ncurses not found)])] )
AC_CHECK_LIB([m], [floor], [], [AC_MSG_ERROR([Math library not found)])] )

# Checks for header files.
#AC_CHECK_HEADERS([fcntl.h sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_OUTPUT
