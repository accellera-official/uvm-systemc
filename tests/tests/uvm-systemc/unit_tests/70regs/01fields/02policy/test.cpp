//---------------------------------------------------------------------- 
//   Copyright 2010-2011 Synopsys, Inc. 
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2011 Cadence Design Systems, Inc.
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
#include <cstdlib>
#include "global.h"

using namespace uvm;
using namespace sc_core;

class a_blk : public uvm_reg_block
{
 public:
  a_blk( const std::string& name = "a_blk") : uvm_reg_block(name)
  {}
};

class a_reg : public uvm_reg
{
 public:
  uvm_reg_field* DC;
  uvm_reg_field* WO1;
  uvm_reg_field* W1;
  uvm_reg_field* WOS;
  uvm_reg_field* WOC;
  uvm_reg_field* WO;
  uvm_reg_field* W0CRS;
  uvm_reg_field* W0SRC;
  uvm_reg_field* W1CRS;
  uvm_reg_field* W1SRC;
  uvm_reg_field* W0T;
  uvm_reg_field* W0S;
  uvm_reg_field* W0C;
  uvm_reg_field* W1T;
  uvm_reg_field* W1S;
  uvm_reg_field* W1C;
  uvm_reg_field* WCRS;
  uvm_reg_field* WSRC;
  uvm_reg_field* WS;
  uvm_reg_field* WC;
  uvm_reg_field* WRS;
  uvm_reg_field* WRC;
  uvm_reg_field* RS;
  uvm_reg_field* RC;
  uvm_reg_field* RW;
  uvm_reg_field* RO;

  a_reg( const std::string& name = "a_reg") : uvm_reg(name, 64, UVM_NO_COVERAGE)
  {}

  virtual void build()
  {
    DC    = uvm_reg_field::type_id::create("DC");
    WO1   = uvm_reg_field::type_id::create("WO1");
    W1    = uvm_reg_field::type_id::create("W1");
    WOS   = uvm_reg_field::type_id::create("WOS");
    WOC   = uvm_reg_field::type_id::create("WOC");
    WO    = uvm_reg_field::type_id::create("WO");
    W0CRS = uvm_reg_field::type_id::create("W0CRS");
    W0SRC = uvm_reg_field::type_id::create("W0SRC");
    W1CRS = uvm_reg_field::type_id::create("W1CRS");
    W1SRC = uvm_reg_field::type_id::create("W1SRC");
    W0T   = uvm_reg_field::type_id::create("W0T");
    W0S   = uvm_reg_field::type_id::create("W0S");
    W0C   = uvm_reg_field::type_id::create("W0C");
    W1T   = uvm_reg_field::type_id::create("W1T");
    W1S   = uvm_reg_field::type_id::create("W1S");
    W1C   = uvm_reg_field::type_id::create("W1C");
    WCRS  = uvm_reg_field::type_id::create("WCRS");
    WSRC  = uvm_reg_field::type_id::create("WSRC");
    WS    = uvm_reg_field::type_id::create("WS");
    WC    = uvm_reg_field::type_id::create("WC");
    WRS   = uvm_reg_field::type_id::create("WRS");
    WRC   = uvm_reg_field::type_id::create("WRC");
    RS    = uvm_reg_field::type_id::create("RS");
    RC    = uvm_reg_field::type_id::create("RC");
    RW    = uvm_reg_field::type_id::create("RW");
    RO    = uvm_reg_field::type_id::create("RO");

       DC->configure(this, 2, 50, "RW",    0, 0x01, 1, 0, 0);
       DC->set_compare(UVM_NO_CHECK);
      WO1->configure(this, 2, 48, "WO1",   0, 0x01, 1, 0, 0);
       W1->configure(this, 2, 46, "W1",    0, 0x01, 1, 0, 0);
      WOS->configure(this, 2, 44, "WOS",   0, 0x01, 1, 0, 0);
      WOC->configure(this, 2, 42, "WOC",   0, 0x01, 1, 0, 0);
       WO->configure(this, 2, 40, "WO",    0, 0x01, 1, 0, 0);
    W0CRS->configure(this, 2, 38, "W0CRS", 0, 0x01, 1, 0, 0);
    W0SRC->configure(this, 2, 36, "W0SRC", 0, 0x01, 1, 0, 0);
    W1CRS->configure(this, 2, 34, "W1CRS", 0, 0x01, 1, 0, 0);
    W1SRC->configure(this, 2, 32, "W1SRC", 0, 0x01, 1, 0, 0);
      W0T->configure(this, 2, 30, "W0T",   0, 0x01, 1, 0, 0);
      W0S->configure(this, 2, 28, "W0S",   0, 0x01, 1, 0, 0);
      W0C->configure(this, 2, 26, "W0C",   0, 0x01, 1, 0, 0);
      W1T->configure(this, 2, 24, "W1T",   0, 0x01, 1, 0, 0);
      W1S->configure(this, 2, 22, "W1S",   0, 0x01, 1, 0, 0);
      W1C->configure(this, 2, 20, "W1C",   0, 0x01, 1, 0, 0);
     WCRS->configure(this, 2, 18, "WCRS",  0, 0x01, 1, 0, 0);
     WSRC->configure(this, 2, 16, "WSRC",  0, 0x01, 1, 0, 0);
       WS->configure(this, 2, 14, "WS",    0, 0x01, 1, 0, 0);
       WC->configure(this, 2, 12, "WC",    0, 0x01, 1, 0, 0);
      WRS->configure(this, 2, 10, "WRS",   0, 0x01, 1, 0, 0);
      WRC->configure(this, 2,  8, "WRC",   0, 0x01, 1, 0, 0);
       RS->configure(this, 2,  6, "RS",    0, 0x01, 1, 0, 0);
       RC->configure(this, 2,  4, "RC",    0, 0x01, 1, 0, 0);
       RW->configure(this, 2,  2, "RW",    0, 0x01, 1, 0, 0);
       RO->configure(this, 2,  0, "RO",    0, 0x01, 1, 0, 0);
  } // build
};

// Use a user-defined front-door to model the DUT instead
class dut : public uvm_reg_frontdoor
{
 public:
  uvm_reg_data_t R; // default width is 64 bits
  bool written;

  dut( const std::string& name="dut") : uvm_reg_frontdoor(name)
  {
    reset();
  }

  void reset()
  {
    R = 0x0005555555555555; // 0000 0000 0000 0101 0101 0101 0101 0101 0101 0101 0101 0101 0101 0101 0101 0101
    written = false;
  }

   void body() override
  {
    uvm_reg_data_t data;

    if (rw_info->access_kind == UVM_WRITE) // note: kind replaced by access_kind in UVM-SystemC
    {
      data = rw_info->value[0];

      R.range(51,50) = data.range(51,50);                              // DC
      R.range(49,48) = (written) ? R.range(49,48) : data.range(49,48); // WO1

      R.range(47,46) = (written) ? R.range(47,46) : data.range(47,46); // W1
      R.range(45,44) = 0b11;                                // WOS
      R.range(43,42) = 0b00;                                // WOC
      R.range(41,40) = data.range(41,40);                   // WO
      R.range(39,38) = R.range(39,38) &  data.range(39,38); // W0CRS
      R.range(37,36) = R.range(37,36) | ~data.range(37,36); // W0SRC
      R.range(35,34) = R.range(35,34) & ~data.range(35,34); // W1CRS
      R.range(33,32) = R.range(33,32) |  data.range(33,32); // W1SRC

      R.range(31,30) = R.range(31,30) ^ ~data.range(31,30); // W0T
      R.range(29,28) = R.range(29,28) | ~data.range(29,28); // W0S
      R.range(27,26) = R.range(27,26) &  data.range(27,26); // W0C
      R.range(25,24) = R.range(25,24) ^  data.range(25,24); // W1T
      R.range(23,22) = R.range(23,22) |  data.range(23,22); // W1S
      R.range(21,20) = R.range(21,20) & ~data.range(21,20); // W1C
      R.range(19,18) = 0b00;                                // WCRS
      R.range(17,16) = 0b11;                                // WSRC

      R.range(15,14) = 0b11;                                // WS
      R.range(13,12) = 0b00;                                // WC
      R.range(11,10) = data.range(11,10);                   // WRS
      R.range( 9, 8) = data.range( 9, 8);                   // WRC
      R.range( 7, 6) = R.range( 7, 6);                      // RS
      R.range( 5, 4) = R.range( 5, 4);                      // RC
      R.range( 3, 2) = data.range( 3, 2);                   // RW
      R.range( 1, 0) = R.range( 1, 0);                      // RO

      written = true;
    }
    else // read
    {
      data = 0;
         
      data.range(51,50) = pseudo_rand() % 255;                   // DC
      data.range(49,48) = 0b00;                                  // WO1
      data.range(47,46) = R.range(47,46);                        // W1
      data.range(45,44) = 0b00;                                  // WOS
      data.range(43,42) = 0b00;                                  // WOC
      data.range(41,40) = 0b00;                                  // WO
      data.range(39,38) = R.range(39,38); R.range(39,38) = 0b11; // W0CRS
      data.range(37,36) = R.range(37,36); R.range(37,36) = 0b00; // W0SRC
      data.range(35,34) = R.range(35,34); R.range(35,34) = 0b11; // W1CRS
      data.range(33,32) = R.range(33,32); R.range(33,32) = 0b00; // W1SRC
      data.range(31,30) = R.range(31,30);                        // W0T
      data.range(29,28) = R.range(29,28);                        // W0S
      data.range(27,26) = R.range(27,26);                        // W0C
      data.range(25,24) = R.range(25,24);                        // W1T
      data.range(23,22) = R.range(23,22);                        // W1S
      data.range(21,20) = R.range(21,20);                        // W1C
      data.range(19,18) = R.range(19,18); R.range(19,18) = 0b11; // WCRS
      data.range(17,16) = R.range(17,16); R.range(17,16) = 0b00; // WSRC
      data.range(15,14) = R.range(15,14);                        // WS
      data.range(13,12) = R.range(13,12);                        // WC
      data.range(11,10) = R.range(11,10); R.range(11,10) = 0b11; // WRS
      data.range( 9, 8) = R.range( 9, 8); R.range( 9, 8) = 0b00; // WRC
      data.range( 7, 6) = R.range( 7, 6); R.range( 7, 6) = 0b11; // RS
      data.range( 5, 4) = R.range( 5, 4); R.range( 5, 4) = 0b00; // RC
      data.range( 3, 2) = R.range( 3, 2);                        // RW
      data.range( 1, 0) = R.range( 1, 0);                        // RO

      rw_info->value[0] = data;
    }
  } // body
};

class test : public uvm_test
{
 public:
  a_blk* blk;
  a_reg* rg;
  dut*   fd;
  uvm_reg_data_t data;

  test( uvm_component_name name ) : uvm_test(name)
  {
    blk = new a_blk("blk");
    rg = new a_reg("rg");
    fd = new dut();
  }

  UVM_COMPONENT_UTILS(test);

  void build_phase(uvm::uvm_phase& phase) override
  {
    blk->default_map = blk->create_map("map", 0, 8, UVM_BIG_ENDIAN);
    rg->build();
    rg->configure(blk);
    blk->default_map->add_reg(rg, 0, "RW", 1, fd);
    blk->default_map->set_auto_predict();
    rg->reset();
    blk->print();
    blk->lock_model();
  }

  void run_phase(uvm::uvm_phase& phase) override
  {
    phase.raise_objection(this);
    uvm_reg_single_bit_bash_seq* seq;
    seq = new uvm_reg_single_bit_bash_seq();
    seq->rg = rg;
    seq->start(nullptr);
    phase.drop_objection(this);
  }
};

int sc_main(int, char*[])
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_report_server* svr = cs->get_report_server();

  // replace with command line verbosity setting
  uvm_root* top = uvm_root::get();
  top->set_report_verbosity_level_hier(UVM_FULL);

  run_test("test");

  if (svr->get_severity_count(UVM_FATAL) +
      svr->get_severity_count(UVM_ERROR) == 0)
    std::cout << "** UVM TEST PASSED **" << std::endl;
  else
    std::cout <<"!! UVM TEST FAILED !!" << std::endl;

  return 0;
}
