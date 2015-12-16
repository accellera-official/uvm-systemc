

#include <systemc>
#include <uvm>

#include "tb_env.h"
#include "tb_test.h"

//----------------------------------------------------------------------
// This example demonstrates how to model a register containing
// read-only fields and a register containing write-only fields
// at the same physical address.
//----------------------------------------------------------------------

int sc_main(int, char*[]) 
{  
  tb_env* env;
  tb_test* test;

  uvm::uvm_root::get()->set_report_verbosity_level(uvm::UVM_FULL);
  uvm::uvm_report_server::get_server()->set_max_quit_count(10);

  env = new tb_env("env");
  test = new tb_test("test");

  uvm::run_test();

  return 0;
}
