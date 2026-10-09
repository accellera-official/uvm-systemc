target_sources (uvm-systemc PRIVATE
  uvm_callback.cpp
  uvm_callbacks_base.cpp
  uvm_typeid.cpp
  uvm_callback.h
  uvm_callback_iter.h
  uvm_callbacks.h
  uvm_callbacks_base.h
  uvm_cbs.h
  uvm_derived_callbacks.h
  uvm_typed_callbacks.h
  uvm_typeid.h)

install_headers (
  uvm_callback.h
  uvm_callback_iter.h
  uvm_callbacks.h
  uvm_callbacks_base.h
  uvm_cbs.h
  uvm_derived_callbacks.h
  uvm_typed_callbacks.h
  uvm_typeid.h)
