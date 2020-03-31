//----------------------------------------------------------------------------
//   Copyright 2012-2014 NXP B.V.
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

#ifndef UVM_FACTORY_H_
#define UVM_FACTORY_H_

#include <string>
#include <list>

#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"


//////////////////////

namespace uvm {


// forward class declarations
class uvm_object;
class uvm_component;
class uvm_object_wrapper;
class uvm_factory_override;

//----------------------------------------------------------------------------
// Class: uvm_factory
//
//! As the name implies, uvm_factory is used to manufacture (create) UVM objects
//! and components. Only one instance of the factory is present in a given
//! simulation (termed a singleton). Object and component types are registered
//! with the factory using lightweight proxies to the actual objects and
//! components being created. The #uvm_object_registry<T> and
//! #uvm_component_registry<T> class are used to proxy uvm_objects
//! and uvm_components.
//!
//! The factory provides both name-based and type-based interfaces.
//!
//! type-based - The type-based interface is far less prone to errors in usage.
//!   When errors do occur, they are caught at compile-time.
//!
//! name-based - The name-based interface is dominated
//!   by string arguments that can be misspelled and provided in the wrong order.
//!   Errors in name-based requests might only be caught at the time of the call,
//!   if at all. Further, the name-based interface is not portable across
//!   simulators when used with parameterized classes.
//!
//! The uvm_factory is an abstract class which declares many of its methods
//! as pure virtual. The UVM uses the uvm_default_factory class
//! as its default factory implementation.
//----------------------------------------------------------------------------

class uvm_factory
{
 public:

  static uvm_factory* get()
  {
    uvm_coreservice_t* s;
    s = uvm_coreservice_t::get();
    return s->get_factory();
  }

  //--------------------------------------------------------------------------
  // Group: Registering Types
  //--------------------------------------------------------------------------

  virtual void do_register( uvm_object_wrapper* obj ) = 0;

  //--------------------------------------------------------------------------
  // Group: Type & Instance Overrides
  //--------------------------------------------------------------------------

  virtual void set_inst_override_by_type( uvm_object_wrapper* original_type,
                                          uvm_object_wrapper* override_type,
                                          const std::string& full_inst_path ) = 0;

  virtual void set_inst_override_by_name( const std::string& original_type_name,
                                          const std::string& override_type_name,
                                          const std::string& full_inst_path ) = 0;

  virtual void set_type_override_by_type( uvm_object_wrapper* original_type,
                                          uvm_object_wrapper* override_type,
                                          bool replace = true ) = 0;

  virtual void set_type_override_by_name( const std::string& original_type_name,
                                          const std::string& override_type_name,
                                          bool replace = true ) = 0;

  //--------------------------------------------------------------------------
  // Group: Creation
  //--------------------------------------------------------------------------

  virtual uvm_object* create_object_by_type( uvm_object_wrapper* requested_type,
                                             const std::string& parent_inst_path = "",
                                             const std::string& name = "" ) = 0;

  virtual uvm_component* create_component_by_type( uvm_object_wrapper* requested_type,
                                                   const std::string& parent_inst_path = "",
                                                   const std::string& name = "",
                                                   uvm_component* parent = NULL ) = 0;

  virtual uvm_object* create_object_by_name( const std::string& requested_type_name,
                                             const std::string& parent_inst_path = "",
                                             const std::string& name = "" ) = 0;

  virtual uvm_component* create_component_by_name( const std::string& requested_type_name,
                                                   const std::string& parent_inst_path = "",
                                                   const std::string& name = "",
                                                   uvm_component* parent = NULL ) = 0;

  virtual bool is_type_name_registered( const std::string& type_name ) const = 0;

  virtual bool is_type_registered( uvm_object_wrapper* obj ) const = 0 ;

  //--------------------------------------------------------------------------
  // Group: Debug
  //--------------------------------------------------------------------------

  virtual void debug_create_by_type( uvm_object_wrapper* requested_type,
                                     const std::string& parent_inst_path = "",
                                     const std::string& name = "" ) = 0;

  virtual void debug_create_by_name( const std::string& requested_type_name,
                                     const std::string& parent_inst_path = "",
                                     const std::string& name = "" ) = 0;

  virtual uvm_object_wrapper* find_override_by_type( uvm_object_wrapper* requested_type,
                                                     const std::string& full_inst_path ) = 0;

  virtual uvm_object_wrapper* find_override_by_name( const std::string& requested_type_name,
                                                     const std::string& full_inst_path ) = 0;

  virtual uvm_object_wrapper* find_wrapper_by_name( const std::string& type_name ) = 0;

  virtual void print( int all_types = 1 ) = 0;

  // implementation defined methods

  virtual bool m_delete_object( uvm_object* obj ) = 0;
  virtual void m_delete_all_objects() = 0;

  virtual bool m_delete_component( uvm_component* comp ) = 0;
  virtual void m_delete_all_components() = 0;

 protected:
  uvm_factory(){};
  virtual ~uvm_factory(){};

  typedef std::list<uvm_object* > m_obj_listT;
  typedef m_obj_listT::iterator m_obj_listItT;
  m_obj_listT m_obj_list;

  typedef std::list<uvm_component* > m_comp_listT;
  typedef m_comp_listT::iterator m_comp_listItT;
  m_comp_listT m_comp_list;
}; // class uvm_factory

} // namespace uvm

//////////////////////

#endif /* UVM_FACTORY_H_ */
