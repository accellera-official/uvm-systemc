//------------------------------------------------------------------------------
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Synopsys, Inc.
//   Copyright 2013      NVIDIA Corporation
//   Copyright 2019      NXP B.V.
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
//------------------------------------------------------------------------------

#ifndef UVM_SIMPLE_LOCK_DAP_H_
#define UVM_SIMPLE_LOCK_DAP_H_

#include <string>
#include <sstream>

#include "uvmsc/dap/uvm_set_get_dap_base.h"
#include "uvmsc/print/uvm_printer.h"
#include "uvmsc/factory/uvm_object_registry.h"
#include "uvmsc/macros/uvm_defines.h"
#include "uvmsc/macros/uvm_object_defines.h"
#include "uvmsc/macros/uvm_message_defines.h"

namespace uvm {

//----------------------------------------------------------------------------
// Class: uvm_simple_lock_dap
// Provides a 'Simple Lock' Data Access Policy.
//
// The 'Simple Lock' Data Access Policy allows for any number of 'sets',
// so long as the value is not 'locked'.  The value can be retrieved using
// 'get' at any time.
//
// The UVM uses this policy to protect the file name value in the
// uvm_text_tr_database.
//----------------------------------------------------------------------------

template <typename T = int>
class uvm_simple_lock_dap : public uvm_set_get_dap_base<T>
{
 public:
  typedef uvm_simple_lock_dap<T> this_type;
   
  UVM_OBJECT_PARAM_UTILS(uvm_simple_lock_dap<T>);

  uvm_simple_lock_dap( const std::string& name = "unnamed-uvm_simple_lock_dap<T>");

  // Group: Set/Get Interface

  virtual void set( const T& value );
  virtual bool try_set( const T& value );
  virtual  T get();
  virtual bool try_get( T& value);

  // Group: Locking

  void lock();
  void unlock();
  bool is_locked() const;
   
   // Group: Introspection

  virtual void do_copy( const uvm_object& rhs ) const;
  virtual void do_pack( uvm_packer& packer ) const;
  virtual void do_unpack( uvm_packer& packer );

  // Group: Reporting

  virtual std::string convert2string() const;
  virtual void do_print( const uvm_printer& printer ) const;

 private: // local vars
  // Stored data
  T m_value;

  // Lock state
  bool m_locked;

}; // class uvm_simple_lock_dap


//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

template <typename T>
uvm_simple_lock_dap<T>::uvm_simple_lock_dap( const std::string& name )
: uvm_set_get_dap_base<T>(name), m_value(0), m_locked(false)
{}

//----------------------------------------------------------------------------
// member function: set (virtual)
// Updates the value stored within the DAP.
//
// The member function set will result in an error if the DAP has
// been locked.
//----------------------------------------------------------------------------

template <typename T>
void uvm_simple_lock_dap<T>::set( const T& value )
{
  if (m_locked)
  {
    std::ostringstream msg;
    msg << "Attempt to set new value on '"
        << this->get_full_name()
        << "', but the data access policy forbids setting while lockted!";
    UVM_ERROR("UVM/SIMPLE_LOCK_DAP/SAG", msg.str());
  }
  else
    m_value = value;
}

//----------------------------------------------------------------------------
// member function: try_set (virtual)
// Attempts to update the value stored within the DAP.
//
// try_set will return true if the value was successfully
// updated, or false if the value can not be updated due
// to the DAP being locked.  No errors will be reported
// if try_set fails.
//----------------------------------------------------------------------------

template <typename T>
bool uvm_simple_lock_dap<T>::try_set( const T& value )
{
  if (m_locked)
    return false;
  else
  {
    m_value = value;
    return true;
  }
}

//----------------------------------------------------------------------------
// member function: get (virtual)
// Returns the current value stored within the DAP
//----------------------------------------------------------------------------

template <typename T>
T uvm_simple_lock_dap<T>::get()
{
  return m_value;
}

//----------------------------------------------------------------------------
// Function: try_get
// Retrieves the current value stored within the DAP
//
// try_get will always return true.
//----------------------------------------------------------------------------

template <typename T>
bool uvm_simple_lock_dap<T>::try_get( T& value )
{
  value = this->get();
  return true;
}

//----------------------------------------------------------------------------
// Function: lock
// Locks the data value
//
// The data value cannot be updated via <set> or <try_set> while locked.
//----------------------------------------------------------------------------

template <typename T>
void uvm_simple_lock_dap<T>::lock()
{
  m_locked = true;
}

//----------------------------------------------------------------------------
// Function: unlock
// Unlocks the data value
//----------------------------------------------------------------------------

template <typename T>
void uvm_simple_lock_dap<T>::unlock()
{
   m_locked = false;
}

//----------------------------------------------------------------------------
// Function: is_locked
// Returns the state of the lock.
//
// Returns:
// true - The value is locked
// false - The value is unlocked
//----------------------------------------------------------------------------

template <typename T>
bool uvm_simple_lock_dap<T>::is_locked() const
{
  return m_locked;
}

//----------------------------------------------------------------------------
// The uvm_simple_lock_dap cannot support the standard UVM
// instrumentation methods (copy, clone, pack and
// unpack), due to the fact that they would potentially
// violate the access policy.
//
// A call to any of these methods will result in an error.
//----------------------------------------------------------------------------

template <typename T>
void uvm_simple_lock_dap<T>::do_copy( const uvm_object& rhs ) const
{
  UVM_ERROR("UVM/SIMPLE_LOCK_DAP/CPY",
    "do_copy is not supported for uvm_simple_lock_dap<T>");
}

template <typename T>
void uvm_simple_lock_dap<T>::do_pack( uvm_packer& packer ) const
{
  UVM_ERROR("UVM/SIMPLE_LOCK_DAP/PCK",
    "do_pack' is not supported for uvm_simple_lock_dap<T>");
}

template <typename T>
void uvm_simple_lock_dap<T>::do_unpack( uvm_packer& packer )
{
  UVM_ERROR("UVM/SIMPLE_LOCK_DAP/UPK",
    "do_unpack is not supported for uvm_simple_lock_dap<T>");
}

//----------------------------------------------------------------------------
// member function: convert2string (virtual)
//----------------------------------------------------------------------------

template <typename T>
std::string uvm_simple_lock_dap<T>::convert2string() const
{
  if (m_locked)
  {
    std::ostringstream msg;
    msg << "(" << UVM_TYPENAME(m_value)
        << ") " << m_value
        << " [LOCKED]";
    return msg.str();
  }
  else
  {
    std::ostringstream msg;
    msg << "(" << UVM_TYPENAME(m_value)
        << ") " << m_value
        << " [UNLOCKED]";
    return msg.str();
  }
}

//----------------------------------------------------------------------------
// member function: do_print (virtual)
//----------------------------------------------------------------------------

template <typename T>
void uvm_simple_lock_dap<T>::do_print( const uvm_printer& printer ) const
{
  uvm_set_get_dap_base<T>::do_print(printer);
  printer.print_field_int("lock_state", m_locked);
  std::ostringstream s;
  s << m_value;
  printer.print_generic("value", UVM_TYPENAME(m_value), 0, s.str());
}


} // namespace uvm

#endif // UVM_SIMPLE_LOCK_DAP_H_
