AUTOMAKE_OPTIONS 	= foreign
SUBDIRS 		= include lib src modules
ACLOCAL_AMFLAGS 	= -I autotools
AM_MAKEFLAGS		= -s
dist_bin_SCRIPTS 	= makeconf
dist_data_DATA		= neostats.motd

docdir			= @prefix@/doc
dist_doc_DATA		= doc/old/* doc/*

DISTCLEANFILES		= ccdv

EXTRA_DIST		= ccdv.c CREDITS INSTNOTES README.win32 RELNOTES Config \
			  ChangeLog.old

install-exec-hook:
		$(CCDV) $(dist_binSCRIPT_INSTALL) "neostats" "@prefix@/neostats";
		test -z "@prefix@/logs" || $(mkdir_p) "@prefix@/logs";
include $(top_srcdir)/autotools/rules.mk