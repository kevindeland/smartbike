################
##
## nabbed some of the config stuff from hillcrest
##
#####################
AC_INIT([libfreespace-examples],
        [0.6])
AC_CONFIG_AUX_DIR(config)
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE()

AC_PROG_CC
AC_PROG_INSTALL

AC_STDC_HEADERS
AC_PROG_RANLIB

# Check for libfreespace
AC_CHECK_HEADERS([freespace/freespace.h], [], [AC_MSG_FAILURE([*** freespace.h header not found. Check CPPFLAGS])])
AC_CHECK_LIB(freespace, freespace_init, [], [AC_MSG_FAILURE([*** libfreespace not found. Check LDFLAGS])])

# Check for poll support for motion_example_async
AC_CHECK_HEADERS([poll.h])

# Enable CPP tests
AC_ARG_WITH(tests, [  --with-tests Build C++ Tests])

AC_CONFIG_FILES([
   Makefile
   README
   doc/Makefile
   m4/Makefile
   src/Makefile
   win/Makefile
   win/vs2005/Makefile
   win/vs2008/Makefile
   win/vs2010/Makefile
])

if test "$with_tests" == "yes"
then
   TESTS_DIR=tests
   AC_CONFIG_FILES([
      tests/Makefile
   ])   
fi
AC_SUBST([TESTS_DIR])

AC_OUTPUT
