SHELL		:= /bin/sh

### Document name and options ########################################
DOCUMENT	:= index
DOCEXT		:= latex
TEX		:= latex
#LOCALE		:= cs_CZ en_GB
LOCALE		:= en_GB
LOCALE_DEF	:= en_GB
DOC_LOCALES	= $(LOCALE:=.$(DOCUMENT).$(DOCEXT))

### (La)TeX 2 HTML software to use ###################################
L2H             = htlatex
#SECTIONING	= html,2,sections+

### WWW specifics ####################################################
HTMLDIR		= html
IMAGEDIR	= images
WEB_SERVER	= shell.sourceforge.net
WEB_PATH	= /home/groups/s/s-/s-2/htdocs
DIR_INSTALL	= /tmp/$(WEB_PATH)


### Project options ##################################################
SPELL_DICT	= uk
BIBLIOGRAPHY	= $(wildcard *.bib)
TEXT_DOC	= $(wildcard *.txt)
LOCAL_SPELL_DICT = $(wildcard .ispell_$(SPELL_DICT))
TEMP_SPELL	= .ispell_$(SPELL_DICT).tmp
ISPELL		= $(SPELL_DICT)spell 
STYLES		= $(wildcard *.sty)
EXTRA_FILES	:=\
  CHECKSUM.SHA1\
  ../{BUGS,COPYING,ChangeLog,INSTALL,README}\
  ../doc/s2.txt 
SRCDEP		= \
  $(DOCUMENT).$(DOCEXT)\
  $(DOC_LOCALES)\
  tex4home.tex\
  $(EXTRA_FILES)\
  ../env/tag\
  $(STYLES) $(BIBLIOGRAPHY) $(TEX4HT_INC)
TEX_JUNK	:= *.log *.toc *.out *.aux *.bbl *.blg *.tmp 
TEX4HT_JUNK	:= *.idv *.lg *.xref *.4ct *.4tc *.fls
MY_JUNK		:= *.spell *.los

### rules ############################################################
.PHONY: clean $(HTMLDIR)

#all: dvi
all: index
index: html

html: $(HTMLDIR)/$(DOCUMENT).html


### HTML #############################################################

$(HTMLDIR)/$(DOCUMENT).html: $(SRCDEP)
	mkdir -p $(HTMLDIR)
	for l in $(LOCALE) ;\
	do \
	  htlatex $$l.$(DOCUMENT).$(DOCEXT) "$(SECTIONING)" "" -d$(HTMLDIR)/ ;\
	  cp $(HTMLDIR)/$$l.$(DOCUMENT)ch1.html $(HTMLDIR)/$$l.$(DOCUMENT).html ;\
	done
	cp $(HTMLDIR)/$(LOCALE_DEF).$(DOCUMENT)ch1.html $(HTMLDIR)/$(DOCUMENT).html
	-rm -f $(DOCUMENT).dvi


### Install on web-server using scp ##################################

web www web_install: html web_install_images
	scp -r $(HTMLDIR)/* $(EXTRA_FILES) $(WEB_SERVER):$(WEB_PATH)

web_install_images: 
	ssh $(WEB_SERVER) mkdir -p $(WEB_PATH)/$(IMAGEDIR)
	-scp `find $(IMAGEDIR) -maxdepth 1 -type f` $(WEB_SERVER):$(WEB_PATH)/$(IMAGEDIR)

web_uninstall_images:
	@for i in `find $(IMAGEDIR) -maxdepth 1 -type f` ;\
	do \
	  ssh $(WEB_SERVER) rm -f $(WEB_PATH)/$$i ;\
	done
	-ssh $(WEB_SERVER) rmdir $(WEB_PATH)/$(IMAGEDIR)

web_uninstall: web_uninstall_images
	ssh $(WEB_SERVER) rm -f $(WEB_PATH)/*.html $(WEB_PATH)/*.css


### Install (locally --- rsync) ######################################

install_images: $(DIR_INSTALL)/$(IMAGEDIR)
$(DIR_INSTALL)/$(IMAGEDIR): $(IMAGEDIR)
	mkdir -p $(DIR_INSTALL)/$(IMAGEDIR)
	-cp `find $(IMAGEDIR) -maxdepth 1 -type f` $(DIR_INSTALL)/$(IMAGEDIR)

uninstall_images: $(IMAGEDIR)
	@for i in `find $(IMAGEDIR) -maxdepth 1 -type f` ;\
	do \
	  rm -f $(DIR_INSTALL)/$$i ;\
	done
	-rmdir $(DIR_INSTALL)/$(IMAGEDIR)

install: album install_images $(DIR_INSTALL)/$(DOCUMENT).html
$(DIR_INSTALL)/$(DOCUMENT).html: $(HTMLDIR)/$(DOCUMENT).html
	cp -a $(HTMLDIR)/* $(EXTRA_FILES) $(DIR_INSTALL)

uninstall: uninstall_images
	rm -f $(DIR_INSTALL)/*.html\
	      $(DIR_INSTALL)/*.css


### Cleanup ##########################################################

texclean: 
	-rm -f $(TEX_JUNK) $(TEX4HT_JUNK) $(MY_JUNK) 2>/dev/null

# Make sure we clean things properly, but not the current dir!
htmlclean:
	-rm -f $(DOCUMENT).htm* $(DOCUMENT).css
	@if test -r "$(HTMLDIR)/$(DOCUMENT).$(DOCEXT)" ; then \
	  echo "Sorry, \`$(DOCUMENT).$(DOCEXT)' found in \`$(HTMLDIR)', cowardly refusing to clean." ; \
	else \
	  rm -rf $(HTMLDIR) ; \
	fi

clean distclean mclean: texclean htmlclean
	-rm -f *.dvi *.pdf *.ps *.ps.gz *.lj *.pcl *.htm* *.css 2>/dev/null
