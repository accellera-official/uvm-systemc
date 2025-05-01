//-----------------------------------------------------------------------------
//   Copyright 2013-2015 NXP B.V.
//   Copyright 2004-2009 Synopsys, Inc.
//   Copyright 2010-2011 Mentor Graphics Corporation
//   Copyright 2010-2011 Cadence Design Systems, Inc.
//   Copyright 2018 Intel Corp.
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
//-----------------------------------------------------------------------------

#ifndef UVM_REG_PREDICTOR_H_
#define UVM_REG_PREDICTOR_H_

#include <map>
#include <systemc>
#include <string>
#include <iostream>

#include "uvmsc/base/uvm_component_name.h"
#include "uvmsc/base/uvm_component.h"
#include "uvmsc/macros/uvm_component_defines.h"
#include "uvmsc/factory/uvm_component_registry.h"
#include "uvmsc/reg/uvm_reg_item.h"
#include "uvmsc/reg/uvm_reg_adapter.h"
#include "uvmsc/reg/uvm_reg_indirect_data.h"
#include "uvmsc/reg/uvm_reg_map.h"
#include "uvmsc/reg/uvm_reg_bus_op.h"
#include "uvmsc/reg/uvm_reg_block.h"
#include "uvmsc/tlm1/uvm_analysis_port.h"
#include "uvmsc/tlm1/uvm_analysis_imp.h"
#include "uvmsc/phasing/uvm_phase.h"

namespace uvm {

//------------------------------------------------------------------------------
// TITLE: Explicit Register Predictor
//
// The uvm_reg_predictor class defines a predictor component,
// which is used to update the register model's mirror values
// based on transactions explicitly observed on a physical bus. 
//------------------------------------------------------------------------------

class uvm_predict_s
{
 public:
  uvm_predict_s()
  {
    reg_item = nullptr;
    addr.clear();
  }

  // data members
  std::map<uvm_reg_addr_t, bool> addr;
  uvm_reg_item* reg_item;

}; // class uvm_predict_s

//------------------------------------------------------------------------------
// CLASS: uvm_reg_predictor
//
//! Updates the register model mirror based on observed bus transactions
//!
//! This class converts observed bus transactions of type #BUSTYPE to generic
//! registers transactions, determines the register being accessed based on the
//! bus address, then updates the register's mirror value with the observed bus
//! data, subject to the register's access mode. See <uvm_reg::predict> for details.
//!
//! Memories can be large, so their accesses are not predicted.
//------------------------------------------------------------------------------

template <typename BUSTYPE = int>
class uvm_reg_predictor : public uvm_component,
                          public tlm::tlm_analysis_if<BUSTYPE>
{
 public:

  // ports
  uvm_analysis_imp< BUSTYPE, uvm_reg_predictor<BUSTYPE> > bus_in;

  uvm_analysis_port<uvm_reg_item> reg_ap;

  // factory registration

  UVM_COMPONENT_PARAM_UTILS(uvm_reg_predictor<BUSTYPE>)

  // Constructor

  explicit uvm_reg_predictor( uvm_component_name name );

  // Methods

  virtual void pre_predict( uvm_reg_item* rw );

  void write( const BUSTYPE& tr ) override;

  void check_phase( uvm_phase& phase ) override;


  // data members

  uvm_reg_map* map;

  uvm_reg_adapter* adapter;

 private:
  typedef std::map<uvm_reg*, uvm_predict_s*> m_pending_t;
  typedef m_pending_t::iterator m_pending_itt;
  m_pending_t m_pending;

  static std::string type_name;

}; // class


//------------------------------------------------------------------------------
// CLass implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// static data member initialization
//------------------------------------------------------------------------------

template <typename BUSTYPE>
std::string uvm_reg_predictor<BUSTYPE>::type_name = "";

//------------------------------------------------------------------------------
// Constructor
//
//! Create a new instance of this type, giving it the optional \p name.
//!
//! NOTE: The argument parent has been removed, as this is not available in
//!       UVM-SC.
//------------------------------------------------------------------------------

template <typename BUSTYPE>
uvm_reg_predictor<BUSTYPE>::uvm_reg_predictor( uvm_component_name name )
  : uvm_component(name), bus_in("bus_in", this), reg_ap("reg_ap")
{
  map = nullptr;
  adapter = nullptr;
  m_pending.clear();
}

//------------------------------------------------------------------------------
// member function: pre_predict
//
//! Override this method to change the value or re-direct the
//! target register
//------------------------------------------------------------------------------

template <typename BUSTYPE>
void uvm_reg_predictor<BUSTYPE>::pre_predict( uvm_reg_item* rw )
{}

//------------------------------------------------------------------------------
// member function: write
//
//! not a user-level method. Do not call directly. See documentation
//! for the #bus_in member.
//------------------------------------------------------------------------------

template <typename BUSTYPE>
void uvm_reg_predictor<BUSTYPE>::write( const BUSTYPE& tr )
{
  uvm_reg* rg;
  uvm_reg_bus_op rw;

  if (adapter == nullptr)
    UVM_FATAL("REG/WRITE/NULL", "write: adapter handle is nullptr");

  // In case they forget to set byte_en
  rw.byte_en = -1;
  adapter->bus2reg(&tr, rw);

  rg = map->get_reg_by_offset(rw.addr, (rw.kind == UVM_READ));

  // UVM_SV TODO: Add memory look-up and call uvm_mem::m_sample()

  if (rg != nullptr)
  {
    bool found = false;
    uvm_reg_item* reg_item;
    uvm_reg_map* local_map;
    uvm_reg_map_info* map_info;
    uvm_predict_s* predict_info;
    uvm_reg_indirect_data* ireg;
    uvm_reg* ir;

    if( m_pending.find(rg) == m_pending.end() ) // not exists
    {
      uvm_reg_item* item = new uvm_reg_item();
      predict_info = new uvm_predict_s();
      item->element_kind = UVM_REG;
      item->element      = rg;
      item->path         = UVM_PREDICT;
      item->map          = map;
      item->access_kind  = rw.kind;
      predict_info->reg_item = item;
      m_pending[rg] = predict_info;
    }

    predict_info = m_pending[rg];
    reg_item = predict_info->reg_item;

    // at least one place reserved to store the value
    if (reg_item->value.size() == 0)
      reg_item->value.resize(1);

    if( predict_info->addr.find(rw.addr) != predict_info->addr.end() ) // exists
    {
      UVM_ERROR("REG_PREDICT_COLLISION",
        "Collision detected for register '" +
          rg->get_full_name() + "'");
      // TODO: what to do with subsequent collisions?
      m_pending.erase(rg); // TODO delete rg from map, but also delete the object
    }

    local_map = rg->get_local_map(map, "predictor::write()" );
    map_info = local_map->get_reg_map_info(rg);
    ireg = dynamic_cast<uvm_reg_indirect_data*>(rg);
    ir = (ireg != nullptr) ? ireg->get_indirect_reg() : rg;

    for( unsigned int i = 0; i < map_info->addr.size(); i++ )
    {
      if (rw.addr == map_info->addr[i])
      {
        found = true;
        reg_item->value[0] |= rw.data << (i * map->get_n_bytes()*8);

        predict_info->addr[rw.addr] = true;
        if (predict_info->addr.size() == map_info->addr.size())
        {
          // We've captured the entire abstract register transaction.
          uvm_predict_e predict_kind =
              (reg_item->access_kind == UVM_WRITE) ? UVM_PREDICT_WRITE : UVM_PREDICT_READ;

          if (reg_item->access_kind == UVM_READ &&
              local_map->get_check_on_read() &&
              reg_item->status != UVM_NOT_OK)
          {
            rg->do_check(ir->get_mirrored_value(), reg_item->value[0], local_map);
          }

          pre_predict(reg_item);

          ir->m_sample( reg_item->value[0], rw.byte_en,
              reg_item->access_kind == UVM_READ, local_map);

          uvm_reg_block* blk = rg->get_parent();
          blk->m_sample(map_info->offset,
              reg_item->access_kind == UVM_READ,
              local_map);

          rg->do_predict(reg_item, predict_kind, rw.byte_en);

          if(reg_item->access_kind == UVM_WRITE)
          {
            std::ostringstream str;
            str << "Observed WRITE transaction to register "
                << ir->get_full_name()
                << ": value = 0x"
                << std::hex << reg_item->value[0].to_uint64()
                << " : updated value = 0x"
                << std::hex << ir->get().to_uint64();
            UVM_INFO("REG_PREDICT", str.str(), UVM_HIGH);
          }
          else
          {
            std::ostringstream str;
            str << "Observed READ transaction to register "
                << ir->get_full_name()
                << ": value= 0x"
                << std::hex << reg_item->value[0].to_uint64();
            UVM_INFO("REG_PREDICT", str.str(), UVM_HIGH);

            reg_ap.write(*reg_item); // TODO check dereferencing
          }
          m_pending.erase(rg); // TODO deleted rg from map, but also delete object?
        }
        break;
      }
    }

    if (!found)
      UVM_ERROR("REG_PREDICT_INTERNAL",
          "Unexpected failed address lookup for register '" +
          rg->get_full_name() + "'");
  }
  else
  {
    std::ostringstream str;
    str << "Observed transaction does not target a register: "
        << tr; // TODO check if all template arguments support operator<<
    UVM_INFO("REG_PREDICT_NOT_FOR_ME", str.str(), UVM_FULL);
  }
}

//------------------------------------------------------------------------------
// member function: check_phase (virtual)
//
//! Checks that no pending register transactions are still enqueued.
//------------------------------------------------------------------------------

template <typename BUSTYPE>
void uvm_reg_predictor<BUSTYPE>::check_phase(uvm_phase& phase)
{
  uvm_component::check_phase(phase);

  if (m_pending.size() > 0)
  {
    std::vector<std::string> q;
    std::ostringstream str;
    str << "There are " << m_pending.size()
        << " incomplete register transactions still pending completion:\n";
    q.push_back(str.str());

    for( m_pending_itt it = m_pending.begin();
         it != m_pending.end();
         it++)
    {
      uvm_reg* rg = (*it).first;
      q.push_back("\n" + rg->get_full_name());
    }

    UVM_ERROR("PENDING REG ITEMS", UVM_STRING_QUEUE_STREAMING_PACK(q));
  }
}

} // namespace uvm

#endif // UVM_REG_PREDICTOR_H_
