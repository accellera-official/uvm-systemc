target_sources (uvm-systemc PRIVATE
  uvm_printer.cpp
  uvm_printer_globals.cpp
  uvm_table_printer.cpp
  uvm_tree_printer.cpp
  uvm_line_printer.h
  uvm_print.h
  uvm_printer.h
  uvm_printer_globals.h
  uvm_printer_knobs.h
  uvm_table_printer.h
  uvm_tree_printer.h)

install_headers (
  uvm_line_printer.h
  uvm_print.h
  uvm_printer.h
  uvm_printer_globals.h
  uvm_printer_knobs.h
  uvm_table_printer.h
  uvm_tree_printer.h)
