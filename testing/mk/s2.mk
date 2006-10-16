.PHONY:	ln test clean

all: ln

help:
	@echo "make [option]"
	@echo "~~~~~~~~~~~~~"
	@echo "help                  this help"
	@echo "ln                    symbolic linking"
	@echo "test                  run all tests in subdirectories (normal)"
	@echo "fast                  run all tests in subdirectories (no debug/logs)"
	@echo "gdb                   run all tests in subdirectories (through gdb)"
	@echo "valgrind              run all tests in subdirectories (through valgrind)"
	@echo "clean                 cleanup"

ln:
	@for s2 in `ls -1 *.$(S2_EXT) 2>/dev/null` ;\
	do \
	  s2_sh=`basename $$s2 .$(S2_EXT)`.$(SH_EXT) ;\
	  ln -sf $(S2_SH) $$s2_sh;\
	done; \
	if test ! -d $(S2_LOGS_DIR) ; then \
	  mkdir -p $(S2_LOGS_DIR);\
	fi;\
	if test ! -d $(CHECK_DIR) ; then \
	  mkdir -p $(CHECK_DIR);\
	fi

test superfast fast gdb valgrind: clean_log ln
	@echo -e "$(call HTML_HEAD,`basename $(CWD)`)" >> $(S2_LOGS_DIR)/$(S2_HTML_LOG)
	@echo -e "$(call HTML_TABLE_HEAD)" >> $(S2_LOGS_DIR)/$(S2_HTML_LOG)
	@for s2 in `ls -1 *.$(S2_EXT) 2>/dev/null` ;\
	do \
	  s2_bare=`basename $$s2 .$(S2_EXT)` ;\
	  s2_sh=$$s2_bare.$(SH_EXT) ;\
	  s2_out=$(S2_LOGS_DIR)/$$s2_bare.$(OUT_EXT) ;\
	  if test x`./$$s2_sh --s2-bin` = xfalse ; then\
	    echo "Error: s2 binary not found." >&2 ; exit 3;\
	  fi;\
	  if test x$${S2_TIMEOUT} = x ; then\
	    ./$$s2_sh $@;\
	  else\
	    ./$$s2_sh $@ -- --timeout=$${S2_TIMEOUT};\
	  fi;\
	  err=$$?;\
	  echo "$$err ($$s2_sh)" >> $(CHECK_DIR)/$(S2_EXIT_LOG);\
	  $(call HTML_TABLE_ROW,$(S2_LOGS_DIR)/$(S2_HTML_LOG));\
	  if test -f $(CHECK_DIR)/$(S2_EXIT_LOG) ; then\
	    grep "^[^ ]* ($$s2_sh)$$" "$(CHECK_DIR)/$(S2_EXIT_LOG)" >/dev/null 2>&1 ;\
	    if test $$? -ne 0 ; then \
	      echo "Warning: exit code check not performed for $$s2_sh, please update $(CHECK_DIR)/$(S2_EXIT_LOG)" >&2 ;\
	    else\
	      grep "^$$err ($$s2_sh)$$" "$(CHECK_DIR)/$(S2_EXIT_LOG)" >/dev/null 2>&1 ;\
	      err=$$?;\
	      if test $$err -ne 0 ; then \
		echo "Error: exit code ($$err) check failed for $$s2_sh.  Please investigate ($$s2_out) and report." >&2 ; exit $$err;\
	      fi\
	    fi\
	  fi;\
	  if test -f $(CHECK_DIR)/$$s2_out ; then \
	    diff $$s2_out $(CHECK_DIR)/$$s2_out >/dev/null ;\
	    err=$$?;\
	    if test $$err -ne 0 ; then \
	      echo "Error: output check failed!  S2 bug?  Please investigate ($$s2_out) and report." >&2 ; exit $$err;\
	    fi\
	  fi;\
	done
	@echo -e "$(call HTML_TABLE_TAIL)" >> $(S2_LOGS_DIR)/$(S2_HTML_LOG)
	@echo -e "$(call HTML_TAIL)" >> $(S2_LOGS_DIR)/$(S2_HTML_LOG)

install:

clean_log:
	@rm -f $(S2_LOGS)
	@cd $(S2_LOGS_DIR); rm -f $(S2_LOGS)

clean: clean_log
	@find . -name \*.sh -type l -exec rm -f {} \;

distclean mclean: clean
