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

#include "ubus_master_driver.h"
#include "ubus_if.h"

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------

ubus_master_driver::ubus_master_driver(uvm::uvm_component_name name)
: uvm::uvm_driver<ubus_transfer>(name)
{}

//------------------------------------------------------------------------------
// member function: build_phase
//------------------------------------------------------------------------------

void ubus_master_driver::build_phase(uvm::uvm_phase& phase)
{
  uvm::uvm_driver<ubus_transfer>::build_phase(phase);

  if(!uvm::uvm_config_db<ubus_if*>::get(this, "", "vif", vif))
    UVM_FATAL("NOVIF", "virtual interface must be set for: "
        + get_full_name() + ".vif");

  // get master id
  if(!uvm::uvm_config_db<int>::get(this, "", "master_id", master_id))
    UVM_FATAL("NO_MASTER_ID","Master ID not found in configuration database");
}

//------------------------------------------------------------------------------
// member function: run_phase
//------------------------------------------------------------------------------

void ubus_master_driver::run_phase(uvm::uvm_phase& phase)
{
  SC_FORK
    sc_core::sc_spawn(sc_bind(&ubus_master_driver::get_and_drive, this)),
    sc_core::sc_spawn(sc_bind(&ubus_master_driver::reset_signals, this))
  SC_JOIN
}

//------------------------------------------------------------------------------
// member function: get_and_drive
//------------------------------------------------------------------------------

void ubus_master_driver::get_and_drive()
{
  sc_core::wait(vif->sig_reset.negedge_event());

  while(true) //forever
  {
    sc_core::wait(vif->sig_clock.posedge_event());

    ubus_transfer req;
    ubus_transfer rsp;

    this->seq_item_port.get_next_item(req);

    UVM_INFO(get_type_name(), "Transfer req received : " +
      req.sprint(), uvm::UVM_NONE);

    // TODO check
    //rsp = dynamic_cast<ubus_transfer*>(req.clone());
    rsp = req;

    rsp.set_id_info(req);
    drive_transfer(rsp);

    this->seq_item_port.item_done();
    this->seq_item_port.put_response(rsp);
  }
}

//------------------------------------------------------------------------------
// member function: reset_signals
//------------------------------------------------------------------------------

void ubus_master_driver::reset_signals()
{
  while(true) // forever
  {
    sc_core::wait(vif->sig_reset.posedge_event());

    vif->sig_request[master_id] = sc_dt::SC_LOGIC_0;
    vif->rw              = sc_dt::SC_LOGIC_0;
    vif->sig_addr        = "zzzzzzzzzzzzzzzz";
    vif->sig_data_out    = "zzzzzzzz";
    vif->sig_size        = "zz";
    vif->sig_read        = sc_dt::SC_LOGIC_Z;
    vif->sig_write       = sc_dt::SC_LOGIC_Z;
    vif->sig_bip         = sc_dt::SC_LOGIC_Z;
    sc_core::wait(sc_core::SC_ZERO_TIME);
  }
}

//------------------------------------------------------------------------------
// member function: drive_transfer
//------------------------------------------------------------------------------

void ubus_master_driver::drive_transfer(ubus_transfer trans)
{
  std::cout << sc_core::sc_time_stamp() << ": drive_transfer" << std::endl;
  if (trans.transmit_delay > 0)
    for (unsigned int i; i < trans.transmit_delay; i++)
      sc_core::wait(vif->sig_clock.posedge_event());

  arbitrate_for_bus();
  drive_address_phase(trans);
  drive_data_phase(trans);

  //TODO temporary
  UVM_INFO(get_type_name(), "Transfer send : " +
    trans.sprint(), uvm::UVM_NONE);
}

//------------------------------------------------------------------------------
// member function: arbitrate_for_bus
//------------------------------------------------------------------------------

void ubus_master_driver::arbitrate_for_bus()
{
  std::cout << sc_core::sc_time_stamp() << ": arbitrate_for_bus" << std::endl;
  vif->sig_request[master_id] = sc_dt::SC_LOGIC_1;

  // @(posedge vif.sig_clock iff vif.sig_grant[master_id] === 1);
  do
  {
    sc_core::wait(vif->sig_clock.posedge_event());
  }
  while (!(vif->sig_grant[master_id] == sc_dt::SC_LOGIC_1));

  vif->sig_request[master_id] = sc_dt::SC_LOGIC_0;
}

//------------------------------------------------------------------------------
// member function: drive_address_phase
//------------------------------------------------------------------------------

void ubus_master_driver::drive_address_phase(ubus_transfer trans)
{
  std::cout << sc_core::sc_time_stamp() << ": drive_address_phase" << std::endl;
  vif->sig_addr = trans.addr;
  drive_size(trans.size);
  drive_read_write(trans.read_write);

  sc_core::wait(vif->sig_clock.posedge_event());

  vif->sig_addr =  "zzzzzzzzzzzzzzzz";
  vif->sig_size = "zz";
  vif->sig_read  = sc_dt::SC_LOGIC_Z;
  vif->sig_write = sc_dt::SC_LOGIC_Z;
}

//------------------------------------------------------------------------------
// member function: drive_data_phase
//------------------------------------------------------------------------------

void ubus_master_driver::drive_data_phase(ubus_transfer trans)
{
  std::cout << sc_core::sc_time_stamp() << ": drive_data_phase" << std::endl;
  bool err;

  for(unsigned int i = 0; i <= trans.size - 1; i++)
  {
    if (i == (trans.size - 1))
      vif->sig_bip = sc_dt::SC_LOGIC_0;
    else
      vif->sig_bip = sc_dt::SC_LOGIC_1;

    switch (trans.read_write)
    {
      case READ    : read_byte(trans.data[i], err); break;
      case WRITE   : write_byte(trans.data[i], err); break;
      default: break;
    }
  } //for loop

  vif->sig_data_out = "zzzzzzzz";
  vif->sig_bip = sc_dt::SC_LOGIC_1;
}

//------------------------------------------------------------------------------
// member function: read_byte
//------------------------------------------------------------------------------

void ubus_master_driver::read_byte(sc_dt::sc_uint<8>& data, bool& error)
{
  std::cout << sc_core::sc_time_stamp() << ": read_byte" << std::endl;
  vif->rw = sc_dt::SC_LOGIC_0;

  // @(posedge vif.sig_clock iff vif.sig_wait === 0);
  do
  {
    sc_core::wait(vif->sig_clock.posedge_event());
  }
  while (!(vif->sig_wait == sc_dt::SC_LOGIC_0));

  std::cout << sc_core::sc_time_stamp() << "[" << this->get_name() << "]: read " << vif->sig_data.read() << " from vif->sig_data" << std::endl;

  data = vif->sig_data.read().to_uint();
}

//------------------------------------------------------------------------------
// member function: write_byte
//------------------------------------------------------------------------------

void ubus_master_driver::write_byte(sc_dt::sc_uint<8> data, bool& error)
{
  std::cout << sc_core::sc_time_stamp() << ": write_byte" << std::endl;

  std::cout << sc_core::sc_time_stamp() << "[" << this->get_name() << "]: write " << data << " to vif->sig_data_out " << std::endl;
  vif->rw = sc_dt::SC_LOGIC_1;
  vif->sig_data_out = data.to_uint();

  // @(posedge vif.sig_clock iff vif.sig_wait === 0);
  do
  {
    sc_core::wait(vif->sig_clock.posedge_event());
  }
  while (!(vif->sig_wait == sc_dt::SC_LOGIC_0));

  vif->rw = sc_dt::SC_LOGIC_0;
}

//------------------------------------------------------------------------------
// member function: drive_size
//------------------------------------------------------------------------------

void ubus_master_driver::drive_size(unsigned int size)
{
  std::cout << sc_core::sc_time_stamp() << ": drive_size : " << size << std::endl;

  switch(size)
  {
    case 1: vif->sig_size = 0b00; break;
    case 2: vif->sig_size = 0b01; break;
    case 4: vif->sig_size = 0b10; break;
    case 8: vif->sig_size = 0b11; break;
    default: /* do nothing */ break;
  }
}

//------------------------------------------------------------------------------
// member function: drive_read_write
//------------------------------------------------------------------------------

void ubus_master_driver::drive_read_write(ubus_read_write_enum rw)
{
  std::cout << sc_core::sc_time_stamp() << ": drive_read_write :" << rw << std::endl;
  switch(rw)
  {
    case NOP   : vif->sig_read = sc_dt::SC_LOGIC_0; vif->sig_write = sc_dt::SC_LOGIC_0; break;
    case READ  : vif->sig_read = sc_dt::SC_LOGIC_1; vif->sig_write = sc_dt::SC_LOGIC_0; break;
    case WRITE : vif->sig_read = sc_dt::SC_LOGIC_0; vif->sig_write = sc_dt::SC_LOGIC_1; break;
    default: /* do nothing */ break;
  }
}
