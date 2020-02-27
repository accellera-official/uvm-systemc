//----------------------------------------------------------------------
//   Copyright 2010-2011 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Synopsys, Inc.
//   Copyright 2013-2014 NXP B.V.
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


#ifndef REG_AGENT_H_
#define REG_AGENT_H_

#include <systemc>
#include <uvm>

class reg_rw : public uvm::uvm_sequence_item
{
 public:

  bool read; // TODO randomize these 4
  sc_dt::sc_bv<32> addr;
  sc_dt::sc_lv<32> data;
  sc_dt::sc_lv<4>  byte_en;

  reg_rw ( std::string name = "reg_rw" ) : uvm::uvm_sequence_item(name)
  {
    std::cout << "constructor called" << std::endl;
  }

  UVM_OBJECT_UTILS(reg_rw);

  /* TODO implement do_print, do_copy, etc.
   `uvm_object_utils_begin(reg_rw)
     `uvm_field_int(read, UVM_ALL_ON | UVM_NOPACK);
     `uvm_field_int(addr, UVM_ALL_ON | UVM_NOPACK);
     `uvm_field_int(data, UVM_ALL_ON | UVM_NOPACK);
     `uvm_field_int(byte_en, UVM_ALL_ON | UVM_NOPACK);
   `uvm_object_utils_end
   */

  std::string convert2string() const
  {
    std::ostringstream str;
    str << "reg_rw: "
        << ((read) ? "READ" : "WRITE" )
        << " addr= 0x" << std::hex << addr.to_int()
        << " data= 0b" << data.to_string()
        << " byte_en = " << byte_en;
    return str.str();
  }

}; // class reg_rw


class reg_sequencer : public uvm::uvm_sequencer<reg_rw>
{
 public:
  UVM_COMPONENT_UTILS(reg_sequencer);

  reg_sequencer( uvm::uvm_component_name name ) : uvm::uvm_sequencer<reg_rw>(name)
  {}

}; // class reg_sequencer



class reg_monitor : public uvm::uvm_monitor
{
 public:
  UVM_COMPONENT_UTILS(reg_monitor);

  uvm::uvm_analysis_port<reg_rw> ap;

  reg_monitor( uvm::uvm_component_name name ) : uvm::uvm_monitor(name), ap("ap")
  {}

}; // class reg_monitor


template <typename DO = int>
class reg_driver: public uvm::uvm_component
{
 public:

  UVM_COMPONENT_PARAM_UTILS(reg_driver<DO>);

  uvm::uvm_seq_item_pull_port<reg_rw> seqr_port;

  reg_driver( uvm::uvm_component_name name )
    : uvm::uvm_component(name),
      seqr_port("seqr_port"),
      m_parent(get_parent())
  {}

  void run_phase(uvm::uvm_phase& phase)
  {
    reg_monitor* mon;
    mon = dynamic_cast<reg_monitor*>(m_parent->get_child("mon"));

    while (true) // forever
    {
      reg_rw rw_req, rw_rsp, tmp;

      seqr_port.peek(rw_req);     // get_next_item
      DO::rw(rw_req);             // rw to dut
      mon->ap.write(rw_req);      // also pass value to the monitor
      rw_rsp.set_id_info(rw_req); // pass id to response
      rw_rsp = rw_req;            // pass modified request to response
      seqr_port.get(rw_req);         // item_done
      seqr_port.put(rw_rsp);      // put response to sequencer
    }
  }

  // local data members
 private:
  uvm::uvm_component* m_parent;

}; // class reg_driver


template <typename DO = int>
class reg_agent : public uvm::uvm_agent
{
 public:
  reg_sequencer*  sqr;
  reg_driver<DO>* drv;
  reg_monitor*    mon;

  UVM_COMPONENT_PARAM_UTILS(reg_agent<DO>);

  reg_agent( uvm::uvm_component_name name )
  : uvm::uvm_agent(name),
    sqr(NULL), drv(NULL), mon(NULL)
  {}

  virtual void build_phase(uvm::uvm_phase& phase)
  {
    sqr = reg_sequencer::type_id::create("sqr", this);
    drv = reg_driver<DO>::type_id::create("drv", this);
    mon = reg_monitor::type_id::create("mon", this);
  }

  virtual void connect_phase(uvm::uvm_phase& phase)
  {
    drv->seqr_port.connect(sqr->seq_item_export);
  }

}; // class reg_agent


class reg2rw_adapter : public uvm::uvm_reg_adapter
{
 public:

  UVM_OBJECT_UTILS(reg2rw_adapter);

  reg2rw_adapter( std::string name = "reg2rw_adapter" ) : uvm::uvm_reg_adapter(name)
  {
    supports_byte_enable = true;
    provides_responses = true;
  }

  virtual uvm::uvm_sequence_item* reg2bus( const uvm::uvm_reg_bus_op& rw )
  {
    reg_rw* bus = reg_rw::type_id::create("rw");
    bus->read    = (rw.kind == uvm::UVM_READ);
    bus->addr    = rw.addr;
    bus->data    = rw.data;
    bus->byte_en = rw.byte_en;
    return bus;
  }

  virtual void bus2reg( const uvm::uvm_sequence_item* bus_item,
                        uvm::uvm_reg_bus_op& rw )
  {
    const reg_rw* bus;
    bus = dynamic_cast<const reg_rw*>(bus_item);

    if (bus == NULL)
    {
      UVM_FATAL("NOT_REG_TYPE", "Provided bus_item is not of the correct type.");
      return;
    }

    rw.kind    = bus->read ? uvm::UVM_READ : uvm::UVM_WRITE;
    rw.addr    = bus->addr;
    rw.data    = bus->data;
    rw.byte_en = bus->byte_en;
    rw.status  = uvm::UVM_IS_OK;
  }

}; // class reg2rw_adapter

#endif //REG_AGENT_H_
