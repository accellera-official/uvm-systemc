include $(top_srcdir)/config/Make-rules

SUBDIRS = \
	docs \
	src \
	examples

DOC_FILES = \
	AUTHORS \
	ChangeLog \
	INSTALL \
	LICENSE \
	NEWS \
	NOTICE \
	README.md \
	RELEASENOTES

EXTRA_DIST += \
			  $(DOC_FILES) \
			  \
			  msvc14

## ****************************************************************************

## install documentation?
if SEPARATE_INSTALL_TREE

rootdoc_DATA = \
    $(DOC_FILES)

uninstall-hook:
	test ! -d "$(docdir)" || rmdir --ignore-fail-on-non-empty "$(docdir)"
	test ! -d "$(prefix)" || rmdir --ignore-fail-on-non-empty "$(prefix)"

endif # SEPARATE_INSTALL_TREE

## ****************************************************************************

doxygen:
	@(cd docs && $(MAKE) $@)

## Taf!
