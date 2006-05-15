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
	rm -f $(S2_TEST_REPORT)
	for s2 in `ls -1 *.$(S2_EXT) 2>/dev/null` ;\
	do \
	  s2_sh=`basename $$s2 .$(S2_EXT)`.$(SH_EXT) ;\
	  s2_out=`basename $$s2 .$(S2_EXT)`.$(OUT_EXT) ;\
	  ./$$s2_sh;\
	  if test -f $(CHECK_DIR)/$$s2_out ; then \
	    diff $$s2_out $(CHECK_DIR)/$$s2_out >/dev/null ;\
	    err=$$?;\
	    if test $$err -ne 0 ; then \
	      echo "Output check failed!  S2 bug?  Please check and report." >&2 ; exit $$err;\
	    fi\
	  fi;\
	  echo "$$? ($$s2_sh)" >> $(S2_TEST_REPORT);\
	done

install:

clean:
	find . -name \*.sh -type l -exec rm -f {} \;
	rm -f $(S2_LOGS)

distclean mclean: clean
