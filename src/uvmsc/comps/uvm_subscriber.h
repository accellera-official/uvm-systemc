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

#ifndef UVM_SUBSCRIBER_H_
#define UVM_SUBSCRIBER_H_

#include <string>

#include "uvmsc/base/uvm_component.h"
#include "uvmsc/base/uvm_component_name.h"
#include "uvmsc/tlm1/uvm_analysis_export.h"


namespace uvm {

//------------------------------------------------------------------------------
// CLASS: uvm_subscriber
//
//! The class #uvm_subscriber provides an analysis export for receiving
//! transactions from a connected analysis export. Making such a connection
//! subscribes this component to any transactions emitted by the connected
//! analysis port.
//!
//! Subtypes of this class must define the write method to process the incoming
//! transactions. This class is particularly useful when designing a coverage
//! collector that attaches to a monitor.
//------------------------------------------------------------------------------

template <typename T = int>
class uvm_subscriber : public uvm_component,
                       public tlm::tlm_analysis_if<T>
{
 public:

  //----------------------------------------------------------------------
  // Port: analysis_export
  //
  //! This export provides access to the write method, which derived subscribers
  //! must implement.
  //----------------------------------------------------------------------

  uvm_analysis_export<T> analysis_export;

  //----------------------------------------------------------------------
  // constructor
  //
  //! Creates and initializes an instance of this class using the normal
  //! constructor arguments for uvm_component, where name is the name of the
  //! instance
  //----------------------------------------------------------------------

  explicit uvm_subscriber( uvm_component_name name_ )
    : uvm_component( name_ ), analysis_export("analysis_export")
  {
    // bind export to itself, to make use of member function write
    // in this class
    analysis_export(*this);
  };

  //----------------------------------------------------------------------
  // member function: write
  //
  //! A pure virtual method that must be defined in each subclass. Access
  //! to this method by outside components should be done via the
  //! analysis_export.
  //----------------------------------------------------------------------

  void write( const T& t ) override = 0;

  //----------------------------------------------------------------------
  // member function: get_type_name()
  //
  //! Return the type name of the object
  //----------------------------------------------------------------------

  const std::string get_type_name() const override
  {
    return std::string(this->kind());
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

#endif /* UVM_SUBSCRIBER_H_ */
