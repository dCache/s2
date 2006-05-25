.PHONY:	ln test clean

all: ln

help:
	@echo "make"
	@echo "~~~~"
	@echo "ln                    symbolic linking"
	@echo "test                  tests"
	@echo "clean                 cleanup"

ln:
	@for s2 in `ls -1 *.$(S2_EXT) 2>/dev/null` ;\
	do \
	  s2_sh=`basename $$s2 .$(S2_EXT)`.$(SH_EXT) ;\
	  ln -sf $(S2_SH) $$s2_sh;\
	done

test: ln
	@rm -f $(S2_EXIT_LOG) $(S2_HTML_LOG)
	@echo -e "$(call HTML_HEAD,`basename $(CWD)`)" >> $(S2_HTML_LOG)
	@echo -e "$(call HTML_TABLE_HEAD)" >> $(S2_HTML_LOG)
	@for s2 in `ls -1 *.$(S2_EXT) 2>/dev/null` ;\
	do \
	  s2_bare=`basename $$s2 .$(S2_EXT)` ;\
	  s2_sh=$$s2_bare.$(SH_EXT) ;\
	  s2_out=$$s2_bare.$(OUT_EXT) ;\
	  ./$$s2_sh;\
          err=$$?;\
	  echo "$$err ($$s2_sh)" >> $(S2_EXIT_LOG);\
	  $(call HTML_TABLE_ROW,$(S2_HTML_LOG),$$s2_bare,$$err);\
	  if test -f $(CHECK_DIR)/$$s2_out ; then \
	    diff $$s2_out $(CHECK_DIR)/$$s2_out >/dev/null ;\
	    err=$$?;\
	    if test $$err -ne 0 ; then \
	      echo "Output check failed!  S2 bug?  Please investigate ($$s2_out) and report." >&2 ; exit $$err;\
	    fi\
	  fi;\
	done;\
	if test -f $(CHECK_DIR)/$(S2_EXIT_LOG) ; then \
	  diff -U0 $(S2_EXIT_LOG) $(CHECK_DIR)/$(S2_EXIT_LOG) ;\
	  err=$$?;\
	  if test $$err -ne 0 ; then \
	    echo "Exit code check(s) failed!  S2 bug?  Please compare $(S2_EXIT_LOG), $(CHECK_DIR)/$(S2_EXIT_LOG) and report." >&2 ; exit $$err;\
	  fi\
	fi
	@echo -e "$(call HTML_TABLE_TAIL)" >> $(S2_HTML_LOG)
	@echo -e "$(call HTML_TAIL)" >> $(S2_HTML_LOG)


install:

clean:
	find . -name \*.sh -type l -exec rm -f {} \;
	rm -f $(S2_LOGS)

distclean mclean: clean
