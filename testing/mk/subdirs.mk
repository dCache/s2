.PHONY:	ln test clean

all: ln

help:
	@echo "make"
	@echo "~~~~"
	@echo "ln                    symbolic linking"
	@echo "test                  run all tests in the current directory"
	@echo "clean                 cleanup"

ln:
	@for dir in $(SUBDIRS) ;\
	do \
	  $(MAKE) -C "$$dir" ln || exit $$? ;\
	done

test: ln
	@for dir in $(SUBDIRS) ;\
	do \
	  $(MAKE) -C "$$dir" test || exit $$? ;\
	done

install:

clean:
	@for dir in $(SUBDIRS_CLEAN) ;\
	do \
	  $(MAKE) -C "$$dir" clean || exit $$? ;\
	done

distclean mclean: clean
