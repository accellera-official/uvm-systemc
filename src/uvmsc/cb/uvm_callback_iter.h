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

#ifndef UVM_CALLBACK_ITER_H_
#define UVM_CALLBACK_ITER_H_

#include "uvmsc/base/uvm_object.h"
#include "uvmsc/cb/uvm_callbacks.h"
#include "uvmsc/cb/uvm_callback.h"

namespace uvm {

//----------------------------------------------------------------------
// CLASS: uvm_callback_iter
//
//! The class #uvm_callback_iter is an iterator class for iterating over
//! callback queues of a specific callback type.
//!
//! The callback iteration macros, UVM_DO_CALLBACKS and
//! UVM_DO_CALLBACKS_EXIT_ON provide a simple method for iterating
//! callbacks and executing the callback methods.
//----------------------------------------------------------------------

template < typename T = uvm_object, typename CB = uvm_callback>
class uvm_callback_iter
{
 public:
  uvm_callback_iter( T* obj );

  CB* first();

  CB* last();

  CB* next();

  CB* prev();

  CB* get_cb();

 private:
  // data members
  int  m_i;
  T*   m_obj;
  CB*  m_cb;

}; // class uvm_callback_iter

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Constructor
//
//! Creates a new callback iterator object. It is required that the object
//! context be provided.
//----------------------------------------------------------------------

template < typename T, typename CB >
uvm_callback_iter<T,CB>::uvm_callback_iter( T* obj )
{
  m_obj = obj;
  m_i = 0;
  m_cb = nullptr;
}

//----------------------------------------------------------------------
// member function: first
//
//! Returns the first valid (enabled) callback of the callback type (or
//! a derivative) that is in the queue of the context object. If the
//! queue is empty then nullptr is returned.
//----------------------------------------------------------------------

template < typename T, typename CB >
CB* uvm_callback_iter<T,CB>::first()
{
  m_cb = uvm_callbacks<T,CB>::get_first(m_i, m_obj);
  return m_cb;
}
//----------------------------------------------------------------------
// member function: last
//
//! Returns the last valid (enabled) callback of the callback type (or
//! a derivative) that is in the queue of the context object. If the
//! queue is empty then nullptr is returned.
//----------------------------------------------------------------------

template < typename T, typename CB >
CB* uvm_callback_iter<T,CB>::last()
{
  m_cb = uvm_callbacks<T,CB>::get_last(m_i, m_obj);
  return m_cb;
}
//----------------------------------------------------------------------
// member function: next
//
//! Returns the next valid (enabled) callback of the callback type (or
//! a derivative) that is in the queue of the context object. If there
//! are no more valid callbacks in the queue, then nullptr is returned.
//----------------------------------------------------------------------

template < typename T, typename CB >
CB* uvm_callback_iter<T,CB>::next()
{
  m_cb = uvm_callbacks<T,CB>::get_next(m_i, m_obj);
  return m_cb;
}
//----------------------------------------------------------------------
// member function: prev
//
//! Returns the previous valid (enabled) callback of the callback type (or
//! a derivative) that is in the queue of the context object. If there
//! are no more valid callbacks in the queue, then nullptr is returned.
//----------------------------------------------------------------------

template < typename T, typename CB >
CB* uvm_callback_iter<T,CB>::prev()
{
  m_cb = uvm_callbacks<T,CB>::get_prev(m_i, m_obj);
  return m_cb;
}
//----------------------------------------------------------------------
// member function:get_cb
//
//! Returns the last callback accessed via a first() or next()
//! call.
//----------------------------------------------------------------------

template < typename T, typename CB >
CB* uvm_callback_iter<T,CB>::get_cb()
{
  return m_cb;
}

/**** UVM-SV TODO trace?
void trace(uvm_object obj = null);
   if (m_cb != null && T::cbs::get_debug_flags() & UVM_CALLBACK_TRACE) begin
      uvm_report_object reporter = null;
      std::string who = "Executing ";
      void'($cast(reporter, obj));
      if (reporter == null) void'($cast(reporter, m_obj));
      if (reporter == null) begin
         uvm_coreservice_t cs = uvm_coreservice_t::get();
         reporter = cs.get_root();
      end
      if (obj != null) who = {obj.get_full_name(), " is executing "};
      else if (m_obj != null) who = {m_obj.get_full_name(), " is executing "};
      reporter.uvm_report_info("CLLBK_TRC", {who, "callback ", m_cb.get_name()}, UVM_LOW);
   end
}
****/

} // namespace uvm

#endif // UVM_CALLBACK_ITER_H_
