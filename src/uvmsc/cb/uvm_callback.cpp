//----------------------------------------------------------------------
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
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

#include <sstream>

#include "uvmsc/cb/uvm_callback.h"
#include "uvmsc/macros/uvm_callback_defines.h"

namespace uvm {

//------------------------------------------------------------------------------
// static data member initialization
//------------------------------------------------------------------------------

const std::string uvm_callback::type_name = "uvm::uvm_callback";

//----------------------------------------------------------------------
// Constructor
//
//! Creates a new uvm_callback object, giving it an optional \p name.
//----------------------------------------------------------------------

uvm_callback::uvm_callback( const std::string& name ) : uvm_object(name)
{
  m_enabled = true;
}


//----------------------------------------------------------------------
// Function: callback_mode
//
//! Enable/disable callbacks (modeled like rand_mode and constraint_mode).
//----------------------------------------------------------------------

bool uvm_callback::callback_mode( int on  )
{
  if( on == 0 || on == 1)
  {
    std::ostringstream str;
    str << "Setting callback mode for "
        << get_name()
        << " to "
        << ( (on==1) ? "ENABLED" : "DISABLED");
    UVM_CB_TRACE_NOOBJ(this,str.str())
  }
  else
  {
    std::ostringstream str;
    str << "Callback mode for "
        << get_name()
        << " is "
        << ((m_enabled==1) ? "ENABLED":"DISABLED");
    UVM_CB_TRACE_NOOBJ(this, str.str())
  }

  bool mode = m_enabled;

  if(on == 0) m_enabled = false;
  if(on == 1) m_enabled = true;

  return mode;
}

//----------------------------------------------------------------------
// member function: is_enabled
//
//! Returns true if the callback is enabled, otherwise it return false.
//----------------------------------------------------------------------

bool uvm_callback::is_enabled()
{
  return callback_mode();
}

//----------------------------------------------------------------------
// member function: get_type_name
//
//! Returns the type name of this callback object.
//----------------------------------------------------------------------

const std::string uvm_callback::get_type_name() const

{
  return type_name;
}


} // namespace uvm
