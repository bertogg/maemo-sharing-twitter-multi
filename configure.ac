#######################################
### Initialization and basic checks ###
#######################################
AC_INIT(Makefile.am)
AM_INIT_AUTOMAKE(sharing-twitter-multi, 0.2)
AM_CONFIG_HEADER(config.h)
AC_PROG_CC
AC_PROG_CPP
AC_HEADER_STDC
AC_PROG_INSTALL
AC_ENABLE_SHARED
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
PKG_PROG_PKG_CONFIG()

#####################################
### Basic dependencies - Required ###
#####################################
PKG_CHECK_MODULES(libhildon, hildon-1)
EXTRA_CFLAGS="$EXTRA_CFLAGS $libhildon_CFLAGS"
EXTRA_LIBS="$EXTRA_LIBS $libhildon_LIBS"

PKG_CHECK_MODULES(libhildonmime, libhildonmime)
EXTRA_CFLAGS="$EXTRA_CFLAGS $libhildonmime_CFLAGS"
EXTRA_LIBS="$EXTRA_LIBS $libhildonmime_LIBS"

PKG_CHECK_MODULES(libsharing, libsharing)
EXTRA_CFLAGS="$EXTRA_CFLAGS $libsharing_CFLAGS"
EXTRA_LIBS="$EXTRA_LIBS $libsharing_LIBS"

PKG_CHECK_MODULES(libconic, conic)
EXTRA_CFLAGS="$EXTRA_CFLAGS $libconic_CFLAGS"
EXTRA_LIBS="$EXTRA_LIBS $libconic_LIBS"

PKG_CHECK_MODULES(libcurl, libcurl)
EXTRA_CFLAGS="$EXTRA_CFLAGS $libcurl_CFLAGS"
EXTRA_LIBS="$EXTRA_LIBS $libcurl_LIBS"

PKG_CHECK_MODULES(liboauth, oauth)
EXTRA_CFLAGS="$EXTRA_CFLAGS $liboauth_CFLAGS"
EXTRA_LIBS="$EXTRA_LIBS $liboauth_LIBS"

PKG_CHECK_MODULES(libxml2, libxml-2.0)
EXTRA_CFLAGS="$EXTRA_CFLAGS $libxml2_CFLAGS"
EXTRA_LIBS="$EXTRA_LIBS $libxml2_LIBS"

#################
# Other options #
#################
# Check whether to enable cast checks
AC_ARG_ENABLE(cast-checks, [
  --enable-cast-checks    Turn on cast checks
  --disable-cast-checks   Turn off cast checks (default)],
  enable_cast_checks=$enableval, enable_cast_checks=no)

if test "x$enable_cast_checks" != "xyes"; then
   EXTRA_CFLAGS="$EXTRA_CFLAGS -DG_DISABLE_CAST_CHECKS"
fi

##################################################################
### Everything has been checked, now define Automake variables ###
##################################################################
pluginsdir=`$PKG_CONFIG libsharing --variable=pluginsdir`
servicesdir=`$PKG_CONFIG libsharing --variable=servicesdir`

AC_SUBST(pluginsdir)
AC_SUBST(servicesdir)
AC_SUBST(EXTRA_CFLAGS)
AC_SUBST(EXTRA_LIBS)

############################
### Produce output files ###
############################
AC_CONFIG_FILES([
    Makefile
    data/Makefile
    data/twitter-multi.service.xml
    src/Makefile
])
AC_OUTPUT

######################
### Status summary ###
######################
echo ""
echo "###########################"
echo "$PACKAGE $VERSION"
echo ""
echo "Plug-ins directory: $pluginsdir"
echo "Services directory: $servicesdir"
echo ""
echo "Type 'make' to compile"
echo ""
