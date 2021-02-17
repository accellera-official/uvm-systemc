//----------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2010-2011 Mentor Graphics Corporation
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
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

#include "uvmsc/reg/uvm_reg_fifo.h"
#include "uvmsc/reg/uvm_reg_field.h"
#include "uvmsc/reg/uvm_reg_item.h"

//////////////

using namespace sc_dt;

namespace uvm {

//----------------------------------------------------------------------
// Group: Initialization
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Constructor
//
//! Creates an instance of a FIFO register having \p size elements of
//! \p n_bits each.
//----------------------------------------------------------------------

uvm_reg_fifo::uvm_reg_fifo( const std::string& name,
                            unsigned int size,
                            unsigned int n_bits,
                            int has_cover )
: uvm_reg( (name.empty())? sc_core::sc_gen_unique_name("reg_fifo") : name , n_bits, has_cover)
{
  m_size = size;
  m_value = NULL;
  m_set_cnt = 0;
}

//----------------------------------------------------------------------
// member function: build (virtual)
//
//! Builds the abstract FIFO register object. Called by
//! the instantiating block, a uvm_reg_block subtype.
//----------------------------------------------------------------------

void uvm_reg_fifo::build()
{
  m_value = uvm_reg_field::type_id::create("value");
  sc_bv<32> reset_value = 0x0;
  m_value->configure(this, get_n_bits(), 0, "RW", false, reset_value, true, false, true);
}


//----------------------------------------------------------------------
// Function: set_compare
//
//! Sets the compare policy during a mirror (read) of the DUT FIFO.
//! The DUT read value is checked against its mirror only when both the
//! \p check argument in the mirror() call and the compare policy
//! for the field is UVM_CHECK.
//----------------------------------------------------------------------

void uvm_reg_fifo::set_compare( uvm_check_e check )
{
  m_value->set_compare(check);
}



//----------------------------------------------------------------------
// Group: Introspection
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: size
//
//! The number of entries currently in the FIFO.
//----------------------------------------------------------------------

unsigned int uvm_reg_fifo::size()
{
  return fifo.size();
}


//----------------------------------------------------------------------
// member function: capacity
//
//! The maximum number of entries, or depth, of the FIFO.
//----------------------------------------------------------------------

unsigned int uvm_reg_fifo::capacity()
{
  return m_size;
}

//----------------------------------------------------------------------
// Group: Access
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: write (virtual)
//
//! Pushes the given value to the DUT FIFO. If auto-predition is enabled,
//! the written value is also pushed to the abstract FIFO before the
//! call returns. If auto-prediction is not enabled (see
//! uvm_map::set_auto_predict), the value is pushed to abstract
//! FIFO only when the write operation is observed on the target bus.
//! This mode requires using the #uvm_reg_predictor<BUSTYPE> class.
//! If the write is via an update() operation, the abstract FIFO
//! already contains the written value and is thus not affected by
//! either prediction mode.
//----------------------------------------------------------------------

// inherted from base class

//----------------------------------------------------------------------
//  member function: read (virtual)
//
//  Reads the next value out of the DUT FIFO. If auto-prediction is
//  enabled, the frontmost value in abstract FIFO is popped.
//----------------------------------------------------------------------

bool uvm_reg_fifo::set_0(uvm_reg_data_t& value)
{
  value &= ((1 << get_n_bits())-1);
  if (fifo.size() == m_size) {
    return 1;
  }
  else {
    return 0;
  }
}

void uvm_reg_fifo::set_1()
{
  m_set_cnt++;
  fifo.push_back(m_value->m_value);
}

//----------------------------------------------------------------------
// member function: update (virtual)
//
//! Pushes (writes) all values preloaded using set(() to the DUT>.
//! You must #update after #set before any blocking statements,
//! else other reads/writes to the DUT FIFO may cause the mirror to
//! become out of sync with the DUT.
//----------------------------------------------------------------------

void uvm_reg_fifo::update( uvm_status_e&      status,
                           uvm_path_e         path,
                           uvm_reg_map*       map,
                           uvm_sequence_base* parent,
                           int                prior,
                           uvm_object*        extension,
                           const std::string&             fname,
                           int                lineno )
{
  uvm_reg_data_t upd;

  if (!m_set_cnt || fifo.size() == 0)
    return;

  m_update_in_progress = true;

  for( int i = fifo.size() - m_set_cnt;
       m_set_cnt > 0;
       i++, m_set_cnt--)
  {
      if (i >= 0)
      {
        // commented in UVM-SV
        //uvm_reg_data_t val = get();
        //super.update(status,path,map,parent,prior,extension,fname,lineno);
        write(status, fifo[i], path, map, parent, prior, extension, fname, lineno);
      }
  }
  m_update_in_progress = false;
}

//----------------------------------------------------------------------
// member function: mirror (virtual)
//
//! Reads the next value out of the DUT FIFO. If auto-prediction is
//! enabled, the frontmost value in abstract FIFO is popped. If
//! the \p check argument is set and comparison is enabled with
//! set_compare().
//----------------------------------------------------------------------

// inherited from base class

//----------------------------------------------------------------------
// member function: get (virtual)
//
//! Returns the next value from the abstract FIFO, but does not pop it.
//! Used to get the expected value in a mirror() operation.
//----------------------------------------------------------------------

uvm_reg_data_t uvm_reg_fifo::get( const std::string& fname, int lineno ) const
{
  //return fifo.pop_front();
  return fifo[0];
}

//----------------------------------------------------------------------
// member function: do_predict
//
//! Updates the abstract (mirror) FIFO based on write() and
//! read() operations.  When auto-prediction is on, this method
//! is called before each read, write, peek, or poke operation returns.
//! When auto-prediction is off, this method is called by a
//! #uvm_reg_predictor upon receipt and conversion of an observed bus
//! operation to this register.
//!
//! If a write prediction, the observed
//! write value is pushed to the abstract FIFO as long as it is
//! not full and the operation did not originate from an update().
//! If a read prediction, the observed read value is compared
//! with the frontmost value in the abstract FIFO if set_compare()
//! enabled comparison and the FIFO is not empty.
//----------------------------------------------------------------------

void uvm_reg_fifo::do_predict( uvm_reg_item*     rw,
                               uvm_predict_e     kind,
                               uvm_reg_byte_en_t be )
{
  // TODO do_predict
}

//----------------------------------------------------------------------
// Group: Special Overrides
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: pre_write (virtual)
//
//! Special pre-processing for a write() or update().
//! Called as a result of a write() or update(). It is an error to
//! attempt a write to a full FIFO or a write while an update is still
//! pending. An update is pending after one or more calls to set().
//! If in your application the DUT allows writes to a full FIFO, you
//! must override #pre_write as appropriate.
//----------------------------------------------------------------------

void uvm_reg_fifo::pre_write( uvm_reg_item* rw )
{
  if (m_set_cnt && !m_update_in_progress)
  {
    UVM_ERROR("Needs Update","Must call update() after set() and before write()");
    rw->status = UVM_NOT_OK;
    return;
  }

  if (fifo.size() >= m_size && !m_update_in_progress)
  {
    UVM_ERROR("FIFO Full","Write to full FIFO ignored");
    rw->status = UVM_NOT_OK;
    return;
  }
}

//----------------------------------------------------------------------
// member function: pre_read (virtual)
//
//! Special post-processing for a write() or update().
//! Aborts the operation if the internal FIFO is empty. If in your application
//! the DUT does not behave this way, you must override #pre_write as
//! appropriate.
//----------------------------------------------------------------------

void uvm_reg_fifo::pre_read( uvm_reg_item* rw )
{
  // abort if fifo empty
  if (fifo.size() == 0)
  {
    rw->status = UVM_NOT_OK;
    return;
  }
}

//----------------------------------------------------------------------
// member function: post_randomize
//
//! Implementation defined
//----------------------------------------------------------------------

void uvm_reg_fifo::post_randomize()
{
  m_set_cnt = 0;
}


} // namespace uvm
