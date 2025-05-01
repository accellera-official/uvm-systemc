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

#ifndef UVM_TEST_H_
#define UVM_TEST_H_

#include "uvmsc/base/uvm_component.h"
#include "uvmsc/base/uvm_component_name.h"

namespace uvm {

//------------------------------------------------------------------------------
// CLASS: uvm_test
//
//! This class is the virtual base class for the user-defined tests.
//!
//! The uvm_test virtual class should be used as the base class for user-defined
//! tests. Doing so provides the ability to select which test to execute using
//! the UVM_TESTNAME command line or argument to the function uvm_root::run_test.
//!
//! Derived from #uvm_component.
//
//------------------------------------------------------------------------------

#include <string>

class uvm_test : public uvm_component
{
 public:
  explicit uvm_test( uvm_component_name name_ ) : uvm_component( name_ ) {};
  	
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

inline const std::string uvm_test::get_type_name() const
{
	return std::string(kind());
}

//----------------------------------------------------------------------
// member function: kind()
//
//! SystemC compatible API
//----------------------------------------------------------------------

inline const char* uvm_test::kind() const
{
  return "uvm::uvm_test";
}


} /* namespace uvm */

#endif /* UVM_TEST_H_ */
