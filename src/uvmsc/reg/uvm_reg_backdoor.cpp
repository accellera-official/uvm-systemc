//----------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2004-2009 Synopsys, Inc.
//   Copyright 2010-2011 Mentor Graphics Corporation
//   Copyright 2010 Cadence Design Systems, Inc.
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
#include "sysc/kernel/sc_dynamic_processes.h"

#include "uvmsc/base/uvm_globals.h"
#include "uvmsc/macros/uvm_message_defines.h"
#include "uvmsc/reg/uvm_reg.h"
#include "uvmsc/reg/uvm_reg_backdoor.h"
#include "uvmsc/reg/uvm_reg_item.h"
#include "uvmsc/reg/uvm_reg_field.h"
#include "uvmsc/reg/uvm_reg_cbs.h"

namespace uvm {


//----------------------------------------------------------------------
// static data member initialization
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Constructor
//
//! Create an instance of this class
//!
//! Create an instance of the user-defined backdoor class
//! for the specified register or memory
//----------------------------------------------------------------------

uvm_reg_backdoor::uvm_reg_backdoor( const std::string& name ) : uvm_object(name)
{
  m_update_thread.clear();
  m_fname = "";
  m_lineno = 0;

}
   
//----------------------------------------------------------------------
// member function: do_pre_read
//
//! Execute the pre-read callbacks
//!
//! This method must be called as the first statement in
//! a user extension of the read() method.
//----------------------------------------------------------------------

void uvm_reg_backdoor::do_pre_read( uvm_handle<uvm_reg_item>  rw )
{
  pre_read(*rw);
  UVM_DO_OBJ_CALLBACKS( uvm_reg_backdoor, uvm_reg_cbs, this,
                        pre_read(*rw) )
}


//----------------------------------------------------------------------
// member function: do_post_read
//
//! Execute the post-read callbacks
//!
//! This method must be called as the last statement in
//! a user extension of the read() method.
//----------------------------------------------------------------------

void uvm_reg_backdoor::do_post_read( uvm_handle<uvm_reg_item>  rw )
{
  uvm_callback_iter<uvm_reg_backdoor, uvm_reg_cbs>* iter =
      new uvm_callback_iter<uvm_reg_backdoor, uvm_reg_cbs>(this);

  for( uvm_reg_cbs* cb = iter->last(); cb != nullptr; cb = iter->prev())
    cb->decode(rw->value);

  UVM_DO_OBJ_CALLBACKS(uvm_reg_backdoor, uvm_reg_cbs, this, post_read(*rw))

  post_read(*rw);
}

//----------------------------------------------------------------------
// member function: do_pre_write
//
//! Execute the pre-write callbacks
//!
//! This method must be called as the first statement in
//! a user extension of the write() method.
//----------------------------------------------------------------------

void uvm_reg_backdoor::do_pre_write( uvm_handle<uvm_reg_item>  rw )
{
  uvm_callback_iter<uvm_reg_backdoor, uvm_reg_cbs>* iter =
      new uvm_callback_iter<uvm_reg_backdoor, uvm_reg_cbs>(this);

  pre_write(*rw);

  UVM_DO_OBJ_CALLBACKS(uvm_reg_backdoor, uvm_reg_cbs, this, pre_write(*rw))

  for( uvm_reg_cbs* cb = iter->first(); cb != nullptr; cb = iter->next())
    cb->encode(rw->value);
}


//----------------------------------------------------------------------
// member function: do_post_write
//
//! Execute the post-write callbacks
//!
//! This method must be called as the last statement in
//! a user extension of the write() method.
//----------------------------------------------------------------------

void uvm_reg_backdoor::do_post_write( uvm_handle<uvm_reg_item>  rw )
{
  UVM_DO_OBJ_CALLBACKS(uvm_reg_backdoor, uvm_reg_cbs, this, post_write(*rw))

      post_write(*rw);
}


//----------------------------------------------------------------------
// member function: write
//
//! User-defined backdoor write operation.
//!
//! Call do_pre_write().
//! Deposit the specified value in the specified register HDL implementation.
//! Call do_post_write().
//! Returns an indication of the success of the operation.
//----------------------------------------------------------------------

void uvm_reg_backdoor::write( uvm_handle<uvm_reg_item>  rw )
{
  UVM_FATAL("RegModel", "Member function uvm_reg_backdoor::write() has not been overloaded");
}

//----------------------------------------------------------------------
// member function: read
//
//! User-defined backdoor read operation.
//!
//! Overload this method only if the backdoor requires the use of task.
//!
//! Call do_pre_read().
//! Peek the current value of the specified HDL implementation.
//! Call do_post_read().
//! Returns the current value and an indication of the success of
//! the operation.
//!
//! By default, calls read_func().
//----------------------------------------------------------------------

void uvm_reg_backdoor::read( uvm_handle<uvm_reg_item>  rw )
{
  do_pre_read(rw);
  read_func(rw);
  do_post_read(rw);
}

//----------------------------------------------------------------------
// member function: read_func
//
//! User-defined backdoor read operation.
//!
//! Peek the current value in the HDL implementation.
//! Returns the current value and an indication of the success of
//! the operation.
//----------------------------------------------------------------------

void uvm_reg_backdoor::read_func( uvm_handle<uvm_reg_item>  rw )
{
  UVM_FATAL("RegModel", "Member function uvm_reg_backdoor::read_func() has not been overloaded");
  rw->status = UVM_NOT_OK;
}

//----------------------------------------------------------------------
// member function: is_auto_updated
//
//! Indicates if wait_for_change() method is implemented
//!
//! Implement to return true if and only if
//! wait_for_change() is implemented to watch for changes
//! in the HDL implementation of the specified field
//----------------------------------------------------------------------

bool uvm_reg_backdoor::is_auto_updated( uvm_reg_field* field )
{
  return false;
}

//----------------------------------------------------------------------
// member function: wait_for_change
//
//! Wait for a change in the value of the register or memory
//! element in the DUT.
//!
//! When this method returns, the mirror value for the register
//! corresponding to this instance of the backdoor class will be updated
//! via a backdoor read operation.
//----------------------------------------------------------------------

void uvm_reg_backdoor::wait_for_change( uvm_object* element )
{
  UVM_FATAL("RegModel", "Member function uvm_reg_backdoor::wait_for_change() has not been overloaded");
}

//----------------------------------------------------------------------
// member function: pre_read
//
//! Called before user-defined backdoor register read.
//!
//! The registered callback methods are invoked after the invocation
//! of this method.
//----------------------------------------------------------------------

void uvm_reg_backdoor::pre_read( uvm_reg_item&  rw )
{}

//----------------------------------------------------------------------
// member function: post_read
//
//! Called after user-defined backdoor register read.
//!
//! The registered callback methods are invoked before the invocation
//! of this method.
//----------------------------------------------------------------------

void uvm_reg_backdoor::post_read( uvm_reg_item&  rw )
{}

//----------------------------------------------------------------------
// member function: pre_write
//
//! Called before user-defined backdoor register write.
//!
//! The registered callback methods are invoked after the invocation
//! of this method.
//!
//! The written value, if modified, modifies the actual value that
//! will be written.
//----------------------------------------------------------------------

void uvm_reg_backdoor::pre_write( uvm_reg_item&  rw )
{}

//----------------------------------------------------------------------
// member function: post_write
//
//! Called after user-defined backdoor register write.
//!
//! The registered callback methods are invoked before the invocation
//! of this method.
//----------------------------------------------------------------------

void uvm_reg_backdoor::post_write( uvm_reg_item&  rw )
{}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------
// member function: start_update_thread
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_backdoor::start_update_thread( uvm_object* element )
{
  uvm_reg* rg = nullptr;

  // if already exists, kill previous thread
  if ( m_update_thread.find(element) != m_update_thread.end()) // if exists
  {
     UVM_WARNING("RegModel", "Existing backdoor update thread found. Previous one is being killed");
     kill_update_thread(element);
  }

  rg = dynamic_cast<uvm_reg*>(element);
  if (rg == nullptr)
    return; // only regs supported at this time

  m_update_thread[element] =
    sc_core::sc_spawn(sc_bind(&uvm_reg_backdoor::start_update_thread_core, this, element, rg));
}

//----------------------------------------------------------------------
// member function: start_update_thread_core
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_backdoor::start_update_thread_core( uvm_object* element, uvm_reg* rg )
{
  std::vector<uvm_reg_field*> fields;

  if (rg == nullptr)
    return; // only regs supported at this time
  else
    rg->get_fields(fields);

  while(true) // forever
  {
     uvm_reg_data_t  val;

     uvm_handle<uvm_reg_item>  r_item =
       uvm_reg_item::type_id::create_handle("bd_r_item");

     r_item->element = rg;
     r_item->element_kind = UVM_REG;

     this->read(r_item);
     val = r_item->value[0];

     if (r_item->status != UVM_IS_OK)
     {
        UVM_ERROR("RegModel", "Backdoor read of register '" +
            rg->get_name() + "' failed.");
     }
     for( unsigned int i = 0; i < fields.size(); i++)
     {
        if (this->is_auto_updated(fields[i]))
        {
           r_item->value[0] = (val >> fields[i]->get_lsb_pos()) &
             uvm_mask_size(fields[i]->get_n_bits());
           fields[i]->do_predict(r_item);
        }
     }
     this->wait_for_change(element);
  }
}

//----------------------------------------------------------------------
// member function: kill_update_thread
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_reg_backdoor::kill_update_thread( uvm_object* element )
{
   if (m_update_thread.find(element) != m_update_thread.end()) // exists
   {
      m_update_thread[element].kill();

      //  delete the actual object/element from the stack
      m_update_thread.erase(element);
   }
}

//----------------------------------------------------------------------
// member function: has_update_threads
//
// Implementation defined
//----------------------------------------------------------------------

bool uvm_reg_backdoor::has_update_threads()
{
  return m_update_thread.size() > 0;
}

/////////////

} // namespace uvm
