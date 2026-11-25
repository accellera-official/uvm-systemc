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
#ifndef UVM_SET_BEFORE_GET_DAP_H_
#define UVM_SET_BEFORE_GET_DAP_H_

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
// Class: uvm_set_before_get_dap
// Provides a 'Set Before Get' Data Access Policy.
//
// The 'Set Before Get' Data Access Policy enforces that the value must
// be written at ~least~ once before it is read.  This DAP can be used to
// pass shared information to multiple components during standard configuration,
// even if that information hasn't yet been determined.
//
// Such DAP objects can be useful for passing a 'placeholder' reference, before
// the information is actually available.
//----------------------------------------------------------------------------

template <typename T = int>
class uvm_set_before_get_dap : public uvm_set_get_dap_base<T>
{
 public:
  typedef uvm_set_before_get_dap<T> this_type;

  UVM_OBJECT_PARAM_UTILS(uvm_set_before_get_dap<T>);

  uvm_set_before_get_dap( const std::string& name = "unnamed-uvm_set_before_get_dap<T>");

  // Group: Set/Get Interface

  virtual void set( const T& value );
  virtual bool try_set( const T& value );
  virtual T get() const;
  virtual bool try_get( T& value ) const; 

  // Group: Introspection

  virtual void do_copy( const uvm_object& rhs ) const;
  virtual void do_pack( uvm_packer& packer ) const;
  virtual void do_unpack( uvm_packer& packer );

  // Group: Reporting

  virtual std::string convert2string() const;
  virtual void do_print( const uvm_printer& printer ) const;
   
 private:
  // Stored data
  T m_value;

  // Set state
  bool m_set;

}; // class uvm_set_before_get_dap


//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

template <typename T>
uvm_set_before_get_dap<T>::uvm_set_before_get_dap( const std::string& name )
: uvm_set_get_dap_base<T>(name), m_value(0), m_set(false)
{}



//----------------------------------------------------------------------------
// member function: set (virtual)
// Updates the value stored within the DAP.
//----------------------------------------------------------------------------

template <typename T>
void uvm_set_before_get_dap<T>::set( const T& value )
{
  m_set = true;
  m_value = value;
}

//----------------------------------------------------------------------------
// member function: try_set (virtual)
// Attempts to update the value stored within the DAP.
//
// try_set will always return true.
//----------------------------------------------------------------------------

template <typename T>
bool uvm_set_before_get_dap<T>::try_set( const T& value )
{
  this->set(value);
  return true;
}

//----------------------------------------------------------------------------
// member function: get
// Returns the current value stored within the DAP.
//
// If 'get' is called before a call to <set> or <try_set>, then
// an error will be reported.
//----------------------------------------------------------------------------

template <typename T>
T uvm_set_before_get_dap<T>::get() const
{
  if (!m_set)
  {
    std::ostringstream msg;
    msg << "Attempt to get value on '"
        << this->get_full_name()
        << "', but the data access policy forbits calling 'get' prior to calling 'set' or 'try_set'!";
    UVM_ERROR("UVM/SET_BEFORE_GET_DAP/NO_SET", msg.str());
  }
 return m_value;
}

//----------------------------------------------------------------------------
// member function: try_get (virtual)
// Attempts to retrieve the current value stored within the DAP
//
// If the value has not been 'set', then try_get will return true,
// otherwise it will return false, and set value to the current
// value stored within the DAP.
//----------------------------------------------------------------------------

template <typename T>
bool uvm_set_before_get_dap<T>::try_get( T& value ) const
{
  if (!m_set)
   return false;
  else
  {
    value = m_value;
    return true;
  }
}

//----------------------------------------------------------------------------
// The uvm_set_before_get_dap cannot support the standard UVM
// instrumentation methods (copy, clone, pack and
// unpack), due to the fact that they would potentially
// violate the access policy.
//
// A call to any of these methods will result in an error.
//----------------------------------------------------------------------------

template <typename T>
void uvm_set_before_get_dap<T>::do_copy( const uvm_object& rhs ) const
{
  UVM_ERROR("UVM/SET_BEFORE_GET_DAP/CPY",
    "do_copy is not supported for uvm_set_before_get_dap<T>");
}

template <typename T>
void uvm_set_before_get_dap<T>::do_pack( uvm_packer& packer ) const
{
  UVM_ERROR("UVM/SET_BEFORE_GET_DAP/PCK",
    "do_pack' is not supported for uvm_set_before_get_dap<T>");
}

template <typename T>
void uvm_set_before_get_dap<T>::do_unpack( uvm_packer& packer )
{
  UVM_ERROR("UVM/SET_BEFORE_GET_DAP/UPK",
    "do_unpack is not supported for uvm_set_before_get_dap<T>");
}


//----------------------------------------------------------------------------
// member function: convert2string (virtual)
//----------------------------------------------------------------------------

template <typename T>
std::string uvm_set_before_get_dap<T>::convert2string() const
{
  if (m_set)
  {
    std::ostringstream msg;
    msg << "(" << UVM_TYPENAME(m_value)
        << ") " << m_value
        << " [SET]";
    return msg.str();
  }
  else
  {
    std::ostringstream msg;
    msg << "(" << UVM_TYPENAME(m_value)
        << ") " << m_value
        << " [UNSET]";
    return msg.str();
  }
}

//----------------------------------------------------------------------------
// member function: do_print (virtual)
//----------------------------------------------------------------------------

template <typename T>
void uvm_set_before_get_dap<T>::do_print( const uvm_printer& printer ) const
{
  uvm_set_get_dap_base<T>::do_print(printer);
  printer.print_field_int("set_state", m_set);
  std::ostringstream s;
  s << m_value;
  printer.print_generic("value", UVM_TYPENAME(m_value), 0, s.str());
}


} // namespace uvm

#endif // UVM_SET_BEFORE_GET_DAP_H_
