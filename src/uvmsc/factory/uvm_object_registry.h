//----------------------------------------------------------------------
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2012-2019 NXP B.V.
//   Copyright 2007-2010 Mentor Graphics Corporation
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

#ifndef UVM_OBJECT_REGISTRY_H_
#define UVM_OBJECT_REGISTRY_H_

#include <string>
#include <sstream>
#include <map>
#include <systemc>

#include "uvmsc/base/uvm_root.h"
#include "uvmsc/base/uvm_component.h"
#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"
#include "uvmsc/report/uvm_report_object.h"
#include "uvmsc/factory/uvm_factory.h"
#include "uvmsc/factory/uvm_object_wrapper.h"
#include "uvmsc/base/uvm_globals.h"

namespace uvm {

// forward declarations of necessary classes
//none

//----------------------------------------------------------------------
// Factory Object Wrapper
//
// This section defines the proxy object class used by the
// factory. To avoid the overhead of creating an instance of every component
// and object that get registered, the factory holds lightweight wrappers,
// or proxies. When a request for a new object is made, the factory calls upon
// the proxy to create the object it represents. 
//
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class: uvm_object_registry<T,Tname>
//
//! The #uvm_object_registry serves as a lightweight proxy for an #uvm_object of
//! type T and type name Tname, a string. The proxy enables efficient
//! registration with the #uvm_factory. Without it, registration would
//! require an instance of the object itself.
//----------------------------------------------------------------------

template <typename T = uvm_object>
class uvm_object_registry : public uvm_object_wrapper
{
 public:

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  virtual uvm_object* create_object( const std::string& name = "" );

  virtual const std::string get_type_name() const;

  static uvm_object_registry<T>* get();

  static T* create( const std::string& name = "",
                    uvm_component* parent = NULL,
                    const std::string& contxt = "" );

  static void set_type_override( uvm_object_wrapper* override_type,
                                 bool replace = true );

  static void set_inst_override( uvm_object_wrapper* override_type,
                                 const std::string& inst_path,
                                 uvm_component* parent = NULL );

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

  static void destroy( T* obj );

 private:
  explicit uvm_object_registry( const std::string& name = "" );

  virtual ~uvm_object_registry();

  static const std::string m_type_name_prop();

  // data members
  static uvm_object_registry<T>* me;

}; // class uvm_object_registry


//----------------------------------------------------------------------
// definition of static members outside class definition
//----------------------------------------------------------------------

template <typename T>
uvm_object_registry<T>* uvm_object_registry<T>::me = get();


//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------

template <typename T>
uvm_object_registry<T>::uvm_object_registry( const std::string& name )
: uvm_object_wrapper(name)
{}

//----------------------------------------------------------------------
// member function: create_object
//
//! Creates an object of type T and returns it as a handle to an
//! #uvm_object. This is an override of the member function in #uvm_object_wrapper.
//! It is called by the factory after determining the type of object to create.
//! You should not call this method directly. Call create instead.
//----------------------------------------------------------------------

template <typename T>
uvm_object* uvm_object_registry<T>::create_object( const std::string& name )
{
  T* obj = NULL;

  if (name.empty())
    obj = new T(sc_core::sc_gen_unique_name("object"));
  else
    obj = new T(name);

  return obj; // note creation of newed object is tracked in factory
}

//----------------------------------------------------------------------
// member function: get_type_name
//
//! Returns the value given by the string parameter, Tname. This member function
//! overrides the member function in #uvm_object_wrapper.
//----------------------------------------------------------------------

template <typename T>
const std::string uvm_object_registry<T>::get_type_name() const
{
  return uvm_object_registry<T>::m_type_name_prop();
}

//----------------------------------------------------------------------
// member function: get (static)
//
//! Returns the singleton instance of this type. Type-based factory operation
//! depends on there being a single proxy instance for each registered type.
//----------------------------------------------------------------------

template <typename T>
uvm_object_registry<T>* uvm_object_registry<T>::get()
{
  if (me == NULL)
  {
    uvm_coreservice_t* cs = uvm_coreservice_t::get();
    uvm_factory* f = cs->get_factory();

    me = new uvm_object_registry<T>("objrgy_" + m_type_name_prop());
    f->do_register(me);
  }
  return me;
}

//----------------------------------------------------------------------
// member function: create (static)
//
//! Returns an instance of the object type, T, represented by this proxy,
//! subject to any factory overrides based on the context provided by the
//! parent's full name. The \p contxt argument, if supplied, supercedes the
//! parent's context. The new instance will have the given leaf name,
//! if provided.
//----------------------------------------------------------------------

template <typename T>
T* uvm_object_registry<T>::create( const std::string& name,
                                   uvm_component* parent,
                                   const std::string& contxt )
{
  std::string l_contxt;
  uvm_object* obj = NULL;

  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_factory* f = cs->get_factory();

  if (contxt.empty() && parent != NULL)
    l_contxt = parent->get_full_name();
  obj = f->create_object_by_type( get(), l_contxt, name );
  T* robj = dynamic_cast<T*>(obj);
  if (robj == NULL)
  {
    std::ostringstream msg;
    msg << "Factory did not return an object of type '" << m_type_name_prop() << "'."
        << " A object of type '" << ( (obj == NULL) ? "null" : obj->get_type_name() )
        << "' was returned instead. Name=" << name << " Parent="
        << ( (parent == NULL) ? "NULL" : parent->get_type_name() ) << " contxt=" << l_contxt;

    uvm_report_fatal("FCTTYP", msg.str(), UVM_NONE);
  }
  return robj;
}

//----------------------------------------------------------------------
// member function: set_type_override (static)
//
//! Configures the factory to create an object of the type represented by
//! override_type whenever a request is made to create an object of the type
//! represented by this proxy, provided no instance override applies. The
//! original type, T, is typically a super class of the override type.
//----------------------------------------------------------------------

template <typename T>
void uvm_object_registry<T>::set_type_override(
  uvm_object_wrapper* override_type,
  bool replace  )
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_factory* factory = cs->get_factory();

  factory->set_type_override_by_type(get(),override_type,replace);
}

//----------------------------------------------------------------------
// member function: set_inst_override (static)
//
//! Configures the factory to create an object of the type represented by
//! override_type whenever a request is made to create an object of the type
//! represented by this proxy, with matching instance paths. The original
//! type, T, is typically a super class of the override type.
//!
//! If \p parent is not specified, \p inst_path is interpreted as an absolute
//! instance path, which enables instance overrides to be set from outside
//! component classes. If \p parent is specified, \p inst_path is interpreted
//! as being relative to the parent's hierarchical instance path.
//! The inst_path may contain wildcards for matching against multiple contexts.
//----------------------------------------------------------------------

template <typename T>
void uvm_object_registry<T>::set_inst_override(
  uvm_object_wrapper* override_type,
  const std::string& inst_path,
  uvm_component* parent )
{
  std::ostringstream loc_inst_path;
  if (parent != NULL)
  {
    if (inst_path.empty())
      loc_inst_path << parent->get_full_name();
    else
      loc_inst_path << parent->get_full_name() << "." << inst_path;
  }
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_factory* factory = cs->get_factory();

  factory->set_inst_override_by_type(get(), override_type, loc_inst_path.str());
}


////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------
// member function: m_type_name_prop (static)
//
//! Implementation defined
//! Type name propagation to retrieve the string from the original
//! class via the macro M_UVM_COMPONENT_REGISTRY_INTERNAL which
//! defines the string as part of the member function m_register_type_name()
//
// TODO: other ways to pass string to this template class?
//----------------------------------------------------------------------

template <typename T>
const std::string uvm_object_registry<T>::m_type_name_prop()
{
  return T::m_register_type_name();
}

//----------------------------------------------------------------------
// member function: destroy (static)
//
//! Configures the factory to delete a object which is created using 
//! 'create' method.
//----------------------------------------------------------------------

template <typename T>
void uvm_object_registry<T>::destroy( T* obj ) 
{
  if (obj == NULL) 
  {
    return;
  }
  
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_factory* f = cs->get_factory();

  if (!f->m_delete_object(obj))
  {
    std::ostringstream msg;
    msg << "Could not destroy object of type '" << obj->get_type_name()
        << "', name=" << obj->get_name()
        << " from factory";
    uvm_report_error("FCTTYP", msg.str(), UVM_NONE);
    return;
  }
  
  obj = NULL;
}


//----------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------

template <typename T>
uvm_object_registry<T>::~uvm_object_registry()
{
  // clean memory
  if (me != NULL)
  {
    delete me;
    me = NULL;
  }

  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_factory* f = cs->get_factory();
  f->m_delete_all_objects();
}


//////////////

} // namespace uvm

#endif // UVM_OBJECT_REGISTRY_H_
