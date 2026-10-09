//------------------------------------------------------------------------------
// Copyright 2026 COSEDA Technologies GmbH
// Copyright 2011-2018 Cadence Design Systems, Inc.
// Copyright 2010-2011 Mentor Graphics Corporation
// Copyright 2017 NVIDIA Corporation
// Copyright 2011 Synopsys, Inc.
// All Rights Reserved Worldwide
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may
// not use this file except in compliance with the License.  You may obtain
// a copy of the License at
// 
//        http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations
// under the License.
//------------------------------------------------------------------------------

#ifndef REG_AGENT_H_
#define REG_AGENT_H_

#include <systemc>
#include <uvm>
#include <sstream>

class field_reg_rw : public uvm::uvm_sequence_item
{
 public:
  bool read;
  sc_dt::sc_bv<32> addr;
  sc_dt::sc_lv<32> data;
  sc_dt::sc_bv<4> byte_en;

  UVM_OBJECT_UTILS(field_reg_rw);

  field_reg_rw( const std::string& name = "field_reg_rw" )
  : uvm::uvm_sequence_item(name), read(false)
  {}

  virtual void do_copy( const uvm::uvm_object& rhs )
  {
    const field_reg_rw* rhs_ = dynamic_cast<const field_reg_rw*>(&rhs);
    if (rhs_ == nullptr)
    {
      UVM_FATAL("do_copy", "Provided object is not of the correct type");
      return;
    }

    uvm::uvm_sequence_item::do_copy(rhs);
    read = rhs_->read;
    addr = rhs_->addr;
    data = rhs_->data;
    byte_en = rhs_->byte_en;
  }

  virtual bool do_compare( const uvm::uvm_object& rhs,
                           const uvm::uvm_comparer* comparer ) const
  {
    const field_reg_rw* rhs_ = dynamic_cast<const field_reg_rw*>(&rhs);
    return rhs_ != nullptr &&
           uvm::uvm_sequence_item::do_compare(rhs, comparer) &&
           read == rhs_->read && addr == rhs_->addr &&
           data == rhs_->data && byte_en == rhs_->byte_en;
  }

  virtual void do_print( const uvm::uvm_printer& printer ) const
  {
    uvm::uvm_sequence_item::do_print(printer);
    printer.print_field_int("read", read, 1);
    printer.print_field_int("addr", addr, 32);
    printer.print_field_int("data", data, 32);
    printer.print_field_int("byte_en", byte_en, 4);
  }

  virtual std::string convert2string() const
  {
    std::ostringstream str;
    str << (read ? "READ" : "WRITE")
        << " addr=" << std::hex << addr.to_uint64()
        << " data=" << data.to_uint64()
        << " be=" << byte_en.to_string();
    return str.str();
  }
};

class reg_sequencer : public uvm::uvm_sequencer<field_reg_rw>
{
 public:
  UVM_COMPONENT_UTILS(reg_sequencer);

  reg_sequencer( uvm::uvm_component_name name )
  : uvm::uvm_sequencer<field_reg_rw>(name)
  {}
};

class reg_monitor : public uvm::uvm_monitor
{
 public:
  uvm::uvm_analysis_port<field_reg_rw> ap;

  UVM_COMPONENT_UTILS(reg_monitor);

  reg_monitor( uvm::uvm_component_name name )
  : uvm::uvm_monitor(name), ap("ap")
  {}
};

template <typename DO = int>
class reg_driver : public uvm::uvm_component
{
 public:
  uvm::uvm_seq_item_pull_port<field_reg_rw> seqr_port;

  UVM_COMPONENT_PARAM_UTILS(reg_driver<DO>);

  reg_driver( uvm::uvm_component_name name )
  : uvm::uvm_component(name), seqr_port("seqr_port")
  {}

  virtual void run_phase( uvm::uvm_phase& phase )
  {
    reg_monitor* mon = dynamic_cast<reg_monitor*>(get_parent()->get_child("mon"));
    if (mon == nullptr)
    {
      UVM_FATAL("NO_MONITOR", "Register monitor not found");
      return;
    }

    while (true)
    {
      field_reg_rw req, rsp;
      seqr_port.peek(req);
      DO::rw(req);
      mon->ap.write(req);
      rsp = req;
      rsp.set_id_info(req);
      seqr_port.get(req);
      seqr_port.put(rsp);
    }
  }
};

template <typename DO = int>
class reg_agent : public uvm::uvm_agent
{
 public:
  reg_sequencer* sqr;
  reg_driver<DO>* drv;
  reg_monitor* mon;

  UVM_COMPONENT_PARAM_UTILS(reg_agent<DO>);

  reg_agent( uvm::uvm_component_name name )
  : uvm::uvm_agent(name), sqr(nullptr), drv(nullptr), mon(nullptr)
  {}

  virtual void build_phase( uvm::uvm_phase& phase )
  {
    sqr = reg_sequencer::type_id::create("sqr", this);
    drv = reg_driver<DO>::type_id::create("drv", this);
    mon = reg_monitor::type_id::create("mon", this);
  }

  virtual void connect_phase( uvm::uvm_phase& phase )
  {
    drv->seqr_port.connect(sqr->seq_item_export);
  }
};

class reg2rw_adapter : public uvm::uvm_reg_adapter
{
 public:
  UVM_OBJECT_UTILS(reg2rw_adapter);

  reg2rw_adapter( const std::string& name = "reg2rw_adapter" )
  : uvm::uvm_reg_adapter(name)
  {
    supports_byte_enable = true;
    // SystemC passes copied requests; return the driver's modified response.
    provides_responses = true;
  }

  virtual uvm::uvm_sequence_item* reg2bus( const uvm::uvm_reg_bus_op& rw )
  {
    field_reg_rw* bus = field_reg_rw::type_id::create("rw");
    bus->read = (rw.kind == uvm::UVM_READ);
    bus->addr = rw.addr;
    bus->data = rw.data;
    bus->byte_en = rw.byte_en;
    return bus;
  }

  virtual void bus2reg( const uvm::uvm_sequence_item* bus_item,
                        uvm::uvm_reg_bus_op& rw )
  {
    const field_reg_rw* bus = dynamic_cast<const field_reg_rw*>(bus_item);
    if (bus == nullptr)
    {
      UVM_FATAL("NOT_REG_TYPE", "Provided bus_item is not of the correct type");
      return;
    }

    rw.kind = bus->read ? uvm::UVM_READ : uvm::UVM_WRITE;
    rw.addr = bus->addr;
    rw.data = bus->data;
    rw.byte_en = bus->byte_en;
    rw.status = uvm::UVM_IS_OK;
  }
};

#endif // REG_AGENT_H_
