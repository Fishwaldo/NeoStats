# Makefile for NeoStats
include src/Makefile.inc 

PROGS = src/neostats cronchk makeconf
CONF = neostats.motd
DOCS = doc/FAQ doc/USERMAN README BUGS AUTHORS COPYING CREDITS TODO \
		ChangeLog doc/README.* src/modules/modules.txt RELNOTES INSTNOTES \
		doc/old/*
DOCS_PROGS = doc/read-faq doc/read-userman
DATA = data/tlds.nfo
BUILDFILES = configure config.sub config.guess *.in install-sh Config \
		makeconf cronchk Makefile

DISTFILES = $(BUILDFILES) $(DATA) $(DOCS) $(DOCS_PROGS) $(CONF)	

SUBDIRS = doc doc/old data logs src src/tools src/modules

all: 
	(cd src; $(MAKE) $@)

clean:
	$(RM) *.cache *.log 
	(cd src; $(MAKE) $@)

distclean:
	$(RM) *.cache *.log 
	$(RM) config.status
	(cd src; $(MAKE) $@)

install: 
	@echo "Installing ..."
	$(INSTALL) -m 755 -d $(DIRECTORY)
	$(INSTALL) -m 755 -d $(DIRECTORY)/modules
	$(INSTALL) -m 755 -d $(DIRECTORY)/include
	$(INSTALL) -m 755 -d $(DIRECTORY)/doc
	$(INSTALL) -m 755 -d $(DIRECTORY)/doc/old
	$(INSTALL) -m 755 -d $(DIRECTORY)/data
	$(INSTALL) -m 750 -d $(DIRECTORY)/logs
	$(INSTALL) -m 755 $(PROGS) $(DIRECTORY)
	$(INSTALL_DATA) $(CONF) $(DIRECTORY)
	$(INSTALL_DATA) $(DOCS) $(DIRECTORY)/doc
	$(INSTALL) -m 755 $(DOCS_PROGS) $(DIRECTORY)/doc
	$(INSTALL_DATA) $(DATA) $(DIRECTORY)/data
	(cd src; $(MAKE) $@)
	@echo "Done."
	@cat INSTNOTES 
	@if test -f INSTNOTES.svn ; then cat INSTNOTES.svn ; fi 

dist: 	
	@echo -n "Creating Directories"
	@$(RM) $(DISTDIR)
	@echo -n "."
	@mkdir $(DISTDIR)
	@echo -n "."
	@mkdir $(DISTDIR)/modules
	@echo -n "."
	@for subdir in $(SUBDIRS); do \
		echo -n "."; \
		mkdir $(DISTDIR)/$$subdir; \
		chmod 777 $(DISTDIR)/$$subdir; \
	done
	@echo "Done"
	@echo -n "Copying Core Distribution files"
	@for file in $(DISTFILES); do \
		echo -n "."; \
		cp -pr $$file $(DISTDIR)/$$file; \
	done
	@$(RM) $(DISTDIR)/configure.in
	@$(RM) $(DISTDIR)/src/config.h
	@echo "Done"
	(cd src; $(MAKE) $@)
	@tar -czf $(DISTDIR).tar.gz $(DISTDIR)/*
#	@$(RM) $(DISTDIR)
	@echo "Tar file $(DISTDIR).tar.gz created. Freshmeat time!"
