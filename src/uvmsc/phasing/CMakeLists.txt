target_sources (uvm-systemc PRIVATE
  uvm_bottomup_phase.cpp
  uvm_common_phases.cpp
  uvm_domain.cpp
  uvm_objection.cpp
  uvm_phase.cpp
  uvm_process_phase.cpp
  uvm_runtime_phases.cpp
  uvm_topdown_phase.cpp
  uvm_bottomup_phase.h
  uvm_common_phases.h
  uvm_domain.h
  uvm_objection.h
  uvm_phase.h
  uvm_phase_queue.h
  uvm_phasing.h
  uvm_process.h
  uvm_process_phase.h
  uvm_runtime_phases.h
  uvm_topdown_phase.h)

install_headers (
  uvm_bottomup_phase.h
  uvm_common_phases.h
  uvm_domain.h
  uvm_objection.h
  uvm_phase.h
  uvm_phase_queue.h
  uvm_phasing.h
  uvm_process.h
  uvm_process_phase.h
  uvm_runtime_phases.h
  uvm_topdown_phase.h)
