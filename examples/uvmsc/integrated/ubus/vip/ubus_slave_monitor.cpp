//----------------------------------------------------------------------
//   Copyright 2016 NXP B.V.
//   Copyright 2007-2010 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010 Synopsys, Inc.
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

#include "ubus_slave_monitor.h"

//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------

ubus_slave_monitor::ubus_slave_monitor(uvm::uvm_component_name name)
: uvm::uvm_monitor(name),
  item_collected_port("item_collected_port"),
  addr_ph_imp("addr_ph_imp", this)
{
  /* TODO coverage
    cov_trans = new();
    cov_trans.set_inst_name({get_full_name(), ".cov_trans"});
    cov_trans_beat = new();
    cov_trans_beat.set_inst_name({get_full_name(), ".cov_trans_beat"});
    trans_collected = new();
   */
  min_addr = 0x0000;
  max_addr = 0xFFFF;

  checks_enable = true;
  coverage_enable = true;
}

//----------------------------------------------------------------------
// member function: build_phase
//----------------------------------------------------------------------

void ubus_slave_monitor::build_phase(uvm::uvm_phase& phase)
{
  if(!uvm::uvm_config_db<ubus_if*>::get(this, "", "vif", vif))
    UVM_FATAL("NOVIF","Virtual interface must be set for: " + get_full_name() + ".vif");
}


//----------------------------------------------------------------------
// member function: set_addr_range
// set the monitor's address range
//----------------------------------------------------------------------

void ubus_slave_monitor::set_addr_range( unsigned int min_addr_, unsigned int max_addr_)
{
  min_addr = min_addr_;
  max_addr = max_addr_;
}


//----------------------------------------------------------------------
// member function: report_phase
//----------------------------------------------------------------------

void ubus_slave_monitor::report_phase(uvm::uvm_phase& phase)
{
  /* TODO report coverage
    std::ostringstream msg;
    msg << "Covergroup 'cov_trans' coverage: "
        <<  cov_trans.get_inst_coverage();
    UVM_INFO(get_full_name(), msg.str(), uvm::UVM_LOW)
   */
}

//----------------------------------------------------------------------
// member function: get_min_addr
//----------------------------------------------------------------------

// get the monitor's min addr
unsigned int ubus_slave_monitor::get_min_addr()
{
  return min_addr;
}

//----------------------------------------------------------------------
// member function: get_max_addr
// get the monitor's max addr
//----------------------------------------------------------------------

unsigned int ubus_slave_monitor::get_max_addr()
{
  return max_addr;
}

//----------------------------------------------------------------------
// member function: run_phase
//----------------------------------------------------------------------

void ubus_slave_monitor::run_phase(uvm::uvm_phase& phase)
{
  SC_FORK
    sc_core::sc_spawn(sc_bind(&ubus_slave_monitor::collect_transactions, this))
  SC_JOIN
}

//----------------------------------------------------------------------
// member function: collect_transactions
//----------------------------------------------------------------------

void ubus_slave_monitor::collect_transactions()
{
  bool range_check;

  while (true) // forever
  {
    trans_collected.slave = get_parent()->get_name();

    std::cout << "trans_collected.slave = " << trans_collected.slave << std::endl;

    collect_address_phase();
    range_check = check_addr_range();

    if (range_check)
    {
      this->begin_tr(trans_collected);

      address_phase_grabbed.notify();

      collect_data_phase();

      UVM_INFO(get_type_name(), "Transfer collected :\n" +
          trans_collected.sprint(), uvm::UVM_FULL);

      if (checks_enable) perform_transfer_checks();
      if (coverage_enable) perform_transfer_coverage();

      item_collected_port.write(trans_collected);
    }
  }
}

//----------------------------------------------------------------------
// member function: check_addr_range
//----------------------------------------------------------------------

bool ubus_slave_monitor::check_addr_range()
{
  if ((trans_collected.addr >= min_addr) &&
      (trans_collected.addr <= max_addr))
    return true;

  return false;
}

//----------------------------------------------------------------------
// member function: collect_address_phase
//----------------------------------------------------------------------

void ubus_slave_monitor::collect_address_phase()
{
  // @(posedge vif.sig_clock iff ( (vif.sig_read === 1) || (vif.sig_write === 1) ) );
  do
  {
    sc_core::wait(vif->sig_clock.posedge_event());
  }
  while (!((vif->sig_write == sc_dt::SC_LOGIC_1) || (vif->sig_read == sc_dt::SC_LOGIC_1)));

  trans_collected.addr = vif->sig_addr.read().to_uint();

  int size = vif->sig_size.read().to_uint();

  if(size == 0) trans_collected.size = 1;
  if(size == 1) trans_collected.size = 2;
  if(size == 2) trans_collected.size = 4;
  if(size == 3) trans_collected.size = 8;

  for (unsigned int i = 0; i < trans_collected.size; i++)
    trans_collected.data.push_back(0); // reserve data fields

  sc_dt::sc_logic read = vif->sig_read.read();
  sc_dt::sc_logic write = vif->sig_write.read();

  if(read == sc_dt::SC_LOGIC_0 && write == sc_dt::SC_LOGIC_0) trans_collected.read_write = NOP;
  if(read == sc_dt::SC_LOGIC_1 && write == sc_dt::SC_LOGIC_0) trans_collected.read_write = READ;
  if(read == sc_dt::SC_LOGIC_0 && write == sc_dt::SC_LOGIC_1) trans_collected.read_write = WRITE;
}

//----------------------------------------------------------------------
// member function: collect_data_phase
//----------------------------------------------------------------------

void ubus_slave_monitor::collect_data_phase()
{
  if (trans_collected.read_write != NOP)
  {
    for (unsigned int i = 0; i < trans_collected.size; i++)
    {
      //  @(posedge vif.sig_clock iff vif.sig_wait === 0);
      do
      {
        sc_core::wait(vif->sig_clock.posedge_event());
      }
      while (!(vif->sig_wait == sc_dt::SC_LOGIC_0));

      trans_collected.data[i] = vif->sig_data.read().to_uint();
    }
  }

  this->end_tr(trans_collected);
}

//----------------------------------------------------------------------
// member function: perform_transfer_coverage
//----------------------------------------------------------------------

void ubus_slave_monitor::perform_transfer_checks()
{
  check_transfer_size();
  check_transfer_data_size();
}

//----------------------------------------------------------------------
// member function: check_transfer_size
//----------------------------------------------------------------------

void ubus_slave_monitor::check_transfer_size()
{
  // TODO use assertion 'assert_transfer_size'
  if (trans_collected.size == 1 ||
      trans_collected.size == 2 || trans_collected.size == 4 || 
      trans_collected.size == 8) /*ok */ ;
  else
    UVM_ERROR(get_type_name(), "Invalid transfer size!");
}

//----------------------------------------------------------------------
// member function: check_transfer_data_size
//----------------------------------------------------------------------

void ubus_slave_monitor::check_transfer_data_size()
{
  /* TODO check_transfer_data_size()
  if (trans_collected.size != trans_collected.data.size())
    UVM_ERROR(get_type_name(),
        "Transfer size field / data size mismatch.");
  */
}

//----------------------------------------------------------------------
// member function: perform_transfer_coverage
//----------------------------------------------------------------------

void ubus_slave_monitor::perform_transfer_coverage()
{
  // TODO sample for coverage
  //cov_trans.sample();

  for (int unsigned i = 0; i < trans_collected.size; i++)
  {
    addr = trans_collected.addr + i;
    data = trans_collected.data[i];
    // UVM-SV: Wait state information is not currently monitored.
    // wait_state = trans_collected.wait_state[i];
    //cov_trans_beat.sample();
  }
}

//----------------------------------------------------------------------
// member function: peek
//----------------------------------------------------------------------

// TODO check peek method
//void ubus_slave_monitor::peek(ubus_transfer& trans)
ubus_transfer ubus_slave_monitor::peek()
{
  sc_core::wait(address_phase_grabbed);

  //TODO temp
  UVM_INFO(get_type_name(), "Transfer peek: " +
      trans_collected.sprint(), uvm::UVM_NONE);

  return trans_collected;
}
