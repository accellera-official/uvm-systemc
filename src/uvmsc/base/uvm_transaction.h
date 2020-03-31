//----------------------------------------------------------------------
//   Copyright 2012-2015 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
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

#ifndef UVM_TRANSACTION_H_
#define UVM_TRANSACTION_H_

#include <systemc>

#include "uvmsc/base/uvm_object.h"
#include "uvmsc/base/uvm_event.h"

namespace uvm {

// forward class declarations
class uvm_printer;
class uvm_component;

//------------------------------------------------------------------------------
// CLASS: uvm_transaction
//
//! The uvm_transaction class is the root base class for UVM transactions.
//! Inheriting all the methods of uvm_object, uvm_transaction adds a timing and
//! recording interface.
//!
//! This class provides timestamp properties, notification events, and transaction
//! recording support.
//!
//! Derived from uvm_object.
//------------------------------------------------------------------------------

class uvm_transaction : public uvm_object
{
 public:
  friend class uvm_reg_map;

  uvm_transaction();

  explicit uvm_transaction( uvm_object_name name );

  void set_transaction_id(int id);

  int get_transaction_id() const;

  // Transaction recording

  int begin_tr( const sc_core::sc_time& begin_time = sc_core::SC_ZERO_TIME );

  void end_tr( const sc_core::sc_time& end_time = sc_core::SC_ZERO_TIME,
               bool free_handle = true );


  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

protected:

  virtual ~uvm_transaction();

  virtual void do_print( const uvm_printer& printer ) const;

private:

  void m_init_transaction();

  // member variables
  int m_transaction_id; /*!< The transaction id data member. */
  uvm_component* initiator;

  sc_core::sc_time begin_time;
  sc_core::sc_time end_time;
  sc_core::sc_time accept_time;

  uvm_event end_event;

};

} /* namespace uvm */

#endif /* UVM_TRANSACTION_H_ */
