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

#ifndef UVM_SCOREBOARD_H_
#define UVM_SCOREBOARD_H_

#include "uvmsc/base/uvm_component_name.h"
#include "uvmsc/base/uvm_component.h"

namespace uvm {

//------------------------------------------------------------------------------
// CLASS: uvm_scoreboard
//
//! The class #uvm_scoreboard should be used as the base class for
//! user-defined scoreboards.
//
//! Deriving from #uvm_scoreboard will allow you to distinguish scoreboards from
//! other component types inheriting directly from uvm_component. Such
//! scoreboards will automatically inherit and benefit from features that may be
//! added to uvm_scoreboard in the future.
//------------------------------------------------------------------------------

#include <string>

class uvm_scoreboard : public uvm_component
{
 public:
  explicit uvm_scoreboard( uvm_component_name name_ ) : uvm_component( name_ ) {};

  //----------------------------------------------------------------------
  // member function: get_type_name()
  //
  //! Return the type name of the object
  //----------------------------------------------------------------------

  const std::string get_type_name() const override
  {
    return std::string(kind());
  }

  //----------------------------------------------------------------------
  // member function: kind()
  //
  //! SystemC compatible API
  //----------------------------------------------------------------------

  const char* kind() const override // SystemC API
  {
    return "uvm::uvm_scoreboard";
  }

};


/////////////////////////////////////////////
/////////////////////////////////////////////


} /* namespace uvm */

#endif /* UVM_SCOREBOARD_H_ */
