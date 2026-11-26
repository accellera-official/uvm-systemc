//----------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Synopsys, Inc.
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

#ifndef UVM_CALLBACKS_BASE_H_
#define UVM_CALLBACKS_BASE_H_

#include <vector>
#include <map>

#include "uvmsc/base/uvm_object.h"
#include "uvmsc/misc/uvm_misc.h"  // for uvm_apprepend
#include "uvmsc/cb/uvm_typeid.h"  // for uvm_typeid_base
#include "uvmsc/conf/uvm_queue.h" // for uvm_queue

namespace uvm {


// forward class declarations
class uvm_callback;
class uvm_default_coreservice_t;

//------------------------------------------------------------------------------
// Class - uvm_callbacks_base
//
// Implementation defined
//
//! Base class singleton that holds generic queues for all instance
//! specific objects. This is an internal class. This class contains a
//! global pool that has all of the instance specific callback queues in it.
//! All of the typewide callback queues live in the derivative class
//! #uvm_typed_callbacks<T>. This is not a user visible class.
//!
//! This class holds the class inheritance hierarchy information
//! (super types and derivative types).
//!
//! Note, all derivative uvm_callbacks class singletons access this
//! global m_pool object in order to get access to their specific
//! instance queue.
//------------------------------------------------------------------------------

class uvm_callbacks_base : public uvm_object
{
 public:
  static uvm_callbacks_base* m_initialize();
  static bool& tracing_enabled();
  static std::map<uvm_object*, uvm_queue<uvm_callback*>*>& callback_pool();
  static uvm_callbacks_base* base_instance();

  virtual bool m_am_i_a( uvm_object* obj );

  virtual bool m_is_for_me( uvm_callback* cb );

  virtual bool m_is_registered( uvm_object* obj, uvm_callback* cb );

  virtual uvm_queue<uvm_callback*>* m_get_tw_cb_q( uvm_object* obj );

  virtual void m_add_tw_cbs( uvm_callback* cb, uvm_apprepend ordering );

  virtual bool m_delete_tw_cbs( uvm_callback* cb );

  bool check_registration( uvm_object* obj, uvm_callback* cb );

 protected:
  friend class uvm_default_coreservice_t;
  uvm_callbacks_base();

 public:  // TODO make some members private
  // data members

  std::vector<uvm_callbacks_base*> m_this_type;  // one to many T->T/CB
  uvm_typeid_base m_super_type;             // one to one relation
  std::vector<uvm_typeid_base*> m_derived_types; // one to many relation

}; // class uvm_callbacks_base

} // namespace uvm

#endif // UVM_CALLBACKS_BASE_H_
