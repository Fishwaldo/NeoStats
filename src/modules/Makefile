include ../Makefile.inc

default: all
modules: all
install:
	@for i in $(MODULES) ""; do \
	  if [ "x$$i" != "x" ]; then \
	    echo "===> $(SDP)modules/$$i"; \
		(cd $$i && $(MAKE) $@) || exit 1; \
		echo "<=== $(SDP)modules/$$i"; \
	  fi; \
	done

all clean :: 
	@for i in $(MODULES) ""; do \
	  if [ "x$$i" != "x" ]; then \
	    echo "===> $(SDP)modules/$$i"; \
		(cd $$i && $(MAKE) $@) || exit 1; \
		echo "<=== $(SDP)modules/$$i"; \
	  fi; \
	done

all distclean :: 
	@for i in $(MODULES) ""; do \
	  if [ "x$$i" != "x" ]; then \
	    echo "===> $(SDP)modules/$$i"; \
		(cd $$i && $(MAKE) $@) || exit 1; \
		echo "<=== $(SDP)modules/$$i"; \
	  fi; \
	done
