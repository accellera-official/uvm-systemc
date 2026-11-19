//---------------------------------------------------------------------- 
//   Copyright 2016 NXP B.V.
//   Copyright 2010 Synopsys, Inc. 
//   Copyright 2010-2011 Mentor Graphics Corporation
//   Copyright 2010-2011 Cadence Design Systems, Inc.
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

#include "reg.h"

using namespace std;
using namespace uvm;
using namespace sc_core;
using namespace sc_dt;

typedef enum { READ, DEPOSIT, FORCE, RELEASE } op_e;

const char* op_name[] = {
    "READ", "DEPOSIT", "FORCE", "RELEASE"};

class dut_t : public sc_module
{
public:

  // regular sc_signal supporting multiple drivers
  // since both the dut as well as the deposit routine
  // writes to this channel
  sc_signal<sc_uint<8>, SC_MANY_WRITERS> w;

  // UVM dummy registers dedicated to SystemC DUT
  reg<sc_uint<8> > q;
  reg<sc_uint<8> > d;

  SC_HAS_PROCESS(dut_t);

  dut_t(sc_module_name nm)
  : sc_module(nm),
  w("w"),
  q("q"),
  d("d")
  {
    SC_THREAD(init);

    SC_THREAD(update_q_with_d);

    SC_METHOD(update_w_with_q);
    sensitive << q.default_event();
    dont_initialize();
  }

  void init()
  {
    // initialize registers
    q = 0x0F;
    d = 0xF0;
  }

  void update_w_with_q()
  {
    w = q.to_uint();
  }

  void update_q_with_d()
  {
    while(true) // forever
    {
      wait(100, SC_MS);
      q = d;
    }
  }
}; // class dut_t


class test : public uvm_test
{
 public:
  UVM_COMPONENT_UTILS(test);

  test( uvm_component_name name ) : uvm_test(name)
  {}

  void op( op_e oper,
      std::string hdl_path,
      sc_uint<8> wr_val = 0,
      sc_uint<8> exp_val = 0,
      int lineno = 0,
      sc_time force_time = SC_ZERO_TIME)
  {
    sc_uint<8> rd_val = 0;

    if (oper == DEPOSIT)
    {
      if (!uvm_hdl_deposit(hdl_path, wr_val))
        UVM_ERROR(hdl_path, "uvm_hdl_deposit returned FALSE");
    }
    else if (oper == FORCE)
    {
      if (force_time == SC_ZERO_TIME)
      {
        if (!uvm_hdl_force(hdl_path, wr_val))
          UVM_ERROR(hdl_path, "uvm_hdl_force returned FALSE");
      }
      else
      {
        uvm_hdl_force_time(hdl_path, wr_val, force_time);
      }
    }
    else if (oper == RELEASE)
    {
      if (!uvm_hdl_release(hdl_path))
        UVM_ERROR(hdl_path, "uvm_hdl_release returned FALSE");
    }

    // before any read, make sure the evaluate-update cycle is completed
    sc_core::wait(SC_ZERO_TIME);

    if (!uvm_hdl_read(hdl_path, rd_val))
      UVM_ERROR(hdl_path, "uvm_hdl_read returned FALSE");

    if (rd_val != exp_val) // TODO check !==
    {
      if (oper == DEPOSIT || oper == FORCE)
      {
        ostringstream str;
        str << "(line " << lineno << "): " << op_name[oper] << " of 0x" << hex << wr_val << " - read back got 0x"
            << rd_val << " instead of 0x" << hex << exp_val;
        uvm_report_error(hdl_path, str.str());
      }
      else
      {
        ostringstream str;
        str << "(line " << lineno << "): " << op_name[oper] << " - read back got 0x"
            << hex << rd_val << " instead of 0x" << hex << exp_val;
        uvm_report_error(hdl_path, str.str());
      }
    }
  };

  // Registers (of type sc_reg):
  // Deposit - overwrites value, DUT may change procedurally anytime
  // Force - forces value until released (note: not supported without DPI)
  // Release - forced value remains until DUT produrally reassigns (note: not supported without DPI)
  //
  // Channels (of type sc_signal):
  // Deposit - overwrites value, is retained until one or more driver(s) change value
  // Force - forces value until released (note: not supported without DPI)
  // Release - if continuously driven, immediately gets driven value accordingly.
  //           if not driven, retains value until next direct assignment
  //           (note: Release not supported without DPI)

  void run_phase( uvm_phase& phase ) override
  {
    phase.raise_objection(this);

    wait(50, SC_MS); // get between updates to q

    op(READ,   "dut.q",          0x00, 0x0F, __LINE__);
    op(READ,   "dut.w",          0x00, 0x0F, __LINE__);
    op(READ,   "dut.q[1]",       0x00, 0x01, __LINE__);

    // check support for $root prefix
    op(READ,   "$root.dut.q",    0x00, 0x0F, __LINE__);
    op(READ,   "$root.dut.w",    0x00, 0x0F, __LINE__);
    op(READ,   "$root.dut.q[1]", 0x00, 0x01, __LINE__);

    op(DEPOSIT, "dut.q",         0x3C, 0x3C, __LINE__);
    op(DEPOSIT, "dut.q[4]",      0x00, 0x00, __LINE__);
    op(DEPOSIT, "dut.q[6]",      0x01, 0x01, __LINE__);

    op(DEPOSIT, "dut.q[6:4]",    0x02, 0x02, __LINE__);
    op(READ,    "dut.q",         0x00, 0x2C, __LINE__);

    op(DEPOSIT, "dut.q[7:4]",    0x06, 0x06, __LINE__);

    wait(0, SC_MS);

    op(READ,    "dut.w",         0x00, 0x6C, __LINE__); // w is now q
    op(DEPOSIT, "dut.w",         0x3C, 0x3C, __LINE__); // w retains until q drives new value

    wait(0, SC_MS);

    op(READ,    "dut.w",         0x00, 0x3C, __LINE__); //
    op(DEPOSIT, "dut.q",         0xA5, 0xA5, __LINE__); // deposit on 'dut.q'

    wait(100, SC_MS); // d propagates to q,w

    op(READ,    "dut.q",         0x00, 0xF0, __LINE__); // q and w are now d again
    op(READ,    "dut.w",         0x00, 0xF0, __LINE__); //

#ifdef DPI_ENABLED

    // This part of the test can only be executed with DPI enabled, as it requires
    // forcing and releasing of signals.

    op(FORCE,   "dut.q",         0x3C, 0x3C, __LINE__); // force q and w
    op(FORCE,   "dut.w",         0xA5, 0xA5, __LINE__); //

    wait(200, SC_MS); // q = d should not "take"

    op(READ,    "dut.q",         0x00, 0x3C, __LINE__); // q and w still forced, not d's value (F0)
    op(READ,    "dut.w",         0x00, 0xA5, __LINE__);

    op(RELEASE, "dut.q",         0x00, 0x3C, __LINE__); // q stays until reassigned, should be C3?
    op(RELEASE, "dut.w",         0x00, 0x3C, __LINE__); // w is re-evaluated, now q

    op(READ,    "dut.q",         0x00, 0x3C, __LINE__); // read q just for chuckles

    wait(100, SC_MS);// d propagates to q,w again

    op(READ,    "dut.q",         0x00, 0xF0, __LINE__); // q and w are now d again
    op(READ,    "dut.w",         0x00, 0xF0, __LINE__); //

    op(FORCE,   "dut.d",         0xA5, 0xA5, __LINE__); // force d

    wait(100, SC_MS);// d propagates to q,w again

    op(READ,    "dut.q",         0x00, 0xA5, __LINE__); // q and w are now d
    op(READ,    "dut.w",         0x00, 0xA5, __LINE__); //

    op(RELEASE, "dut.d",         0x00, 0xA5, __LINE__); // d released, stays the same

    wait(100, SC_MS); // d propagates to q,w again

    op(READ,    "dut.q",         0x00, 0xA5, __LINE__); // q and w still d
    op(READ,    "dut.w",         0x00, 0xA5, __LINE__);

    op(FORCE,   "dut.d",         0xF0, 0xF0, __LINE__); // force d back to F0

    wait(100, SC_MS); // d propagates to q,w again

    op(READ,    "dut.q",         0x00, 0xF0, __LINE__); // q and w back to d
    op(READ,    "dut.w",         0x00, 0xF0, __LINE__);

    op(FORCE,   "dut.d",         0x0F, 0x0F, __LINE__, sc_time(100, SC_MS)); // d forced for 100, then released

    op(READ,    "dut.q",         0x00, 0x0F, __LINE__); // q and w should be d
    op(READ,    "dut.w",         0x00, 0x0F, __LINE__);

    op(FORCE,   "dut.w",         0xAA, 0x0F, __LINE__, sc_time(100, SC_MS)); // w is driven to AA for 100, then released,

    // which immed re-evaluates to its q driver, which is 0x0F

#endif //DPI_ENABLED

    // TODO: test undriven wire

    phase.drop_objection(this);
  }
};

int sc_main(int, char*[])
{
  test test1("test1"); // instantiate test

  dut_t dut("dut");

  uvm_root::get()->set_report_verbosity_level(uvm::UVM_FULL);

  run_test();

  // TODO replace by coreservice_t
  // uvm_coreservice_t cs_;
  // uvm::uvm_report_server* svr;
  // cs_ = uvm_coreservice_t::get();
  // svr = cs_.get_report_server();
  // svr->report_summarize();

  uvm::uvm_report_server* svr = uvm::uvm_report_server::get_server();

  if (svr->get_severity_count(uvm::UVM_FATAL) +
      svr->get_severity_count(uvm::UVM_ERROR) == 0)
    std::cout << "** UVM TEST PASSED **" << std::endl;
  else
    std::cout <<"!! UVM TEST FAILED !!" << std::endl;

  return 0;
}

