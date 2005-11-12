AUTOMAKE_OPTIONS 	= foreign
SUBDIRS 		= include lib src modules
ACLOCAL_AMFLAGS 	= -I autotools
AM_MAKEFLAGS		= -s
dist_bin_SCRIPTS 	= makeconf
dist_data_DATA		= neostats.motd

docdir			= @prefix@/doc
dist_doc_DATA		= doc/FAQ doc/USERMAN

DISTCLEANFILES		= ccdv

EXTRA_DIST		= ccdv.c CREDITS INSTNOTES RELNOTES autotools/shtool

all-local:
		if test ! -f .message; then \
		echo ""; \
		$(top_srcdir)/autotools/shtool echo -e "%BCompilation complete.%b"; \
		echo ""; \
		echo "Run 'make install' (or 'gmake install' on some systems) to install NeoStats."; \
		echo "If you require support, see the README file."; \
		touch .message;\
		fi;

install-exec-hook:
		$(CCDV) $(dist_binSCRIPT_INSTALL) "neostats" "@prefix@/neostats";
		test -z "@prefix@/logs" || $(mkdir_p) "@prefix@/logs";

install-data-hook:
		echo ""
		$(top_srcdir)/autotools/shtool echo -e "%BInstallation complete.%b"
		echo ""
		echo "Change directory to '@prefix@' then run './neostats'"
		echo "If you require support, see the README file."
		rm -f .message

include $(top_srcdir)/autotools/rules.mk
