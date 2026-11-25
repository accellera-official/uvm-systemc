//----------------------------------------------------------------------------
//   Copyright 2012-2013 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
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
//----------------------------------------------------------------------------

#ifndef UVM_OBJECT_WRAPPER_H_
#define UVM_OBJECT_WRAPPER_H_

#include <string>
#include <map>

//////////////////////

namespace uvm {


// forward declarations
class uvm_object;
class uvm_component;

//----------------------------------------------------------------------------
// CLASS: uvm_object_wrapper
//
//! The #uvm_object_wrapper provides an abstract interface for creating object and
//! component proxies. Instances of these lightweight proxies, representing every
//! uvm_object-based and uvm_component-based object available in the test
//! environment, are registered with the uvm_factory. When the factory is
//! called upon to create an object or component, it finds and delegates the
//! request to the appropriate proxy.
//----------------------------------------------------------------------------

class uvm_object_wrapper
{
 public:

  //--------------------------------------------------------------------------
  // member function: create_object
  //
  //! Creates a new object with the optional name.
  //! An object proxy (e.g., uvm_object_registry<T>) implements this
  //! member function to create an object of a specific type, T.
  //--------------------------------------------------------------------------

  virtual uvm_object* create_object( const std::string& name = "" )
  {
    return nullptr;
  }

  //--------------------------------------------------------------------------
  // member function: create_component
  //
  //! Creates a new component, passing to its constructor the given name
  //! and parent. A component proxy (e.g. <uvm_component_registry<T>)
  //! implements this member function to create a component of a specific
  //! type, T.
  //--------------------------------------------------------------------------

  virtual uvm_component* create_component( const std::string& name,
                                           uvm_component* parent)
  {
    return nullptr;
  }

  //--------------------------------------------------------------------------
  // Function: get_type_name
  //
  //! Derived classes implement this method to return the type name of the
  //! object created by create_component or create_object. The factory uses
  //! this name when matching against the requested type in name-based lookups.
  //--------------------------------------------------------------------------

  virtual std::string get_type_name() const = 0;

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

  virtual ~uvm_object_wrapper() {}

 protected:
   uvm_object_wrapper( const std::string& name = "" ) {}

};

} // namespace uvm

//////////////////////

#endif /* UVM_OBJECT_WRAPPER_H_ */
