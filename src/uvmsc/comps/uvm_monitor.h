//----------------------------------------------------------------------
//   Copyright 2012-2015 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc.
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

#ifndef UVM_MONITOR_H_
#define UVM_MONITOR_H_

#include <string>

#include "uvmsc/base/uvm_component.h"
#include "uvmsc/base/uvm_component_name.h"

namespace uvm {

//------------------------------------------------------------------------------
// CLASS: uvm_monitor
//
//! This class should be used as the base class for user-defined monitors.
//!
//! Deriving from uvm_monitor allows you to distinguish monitors from generic
//! component types inheriting from uvm_component.  Such monitors will
//! automatically inherit features that may be added to uvm_monitor in the future.
//!
//! Derived from #uvm_component.
//
//------------------------------------------------------------------------------

class uvm_monitor : public uvm_component
{
 public:
 		
  explicit uvm_monitor( uvm_component_name name_ ) : uvm_component( name_ ) {};
  	
  const std::string get_type_name() const override;

  const char* kind() const override; // SystemC API
};

/////////////////////////////////////////////
/////////////////////////////////////////////

//----------------------------------------------------------------------
// member function: get_type_name()
//
//! Return the type name of the object
//----------------------------------------------------------------------

inline const std::string uvm_monitor::get_type_name() const
{
	return std::string(kind());
}

//----------------------------------------------------------------------
// member function: kind()
//
//! SystemC compatible API
//----------------------------------------------------------------------

inline const char* uvm_monitor::kind() const
{
  return "uvm::uvm_monitor";
}

} /* namespace uvm */

#endif /* UVM_MONITOR_H_ */
