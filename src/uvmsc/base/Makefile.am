include $(top_srcdir)/config/Make-rules

H_FILES = \
	uvm_base.h \
	uvm_component.h \
	uvm_component_name.h \
	uvm_coreservice_t.h \
	uvm_default_coreservice_t.h \
	uvm_event_callback.h \
	uvm_event.h \
	uvm_export_base.h \
	uvm_globals.h \
	uvm_mutex.h \
	uvm_object.h \
	uvm_object_globals.h \
	uvm_port_base.h \
	uvm_root.h \
	uvm_transaction.h \
	uvm_void.h \
	uvm_version.h

NO_H_FILES = 

noinst_HEADERS = $(H_FILES)

CXX_FILES = \
	uvm_component.cpp \
	uvm_component_name.cpp \
	uvm_coreservice_t.cpp \
	uvm_default_coreservice_t.cpp \
	uvm_event_callback.cpp \
	uvm_event.cpp \
	uvm_globals.cpp \
	uvm_mutex.cpp \
	uvm_object.cpp \
	uvm_object_globals.cpp \
	uvm_root.cpp \
	uvm_transaction.cpp \
	uvm_version.cpp

NO_CXX_FILES =

noinst_LTLIBRARIES = libbase.la

libbase_la_SOURCES = $(H_FILES) $(NO_H_FILES) $(CXX_FILES)

INCLUDE_DIR = $(prefix)/include/uvmsc/base

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(INCLUDE_DIR)
	for file in $(noinst_HEADERS); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(DESTDIR)$(INCLUDE_DIR)/$$file; \
	done

uninstall-local:
	@$(NORMAL_UNINSTALL)
	for file in $(noinst_HEADERS); do \
	  rm -f $(DESTDIR)$(INCLUDE_DIR)/$$file; \
	done
	-rmdir $(DESTDIR)$(INCLUDE_DIR)

## Taf!
