//---------------------------------------------------------------------- 
//   Copyright 2010 Synopsys, Inc. 
//   Copyright 2010-2011 Cadence Design Systems, Inc.
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2014 NXP B.V.
//   All Rights Reserved Worldwide 
// 
//   Licensed under the Apache License, Version 2.0 (the 
//   "License"); you may not use this file except in 
//   compliance with the License.  You may obtain a copy of 
//   the License at 
// 
//       http://www.apache.org/licenses/LICENSE-2.0 
// 
//   Unless required by applicable law or agreed to in 
//   writing, software distributed under the License is 
//   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
//   CONDITIONS OF ANY KIND, either express or implied.  See 
//   the License for the specific language governing 
//   permissions and limitations under the License. 
//----------------------------------------------------------------------

#include <systemc>
#include <uvm>
#include <string>
#include <cstdarg>

using namespace uvm;

std::string sformatf(const char* format, ...)
{
  char s[500];
  va_list argptr;
  va_start(argptr, format);
  sprintf(s, format, argptr);
  va_end(argptr);
  return std::string(s);
}

class reg_id : public uvm_reg
{
 public:
  uvm_reg_field* revision_id;
  uvm_reg_field* chip_id;
  uvm_reg_field* product_id;

  reg_id( const std::string& name = "reg_id" ) : uvm_reg(name, 32, UVM_NO_COVERAGE)
  {}

  virtual void build()
  {
    revision_id = uvm_reg_field::type_id::create("revision_id");
    chip_id     = uvm_reg_field::type_id::create("chip_id");
    product_id  = uvm_reg_field::type_id::create("product_id");

    revision_id->configure(this, 8,  0, "RW", false,   0x03, true, false, true);
    chip_id->configure(this, 8,  8, "RW", 0,   0xFF, false, false, true);
    product_id->configure(this, 10, 16, "RW", 0, 0x176, true, false, true);

    revision_id->set_reset(0x30);
    chip_id->set_reset(0x3C, "SOFT");
  }
};

int sc_main(int, char*[])
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_report_server* svr = cs->get_report_server();

  uvm_reg_data_t data; // 2-state data value (no X and Z states)
  reg_id* rg;
  rg = new reg_id();

  rg->build();
   
  rg->revision_id->set(0xFC);
  rg->chip_id->set(0xA5);
  rg->product_id->set(0x289);

  data = rg->get();

  if (data != 0x289A5FC) {
    UVM_ERROR("Test", "Field values were not set");
  }
  else {
    UVM_INFO("Test", "Field values set correctly", UVM_NONE);
  }

  rg->reset("SOFT");
  data = rg->get();

  if (data != 0x2893CFC) {
    UVM_ERROR("Test", sformatf("Soft reset value is 0x%h instead of 0x2893CFC", data.to_int()));
  }
  else {
    UVM_INFO("Test", "Soft reset value is 0x2893CFC", UVM_NONE);
  }

  rg->reset();
  data = rg->get();

  if (data != 0x1763C30) {
    UVM_ERROR("Test", sformatf("Hard reset value is 0x%h instead of 0x1765A30", data.to_int()));
  }
  else {
    UVM_INFO("Test", "Hard reset value is 0x1765A30", UVM_NONE);
  }

  rg->revision_id->set(0xFC);
  rg->chip_id->set(0xA5);
  rg->product_id->set(0x289);

  data = rg->get();

  if (data != 0x289A5FC) {
    UVM_ERROR("Test", "Field values were not set");
  }
  else {
    UVM_INFO("Test", "Field values set correctly", UVM_NONE);
  }

  rg->product_id->has_reset("HARD", 1);
  rg->chip_id->set_reset(0x5A);
  rg->reset();

  data = rg->get();

  if (data != 0x2895A30) {
    UVM_ERROR("Test", sformatf("Hard reset value is 0x%h instead of 0x2895A30", data.to_int()));
  }
  else {
    UVM_INFO("Test", "Hard reset value is 0x2895A30", UVM_NONE);
  }

  rg->print();

  svr->report_summarize();

  if (svr->get_severity_count(UVM_FATAL) +
      svr->get_severity_count(UVM_ERROR) == 0)
    std::cout << "** UVM TEST PASSED **" << std::endl;
  else
    std::cout << "!! UVM TEST FAILED !!" << std::endl;

  return 0;
}
