AUTOMAKE_OPTIONS 	= foreign
SUBDIRS 		= include lib src modules
ACLOCAL_AMFLAGS 	= -I autotools
AM_MAKEFLAGS		= -s
dist_bin_SCRIPTS 	= makeconf
dist_data_DATA		= neostats.motd

docdir			= @prefix@/doc
dist_doc_DATA		= doc/FAQ doc/README.sqlsrv doc/USERMAN

DISTCLEANFILES		= ccdv

EXTRA_DIST		= ccdv.c CREDITS INSTNOTES README.win32 RELNOTES

all-local:
		clear
		echo "Compilation complete."
		echo "Run 'make install' (or 'gmake install' on some systems) to install NeoStats."
		echo "If you require support, see the README file."

install-exec-hook:
		$(CCDV) $(dist_binSCRIPT_INSTALL) "neostats" "@prefix@/neostats";
		test -z "@prefix@/logs" || $(mkdir_p) "@prefix@/logs";

install-data-hook:
		clear
		echo "Installation complete."
		echo "Change directory to '@prefix@' then run './neostats'"
		echo "If you require support, see the README file."

include $(top_srcdir)/autotools/rules.mk
