AC_DEFUN([AX_PATH_LIB_CURL], [dnl
AC_MSG_CHECKING([lib curl])
AC_ARG_WITH(curl,
[  --with-curl[[=prefix]]    compile libcurl part ],,
     with_curl="yes")
if test ".$with_curl" = ".no" ; then
  AC_MSG_RESULT([disabled])
  m4_ifval($2,$2)
  AX_CONFIG_LIB_CURL
else
  AC_MSG_RESULT([(testing)])
  my_cv_curl_vers=NONE
  dnl check is the plain-text version of the required version
  check="7.10.3"
  dnl check_hex must be UPPERCASE if any hex letters are present
  check_hex="070A03"
 
  AC_MSG_CHECKING([for curl >= $check])
 
  if eval curl-config --version 2>/dev/null >/dev/null; then
   	ver=`curl-config --version | sed -e "s/libcurl //g"`
   	hex_ver=`curl-config --vernum | tr 'a-f' 'A-F'`
   	ok=`echo "ibase=16; if($hex_ver>=$check_hex) $hex_ver else 0" | bc`
 
   	if test x$ok != x0; then
     		my_cv_curl_vers="$ver"
     		AC_MSG_RESULT([$my_cv_curl_vers])
		CURL_LIBS=`curl-config --libs`
		if test -d "/usr/include/curl"; then
			CURL_CFLAGS="-I/usr/include/curl"
		else
			CURL_CFLAGS=`curl-config --cflags`
		fi
	     	AM_CONDITIONAL(BUILD_CURL, false)
 	else
     		AC_MSG_RESULT(FAILED)
     		AX_CONFIG_LIB_CURL
   	fi
  else
   	AC_MSG_RESULT(FAILED)
	AX_CONFIG_LIB_CURL
  fi
fi 
AC_SUBST([CURL_LIBS])
AC_SUBST([CURL_CFLAGS])
])

AC_DEFUN([AX_CONFIG_LIB_CURL],[dnl
AC_MSG_CHECKING([How to build a local curl])
dnl CURL options
AC_SYS_LARGEFILE
AC_MSG_CHECKING([if we need -no-undefined])
case $host in
  *-*-cygwin | *-*-mingw* | *-*-pw32*)
    need_no_undefined=yes
    ;;
  *)
    need_no_undefined=no
    ;;
esac
AC_MSG_RESULT($need_no_undefined)
dnl AM_CONDITIONAL(NO_UNDEFINED, test x$need_no_undefined = xyes)

AC_MSG_CHECKING([if we need -mimpure-text])
case $host in
  *-*-solaris2*)
    if test "$GCC" = "yes"; then
      mimpure="yes"
    fi
    ;;
  *)
    mimpure=no
    ;;
esac
AC_MSG_RESULT($mimpure)
dnl AM_CONDITIONAL(MIMPURE, test x$mimpure = xyes)
CURL_CHECK_NONBLOCKING_SOCKET
AC_DEFINE(DISABLED_THREADSAFE, 1, Set to explicitly specify we don't want to use thread-safe functions in curl)
dnl gethostbyname in the nsl lib?
AC_CHECK_FUNC(gethostbyname, , [ AC_CHECK_LIB(nsl, gethostbyname) ])

if test "$ac_cv_lib_nsl_gethostbyname" != "yes" -a "$ac_cv_func_gethostbyname" != "yes"; then
  dnl gethostbyname in the socket lib?
  AC_CHECK_FUNC(gethostbyname, , [ AC_CHECK_LIB(socket, gethostbyname) ])
fi

dnl At least one system has been identified to require BOTH nsl and
dnl socket libs to link properly.
if test "$ac_cv_lib_nsl_gethostbyname" != "yes" -a "$ac_cv_lib_socket_gethostbyname" != "yes" -a "$ac_cv_func_gethostbyname" != "yes"; then
  AC_MSG_CHECKING([trying both nsl and socket libs])
  my_ac_save_LIBS=$LIBS
  LIBS="-lnsl -lsocket $LIBS"
  AC_TRY_LINK( ,
             [gethostbyname();],
             my_ac_link_result=success,
             my_ac_link_result=failure )

  if test "$my_ac_link_result" = "failure"; then
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([couldn't find libraries for gethostbyname()])
    dnl restore LIBS
    LIBS=$my_ac_save_LIBS
  else
    AC_MSG_RESULT([yes])
  fi
fi

dnl resolve lib?
AC_CHECK_FUNC(strcasecmp, , [ AC_CHECK_LIB(resolve, strcasecmp) ])

if test "$ac_cv_lib_resolve_strcasecmp" = "$ac_cv_func_strcasecmp"; then
  AC_CHECK_LIB(resolve, strcasecmp,
              [LIBS="-lresolve $LIBS"],
               ,
               -lnsl)
fi

dnl socket lib?
AC_CHECK_FUNC(connect, , [ AC_CHECK_LIB(socket, connect) ])

AC_MSG_CHECKING([if argv can be written to])
AC_CACHE_VAL(curl_cv_writable_argv, [
AC_RUN_IFELSE([[
int main(int argc, char ** argv) {
	argv[0][0] = ' ';
	return (argv[0][0] == ' ')?0:1;
}
	]],
	curl_cv_writable_argv=yes,
	curl_cv_writable_argv=no,
	curl_cv_writable_argv=cross)
])
case $curl_cv_writable_argv in
yes)
	AC_DEFINE(HAVE_WRITABLE_ARGV, 1, [Define this symbol if your OS supports changing the contents of argv])
	AC_MSG_RESULT(yes)
	;;
no)
	AC_MSG_RESULT(no)
	;;
*)
        AC_MSG_RESULT(no)
        AC_MSG_WARN([the previous check could not be made default was used])
	;;
esac



CURL_LIBS='${top_srcdir}/'src/curl/libcurl.la
CURL_CFLAGS='-I${top_srcdir}/'src/curl
AC_SUBST([CURL_LIBS])
AC_SUBST([CURL_CFLAGS])
AM_CONDITIONAL(BUILD_CURL, true)
AC_MSG_RESULT([ok])
])
