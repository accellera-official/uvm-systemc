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

#include "ubus_slave_driver.h"

//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------


ubus_slave_driver::ubus_slave_driver(uvm::uvm_component_name name)
: uvm::uvm_driver<ubus_transfer>(name)
{}

//----------------------------------------------------------------------
// member function: build_phase
//----------------------------------------------------------------------

void ubus_slave_driver::build_phase(uvm::uvm_phase& phase)
{
  if(!uvm::uvm_config_db<ubus_if*>::get(this, "", "vif", vif))
    UVM_FATAL("NOVIF", "Virtual interface must be set for: " + get_full_name() + ".vif");
}

//----------------------------------------------------------------------
// member function: run_phase
//----------------------------------------------------------------------

void ubus_slave_driver::run_phase(uvm::uvm_phase& phase)
{
  SC_FORK
    sc_core::sc_spawn(sc_bind(&ubus_slave_driver::get_and_drive, this)),
    sc_core::sc_spawn(sc_bind(&ubus_slave_driver::reset_signals, this))
  SC_JOIN
}

//----------------------------------------------------------------------
// member function: get_and_drive
//----------------------------------------------------------------------

void ubus_slave_driver::get_and_drive()
{
  std::cout << sc_core::sc_time_stamp() << ": ubus_slave_driver::get_and_drive" << std::endl;
  sc_core::wait(vif->sig_reset.negedge_event());

  while(true) // forever
  {
    ubus_transfer req;
    ubus_transfer rsp;

    sc_core::wait(vif->sig_clock.posedge_event());

    this->seq_item_port.get_next_item(req);
    respond_to_transfer(req);
    this->seq_item_port.item_done();
  }
}

//----------------------------------------------------------------------
// member function: reset_signals
//----------------------------------------------------------------------

void ubus_slave_driver::reset_signals()
{
  std::cout << sc_core::sc_time_stamp() << ": ubus_slave_driver::reset_signals" << std::endl;

  while(true) // forever
  {
    sc_core::wait(vif->sig_reset.posedge_event());
    vif->sig_error = sc_dt::SC_LOGIC_Z;
    vif->sig_wait  = sc_dt::SC_LOGIC_Z;
    vif->rw        = sc_dt::SC_LOGIC_0;
  }
}

//----------------------------------------------------------------------
// member function: respond_to_transfer
//----------------------------------------------------------------------

void ubus_slave_driver::respond_to_transfer(ubus_transfer resp)
{
  std::cout << sc_core::sc_time_stamp() << ": ubus_slave_driver::respond_to_transfer" << std::endl;

  // TODO temp
  UVM_INFO(get_type_name(), "Transfer resp: " +
    resp.sprint(), uvm::UVM_NONE);

  if (resp.read_write != NOP)
  {
    vif->sig_error = sc_dt::SC_LOGIC_0;
    for (unsigned int i = 0; i < resp.size; i++)
    {
      switch (resp.read_write)
      {
      case READ :
        vif->rw = sc_dt::SC_LOGIC_1;
        std::cout << sc_core::sc_time_stamp() << " [" << this->get_full_name() << "]: write " << resp.data[i] << " to vif->sig_data_out " << std::endl;
        vif->sig_data_out = resp.data[i];
        break;
      case WRITE : break;
      default: break;
      }

      if (resp.wait_state[i] > 0)
      {
        vif->sig_wait = sc_dt::SC_LOGIC_1;
        for (unsigned int j = 0; j < resp.wait_state[j]; j++)
          sc_core::wait(vif->sig_clock.posedge_event());
      }

      vif->sig_wait = sc_dt::SC_LOGIC_0;
      sc_core::wait(vif->sig_clock.posedge_event());

      std::cout << sc_core::sc_time_stamp() << "[" << this->get_name() << "]: reading " << vif->sig_data.read() << "from vif->sig_data " << std::endl;
      resp.data[i] = vif->sig_data.read().to_uint();

    }
    vif->rw        = sc_dt::SC_LOGIC_0;
    vif->sig_wait  = sc_dt::SC_LOGIC_Z;
    vif->sig_error = sc_dt::SC_LOGIC_Z;
  }
}
