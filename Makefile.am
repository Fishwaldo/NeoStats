AUTOMAKE_OPTIONS 	= foreign
SUBDIRS 		= include lib src modules
ACLOCAL_AMFLAGS 	= -I autotools
AM_MAKEFLAGS		= -s
dist_bin_SCRIPTS = makeconf
bin_SCRIPTS		= cronchk
dist_data_DATA		= neostats.motd

docdir			= @prefix@/doc
dist_doc_DATA		= doc/* doc/old/*

include $(top_srcdir)/autotools/rules.mk