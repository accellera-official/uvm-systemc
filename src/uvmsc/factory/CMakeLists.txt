target_sources (uvm-systemc PRIVATE
  uvm_default_factory.cpp
  uvm_factory_override.cpp
  uvm_component_registry.h
  uvm_default_factory.h
  uvm_fact_reg.h
  uvm_factory.h
  uvm_factory_override.h
  uvm_object_registry.h
  uvm_object_wrapper.h)

install_headers (
  uvm_component_registry.h
  uvm_default_factory.h
  uvm_fact_reg.h
  uvm_factory.h
  uvm_factory_override.h
  uvm_object_registry.h
  uvm_object_wrapper.h)
