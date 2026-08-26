###############################################################################
#
# UVM-SystemC component sources
#
# Keep these subdirectories aligned with the authoritative Autotools manifests
# in the adjacent */files.am files.
#
###############################################################################

if (NOT TARGET uvm-systemc)
  message (FATAL_ERROR
          "UVM-SystemC components must be configured from the project root.")
endif ()

add_subdirectory (base)
add_subdirectory (cb)
add_subdirectory (comps)
add_subdirectory (conf)
add_subdirectory (dap)
add_subdirectory (dpi)
add_subdirectory (factory)
add_subdirectory (macros)
add_subdirectory (misc)
add_subdirectory (phasing)
add_subdirectory (policy)
add_subdirectory (print)
add_subdirectory (reg)
add_subdirectory (report)
add_subdirectory (seq)
add_subdirectory (tlm1)
