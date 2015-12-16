include $(top_srcdir)/config/Make-rules.examples

all:
	@echo "To compile and run the examples call"
	@echo "   make check "


# begin with empty variables, is filled later in test.am files
examples_BUILD = 
examples_CLEAN = 
examples_FILES = 
examples_TESTS = 
examples_XFAIL_TESTS = 


# include simple tests
include $(srcdir)/simple/callbacks/basic/test.am
include $(srcdir)/simple/configuration/manual/test.am
include $(srcdir)/simple/factory/basic/test.am
include $(srcdir)/simple/hello_world/test.am
include $(srcdir)/simple/objections/basic/test.am
include $(srcdir)/simple/phases/basic/test.am
include $(srcdir)/simple/phases/jump/test.am
include $(srcdir)/simple/phases/runtime/test.am
include $(srcdir)/simple/phases/timeout/test.am
include $(srcdir)/simple/producer_consumer/basic/test.am
include $(srcdir)/simple/producer_consumer/override/test.am
include $(srcdir)/simple/registers/models/aliasing/test.am
include $(srcdir)/simple/registers/models/ro_wo_same_addr/test.am
include $(srcdir)/simple/reporting/error/test.am
include $(srcdir)/simple/reporting/info/test.am
include $(srcdir)/simple/reporting/warning/test.am
include $(srcdir)/simple/scoreboard/basic/test.am
include $(srcdir)/simple/sequence/arbitration/test.am
include $(srcdir)/simple/sequence/basic_read_write_sequence/test.am
include $(srcdir)/simple/sequence/basic_read_write_sequence_tlm1/test.am
include $(srcdir)/simple/sequence/basic_read_write_sequence_try/test.am
include $(srcdir)/simple/sequence/sequence_lock_grab/test.am
include $(srcdir)/simple/tlm1/bidir/test.am
include $(srcdir)/simple/tlm1/hierarchy/test.am
include $(srcdir)/simple/tlm1/producer_consumer/test.am
include $(srcdir)/simple/trivial/test.am


CLEANFILES += $(examples_CLEAN)

check_PROGRAMS = $(examples_TESTS)

TESTS = $(check_PROGRAMS)

XFAIL_TESTS = $(examples_XFAIL_TESTS)

install-data-local:
	@$(NORMAL_INSTALL)
	if test "$(realpath $(srcdir))" != "$(realpath $(DESTDIR)$(examplesdir)/uvmsc)"; then \
	  $(mkinstalldirs) $(DESTDIR)$(examplesdir)/uvmsc; \
	  for file in $(examples_FILES); do \
	    $(INSTALL_DATA) -D $(srcdir)/$$file $(DESTDIR)$(examplesdir)/uvmsc/$$file; \
	  done; \
	fi

uninstall-local:
	@$(NORMAL_UNINSTALL)
	if test "$(realpath $(srcdir))" != "$(realpath $(DESTDIR)$(examplesdir)/uvmsc)"; then \
	  for file in $(examples_FILES); do \
	    rm -f $(DESTDIR)$(examplesdir)/uvmsc/$$file; \
	  done; \
	  rm -rf $(DESTDIR)$(examplesdir)/uvmsc; \
	fi

## Taf!
