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
	@echo -e $(call HTML_HEAD,"`basename $(CWD)`") > $(S2_HTML_LOG)
	@for dir in $(SUBDIRS) ;\
	do \
	  echo -e "<A HREF=$$dir/$(S2_HTML_LOG)>$$dir</A><BR>" >> $(S2_HTML_LOG);\
	  $(MAKE) -C "$$dir" test || exit $$? ;\
	done
	@echo -e $(call HTML_TAIL) >> $(S2_HTML_LOG)


install:

clean:
	@for dir in $(SUBDIRS_CLEAN) ;\
	do \
	  $(MAKE) -C "$$dir" clean || exit $$? ;\
	done

distclean mclean: clean
