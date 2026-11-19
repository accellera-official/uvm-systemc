//---------------------------------------------------------------------- 
//   Copyright 2010 Synopsys, Inc. 
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
#include <cstring>
#include <cstdlib>

using namespace uvm;


class reg_id : public uvm_reg
{
 public:
  uvm_reg_field* product_id;

  reg_id( const std::string& name = "reg_id") : uvm_reg(name,32,UVM_NO_COVERAGE)
  {}

  void build()
  {
    product_id = uvm_reg_field::type_id::create("product_id");
    product_id->configure(this, 65, 16,"RW", 0, 0x176, 1, 0, 1);
  }
};

class blk : public uvm_reg_block
{
 public:
   reg_id* id;

  blk( const std::string& name = "blk") : uvm_reg_block(name)
   {}

  virtual void build()
  {
    id = new reg_id("id");
    id->configure(this, nullptr, "");
    id->build();
    lock_model();
  }
};


class my_catcher : public uvm_report_catcher
{
 public:
  static int seen;

  my_catcher(){}

  action_e do_catch() override
  {
    std::string txt = get_message();

    if (get_severity() == UVM_ERROR
      && get_id() == "RegModel")
    {
      std::cout << "Error message caught: " << txt << std::endl;
      if (strstr(txt.c_str(), "Fields use more bits"))
      {
        set_severity(UVM_WARNING);
        set_action(UVM_DISPLAY);
      }
      return THROW;
    }

    if (get_severity() == UVM_FATAL &&
        get_id() == "RegModel")
    {
      std::cout << "Fatal message caught: " << txt << std::endl;
      if (strstr(txt.c_str(), "UVM_REG_DATA_WIDTH"))
      {
        seen++;
        set_severity(UVM_WARNING);
        set_action(UVM_DISPLAY);
        return THROW;
      }
    }
    return THROW;
  }
};

// initialize static variable
int my_catcher::seen = 0;

int sc_main(int, char*[])
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_report_server* svr = cs->get_report_server();

  blk* b = new blk();
  my_catcher* c = new my_catcher();

  uvm_report_cb::add(nullptr, c);

  b->build();
   
  if (my_catcher::seen != 1)
    UVM_ERROR("Test", "Fatal message about invalid UVM_REG_DATA_WIDTH value not seen");

  svr->report_summarize();

  if (svr->get_severity_count(UVM_FATAL) +
      svr->get_severity_count(UVM_ERROR) == 0)
    std::cout << "** UVM TEST PASSED **" << std::endl;
  else
    std::cout <<"!! UVM TEST FAILED !!" << std::endl;

  return 0;
}
