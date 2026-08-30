// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#include <systemc>
#include <uvm>
#include <crave/ConstrainedRandom.hpp>

#include "dut/jelly_bean_if.h"
#include "dut/jelly_bean_taster.h"
#include "verification/jelly_bean_test.h"

int sc_main(int, char*[])
{
  crave::init("crave.cfg");
  jelly_bean_taster* jb_taster = new jelly_bean_taster("jb_taster");
  jelly_bean_if* jb_if = new jelly_bean_if();

  jb_taster->taste(jb_if->taste);
  jb_taster->sour(jb_if->sour);
  jb_taster->sugar_free(jb_if->sugar_free);
  jb_taster->flavor(jb_if->flavor);
  jb_taster->color(jb_if->color);
  jb_taster->clk(jb_if->clk);

  //uvm_config_db_options::turn_on_tracing();
  uvm_config_db<jelly_bean_if*>::set(0, "*", "interface", jb_if);
  uvm::run_test("jelly_bean_test");
  uvm::print_topology();
  return 0;
}
