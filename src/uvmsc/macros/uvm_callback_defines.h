//----------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
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

#ifndef UVM_CALLBACK_DEFINES_H_
#define UVM_CALLBACK_DEFINES_H_

#include "uvmsc/macros/uvm_message_defines.h"

//-----------------------------------------------------------------------------
// MACRO: UVM_REGISTER_CB
//
//! Registers the given \p CB callback type with the given \p T object type. If
//! a type-callback pair is not registered then a warning is issued if an
//! attempt is made to use the pair (add, delete, etc.).
//!
//! The registration will typically occur in the component that executes the
//! given type of callback.
//-----------------------------------------------------------------------------

#define UVM_REGISTER_CB(T,CB) \
  bool m_register_cb() override { \
  return ::uvm::uvm_callbacks<T,CB>::m_register_pair(#T,#CB); }

//-----------------------------------------------------------------------------
// MACRO: UVM_DO_CALLBACKS
//
//! Calls the given \p METHOD of all callbacks of type \p CB registered with
//! the calling object (i.e. the this object), which is or is based on type \p T.
//!
//! This macro executes all of the callbacks associated with the calling
//! object (i.e. the this object). The macro takes three arguments:
//!
//! - CB is the class type of the callback objects to execute. The class
//!   type must have a function signature that matches the METHOD argument.
//!
//! - T is the type associated with the callback. Typically, an instance
//!   of type T is passed as one the arguments in the \p METHOD call.
//!
//! - METHOD is the method call to invoke, with all required arguments as
//!   if they were invoked directly.
//-----------------------------------------------------------------------------

#define UVM_DO_CALLBACKS(T,CB,METHOD) \
  UVM_DO_OBJ_CALLBACKS(T,CB,this,METHOD)

//-----------------------------------------------------------------------------
// MACRO: UVM_DO_OBJ_CALLBACKS
//
//! Calls the given \p METHOD of all callbacks based on type \p CB registered with
//! the given object, \p OBJ, which is or is based on type \p T.
//!
//! This macro is identical to macro UVM_DO_CALLBACKS,
//! but it has an additional \p OBJ argument to allow the specification of an
//! external object to associate the callback with. For example, if the
//! callbacks are being applied in a sequence, argument \p OBJ could be specified
//! as the associated sequencer or parent sequence.
//-----------------------------------------------------------------------------

#define UVM_DO_OBJ_CALLBACKS(T,CB,OBJ,METHOD) \
   { \
     ::uvm::uvm_callback_iter<T,CB>* iter = new ::uvm::uvm_callback_iter<T,CB>(OBJ); \
     CB* cb = iter->first(); \
     while(cb != nullptr) { \
       /*UVM_CB_TRACE_NOOBJ(cb, "Executing callback method 'METHOD' for callback " + cb->get_name() + " (CB) from " + OBJ->get_full_name() + " (T)")*/ \
       UVM_CB_TRACE_NOOBJ(cb, "Test") \
       cb->METHOD; \
       cb = iter->next(); \
     } \
   }


//-----------------------------------------------------------------------------
// MACRO: UVM_DO_CALLBACKS_EXIT_ON
//
//! Calls the given \p METHOD of all callbacks of type \p CB registered with
//! the calling object (i.e. the this object), which is or is based on type \p T,
//! returning upon the first callback returning the bit value given by \p VAL.
//!
//! This macro executes all of the callbacks associated with the calling
//! object (i.e. the this object). The macro takes three arguments:
//!
//! - CB is the class type of the callback objects to execute. The class
//!   type must have a function signature that matches the METHOD argument.
//!
//! - T is the type associated with the callback. Typically, an instance
//!   of type T is passed as one the arguments in the \p METHOD call.
//!
//! - METHOD is the method call to invoke, with all required arguments as
//!   if they were invoked directly.
//!
//! - VAL, if true, says return upon the first callback invocation that
//!   returns true. If false, says return upon the first callback invocation that
//!   returns false.
//!
//! Because this macro calls return, its use is restricted to implementations
//! of functions that return a boolean value, as in the above example.
//-----------------------------------------------------------------------------

#define UVM_DO_CALLBACKS_EXIT_ON(T,CB,METHOD,VAL) \
  UVM_DO_OBJ_CALLBACKS_EXIT_ON(T,CB,this,METHOD,VAL) \

//-----------------------------------------------------------------------------
// MACRO: UVM_DO_OBJ_CALLBACKS_EXIT_ON
//
//! Calls the given \p METHOD of all callbacks of type \p CB registered with
//! the given object \p OBJ, which must be or be based on type \p T, and returns
//! upon the first callback that returns the bit value given by \p VAL. It is
//! exactly the same as the macro UVM_DO_CALLBACKS_EXIT_ON but has a specific
//! object instance (instead of the implicit this instance) as the third
//! argument.
//!
//! Because this macro calls return, its use is restricted to implementations
//! of functions that return a boolean value, as in the above example.
//-----------------------------------------------------------------------------

#define UVM_DO_OBJ_CALLBACKS_EXIT_ON(T,CB,OBJ,METHOD,VAL) \
{ \
  ::uvm::uvm_callback_iter<T,CB>* iter = new ::uvm::uvm_callback_iter<T,CB>(OBJ); \
  CB* cb = iter->first(); \
  while(cb != nullptr) { \
    if (cb->METHOD == true) { \
      UVM_CB_TRACE_NOOBJ(cb, "Executed callback method 'METHOD' for callback " + cb->get_name() + " (CB) from " + OBJ->get_full_name() + " (T) : returned value VAL (other callbacks will be ignored)") \
      return VAL; \
    } \
    UVM_CB_TRACE_NOOBJ(cb, "Executed callback method 'METHOD' for callback " + cb->get_name() + " (CB) from " + OBJ->get_full_name() + " (T) : did not return value VAL") \
    cb = iter->next(); \
  } \
  return !VAL; \
}

// The +define+UVM_CB_TRACE_ON setting will instrument the uvm library to emit 
// messages with message id UVMCB_TRC and UVM_NONE verbosity 
// notifying add,delete and execution of uvm callbacks. The instrumentation is off by default.

#ifdef UVM_CB_TRACE_ON

#define UVM_CB_TRACE(OBJ,CB,OPER) \
  { \
    std::ostringstream msg; \
    std::ostringstream str; \
    inf << OBJ->get_full_name() << " (" << OBJ->get_type_name() << "@" \
        << OBJ->get_inst_id() << ")" \
    str << OPER << ": callback " <<  CB->get_name() << " (" \
        << CB->get_type_name() << "@" << CB->get_inst_id() << ") : to object " \
        << ( (OBJ == nullptr) ? "nullptr" : inf.str() ); \
    UVM_INFO("UVMCB_TRC", str.str(), UVM_NONE); \
  }

#define UVM_CB_TRACE_NOOBJ(CB,OPER) \
  { \
    if(::uvm::uvm_callbacks_base::m_tracing) { \
      std::ostringstream _str; \
      _str << OPER << ": callback " << CB->get_name() << " (" \
        << CB->get_type_name() << "@" << CB->get_inst_id() << ")"; \
      UVM_INFO("UVMCB_TRC", _str.str(), UVM_NONE); \
    } \
  }
#else

#define UVM_CB_TRACE_NOOBJ(CB,OPER) /* nullptr */
#define UVM_CB_TRACE(OBJ,CB,OPER) /* nullptr */

#endif // UVM_CB_TRACE_ON

#endif // UVM_CALLBACK_DEFINES_H_
