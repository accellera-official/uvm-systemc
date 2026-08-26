include $(top_srcdir)/config/Make-rules

SUBDIRS = \
	docs \
	src \
	examples

DOC_FILES = \
	AUTHORS.md \
	ChangeLog \
	CONTRIBUTING.md \
	INSTALL.md \
	LICENSE \
	NEWS \
	NOTICE \
	README.md \
	RELEASENOTES.md

EXTRA_DIST += \
			  $(DOC_FILES) \
			  \
			  msvc16 \
			  CMakeLists.txt \
			  cmake/UVMSystemCConfig.cmake.in \
			  cmake/UVMSystemCTesting.cmake \
			  cmake/config.h.in \
			  cmake/run-test.py \
			  cmake/uvm-systemc.pc.in \
			  src/CMakeLists.txt \
			  src/uvmsc/CMakeLists.txt \
			  src/uvmsc/base/CMakeLists.txt \
			  src/uvmsc/cb/CMakeLists.txt \
			  src/uvmsc/comps/CMakeLists.txt \
			  src/uvmsc/conf/CMakeLists.txt \
			  src/uvmsc/dap/CMakeLists.txt \
			  src/uvmsc/dpi/CMakeLists.txt \
			  src/uvmsc/factory/CMakeLists.txt \
			  src/uvmsc/macros/CMakeLists.txt \
			  src/uvmsc/misc/CMakeLists.txt \
			  src/uvmsc/phasing/CMakeLists.txt \
			  src/uvmsc/policy/CMakeLists.txt \
			  src/uvmsc/print/CMakeLists.txt \
			  src/uvmsc/reg/CMakeLists.txt \
			  src/uvmsc/reg/sequences/CMakeLists.txt \
			  src/uvmsc/report/CMakeLists.txt \
			  src/uvmsc/seq/CMakeLists.txt \
			  src/uvmsc/tlm1/CMakeLists.txt \
			  tests/CMakeLists.txt

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
