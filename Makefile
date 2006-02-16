SHELL		:= /bin/sh

# Include local configuration file (generated by ./configure) ########
_CONFIG_MAK	:= ./config.mak
sinclude $(_CONFIG_MAK)


# Makefile variables #################################################
TODAY		:= $(shell date '+%Y-%m-%d')
VERSION_H	:= include/version.h


# Subdirectories #####################################################
SUBDIRS		:= libcommon libmatch
ifeq ($(_enable_gsoap),yes)
SUBDIRS		+= gsoap libsrm2api libsrm2n
endif
SUBDIRS		+=  libptree src doc testing


# Documentation ######################################################
DOCS		:= \
  AUTHORS\
  BUGS\
  COPYING\
  ChangeLog\
  INSTALL\
  README\
  TODO\


# Test cases #########################################################
TEST_DIR		:= testing


# Rules ##############################################################
.PHONY: Makefile clean

ifneq ($(CONFIG_MAK),)
all: $(VERSION_H) $(SUBDIRS)
	@for dir in $(SUBDIRS) ;\
	do \
	  make -C "$$dir" || exit $$? ;\
	done
endif

$(_CONFIG_MAK):
	@echo "Please run ./configure to generate $(_CONFIG_MAK)" >&2
	@exit 1


# Install ############################################################
install: install-subdirs install-docs install-tests

install-subdirs: $(SUBDIRS)
	@for dir in $(SUBDIRS) ;\
	do \
	  make -C "$$dir" install || exit $$? ;\
	done

install-docs: $(DOCS)
	@if test -n "$(DOCS)" ; then\
	  mkdir -p $(prefix)$(_docdir) ;\
	  install -m 0644 $(DOCS) $(prefix)$(_docdir) ;\
	fi

install-tests: $(TEST_DIR)
	@if test -n "$(TEST_DIR)" ; then\
	  mkdir -p $(prefix)$(_docdir) ;\
	  cp -R $(TEST_DIR) $(prefix)$(_docdir) ;\
	fi


# RPM ################################################################
RPM_PACKAGE	= $(_package_prefix)$(PACKAGE)$(_package_suffix)
RELEASE		= $(RPM_PACKAGE)-$(VERSION)
RPMTOPDIR	= $(shell rpm --eval '%_topdir')
BUILDDIR	= $(shell rpm --eval '%_builddir')
BUILDROOT	= $(shell rpm --eval '%_tmppath')/$(RELEASE)-buildroot
RPMBUILD	= rpmbuild --sign

rpm: rpm_build rpm_show

rpm_dirs:
	mkdir -p $(RPMTOPDIR)/{,BUILD,RPMS,SOURCES,SPECS,SRPMS} \
	         $(RPMTOPDIR)/RPMS/{i386,i586,i686,noarch} \
		 $(RPMTOPDIR)/tmp

tar: rpmclean
	-rm -rf $(BUILDROOT)
	-rm -rf $(BUILDDIR)/$(RELEASE)
	mkdir $(BUILDDIR)/$(RELEASE)
	cp -r * $(BUILDDIR)/$(RELEASE)
	cd $(BUILDDIR) ; tar zcvf $(RELEASE).tar.gz\
	  --exclude=CVS --exclude=.svn --exclude=.cvsignore --exclude=.depend\
	  --exclude=1gnore\
	  --exclude='*~' --exclude='#*#' --exclude='20*'\
	  $(RELEASE)
	mv $(BUILDDIR)/$(RELEASE).tar.gz $(RPMTOPDIR)/SOURCES

rpm_build: rpm_dirs tar
	$(RPMBUILD) -ta $(RPMTOPDIR)/SOURCES/$(RELEASE).tar.gz\
	  --define "_name $(RPM_PACKAGE)" \
	  --define "_version $(VERSION)"\
	  --define "_release 1"\
	  --define "_prefix $(_prefix)"

rpm_show: 
	@echo "You have now:" ; ls -l $(RPMTOPDIR)/*RPMS/*/*.rpm


# Versioning #########################################################
$(VERSION_H):
	@echo $(VERSION) > VERSION
	@echo "#define MK_VERSION \"$(VERSION)\"" > $(VERSION_H)
	@echo "#define BUILD_DATE \"$(TODAY)\"" >> $(VERSION_H)


# CVS/Subversion #####################################################
cvsignore:
	@for f in `find . -name .cvsignore` ;\
	do \
	  svn propset svn:ignore -F "$$f" `dirname "$$f"` ;\
	done

ifeq ($(svnroot),)
tag: $(tagfile) $(_CONFIG_MAK)
	@echo "commit..."
	cvs commit
	@echo "tag..."
	cvs rtag v$(subst .,_,$(shell cat $(tagfile))) $(CVS_REPOSITORY)
else
tag: $(tagfile) $(_CONFIG_MAK)
	@echo "commit..."
	svn commit
	@echo "tag..."
	svn copy $(svnroot)/trunk \
	         $(svnroot)/tags/$(shell cat $(tagfile))
endif


# Cleanup ############################################################
clean: clean-subdirs
	-find locale \( -name *.po[~t] -or -name *.mo \) | xargs rm -f
	-rm -f *.log

clean-subdirs:
	@for dir in $(SUBDIRS) ;\
	do \
	  make -C "$$dir" clean || exit $$? ;\
	done

dependclean:
	-find . -name .depend -type f -exec rm -rf {} \;

cvsclean:
	-find . -name CVS -type d -exec rm -rf {} \;

svnclean:
	-find . -name .svn -type d -exec rm -rf {} \;

rpmclean:

update:
	cvs update -d
	rm -rf tests

distclean mclean: clean
	@for dir in $(SUBDIRS) ;\
	do \
	  make -C "$$dir" distclean || exit $$? ;\
	done
	-rm -f $(CONFIG_MAK) $(CONFIG_H) $(VERSION_H) VERSION .rpm*
