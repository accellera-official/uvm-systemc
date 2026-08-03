target_sources (uvm-systemc PRIVATE
  uvm_default_report_server.cpp
  uvm_report_catcher.cpp
  uvm_report_catcher_data.cpp
  uvm_report_handler.cpp
  uvm_report_message.cpp
  uvm_report_object.cpp
  uvm_report_server.cpp
  uvm_default_report_server.h
  uvm_report.h
  uvm_report_catcher.h
  uvm_report_catcher_data.h
  uvm_report_handler.h
  uvm_report_message.h
  uvm_report_object.h
  uvm_report_server.h)

install_headers (
  uvm_default_report_server.h
  uvm_report.h
  uvm_report_catcher.h
  uvm_report_catcher_data.h
  uvm_report_handler.h
  uvm_report_message.h
  uvm_report_object.h
  uvm_report_server.h)
