.PHONY:	ln test clean

all: ln

help:
	@echo "make [option]"
	@echo "~~~~~~~~~~~~~"
	@echo "help                  this help"
	@echo "ln                    symbolic linking"
	@echo "test                  run all tests in this directory (normal)"
	@echo "fast                  run all tests in this directory (no debug/logs)"
	@echo "gdb                   run all tests in this directory (through gdb)"
	@echo "valgrind              run all tests in this directory (through valgrind)"
	@echo "clean                 cleanup"

ln:
	@for dir in $(SUBDIRS) ;\
	do \
	  $(MAKE) -C "$$dir" $@ || exit $$? ;\
	done

test superfast fast gdb valgrind:
	@echo -e "$(call HTML_HEAD,`basename $(CWD)`)" > $(S2_HTML_LOG)
	@for dir in $(SUBDIRS) ;\
	do \
	  echo -e "<A HREF=$$dir/$(S2_HTML_LOG)>$$dir</A><BR>" >> $(S2_HTML_LOG);\
	  $(MAKE) -C "$$dir" $@ || exit $$? ;\
	done
	@echo -e "$(call HTML_TAIL)" >> $(S2_HTML_LOG)

install:

clean:
	@for dir in $(SUBDIRS_CLEAN) ;\
	do \
	  rm -f $(S2_HTML_LOG) ;\
	  $(MAKE) -C "$$dir" clean || exit $$? ;\
	done

distclean mclean: clean
