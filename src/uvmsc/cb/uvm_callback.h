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

#ifndef UVM_CALLBACK_H_
#define UVM_CALLBACK_H_

#include <string>
#include <iostream>

#include "uvmsc/base/uvm_object.h"

namespace uvm {


//------------------------------------------------------------------------------
// CLASS: uvm_callback
//
//! The class #uvm_callback class is the base class for user-defined callback classes.
//! Typically, the component developer defines an application-specific callback
//! class that extends from this class. In it, he defines one or more virtual
//! methods, called a callback interface, that represent the hooks available
//! for user override.
//!
//! Methods intended for optional override should not be declared pure virtual. Usually,
//! all the callback methods are defined with empty implementations so users have
//! the option of overriding any or all of them.
//!
//! The prototypes for each hook method are completely application specific with
//! no restrictions.
//------------------------------------------------------------------------------

class uvm_callback : public uvm_object
{
 public:

  uvm_callback( const std::string& name = "uvm_callback" );

  bool callback_mode( int on = -1 );

  bool is_enabled();

  const std::string get_type_name() const override;

 private:

  // TODO reporter
  //static uvm_report_object reporter = new("cb_tracer");

  static std::string type_name;

 protected:
  bool m_enabled; // default set to true

}; // class uvm_callback

} // namespace uvm

#endif // UVM_CALLBACK_H_
