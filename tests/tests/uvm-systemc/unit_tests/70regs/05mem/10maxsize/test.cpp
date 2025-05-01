//---------------------------------------------------------------------- 
//   Copyright 2010 Synopsys, Inc. 
//   Copyright 2015 NXP B.V.
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

struct mem : public uvm_mem
{
  mem( const std::string& name = "mem" )
  : uvm_mem(name, 256, 65) {}
}; // class mem


struct blk : public uvm_reg_block
{
  mem m;

  blk( const std::string& name = "blk" )
  : uvm_reg_block(name), m("m")
  {}

  virtual void build()
  {
    m.configure(this);
    lock_model();
  }
}; // class blk


class my_catcher : public uvm_report_catcher
{
public:
  static int seen;

  action_e do_catch() override
  {
    std::string txt = get_message();

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
}; // class my_catcher


// initialize static variable
int my_catcher::seen = 0;


int sc_main(int, char*[])
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_report_server* svr = cs->get_report_server();

  blk b;
  my_catcher c;

  uvm_report_cb::add(nullptr, &c);

  b.build();
  b.print();

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
