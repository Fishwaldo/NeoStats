AUTOMAKE_OPTIONS 	= foreign
SUBDIRS 		= include lib modules tools src
ACLOCAL_AMFLAGS 	= -I autotools

bin_SCRIPTS		= makeconf cronchk
data_DATA		= neostats.motd

docdir			= @prefix@/doc
doc_DATA		= doc/* doc/old/*

all-local:
	@touch include/version.sh

include $(top_srcdir)/autotools/rules.mk