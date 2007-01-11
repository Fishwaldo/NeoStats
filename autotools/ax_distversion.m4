AC_DEFUN([AX_DISTVERSION], [
AC_ARG_WITH(distversion, [    --with-distversion - Version Name to use for making distributions],
[DISTDIRVERSION="$PACKAGE-$VERSION-$withval"],
[DISTDIRVERSION="$PACKAGE-$VERSION"])
AC_SUBST(DISTDIRVERSION)
])