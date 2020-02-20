//----------------------------------------------------------------------
//   Copyright 2013-2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2012-2017 NXP B.V.
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
//----------------------------------------------------------------------

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <list>
#include <vector>

#include "uvmsc/base/uvm_globals.h"
#include "uvmsc/base/uvm_component.h"
#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"
#include "uvmsc/factory/uvm_default_factory.h"
#include "uvmsc/factory/uvm_factory.h"
#include "uvmsc/factory/uvm_factory_override.h"
#include "uvmsc/factory/uvm_object_wrapper.h"
#include "uvmsc/macros/uvm_message_defines.h"
#include "uvmsc/macros/uvm_string_defines.h"
#include "uvmsc/misc/uvm_misc.h"


namespace uvm {

//----------------------------------------------------------------------------
// uvm_default_factory implementation
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// initialization static member variables
//----------------------------------------------------------------------------

bool uvm_default_factory::m_debug_pass = false;

//----------------------------------------------------------------------------
// constructor uvm_default_factory
//----------------------------------------------------------------------------

uvm_default_factory::uvm_default_factory()
{
  m_types.clear();
  m_type_names.clear();
  m_lookup_strs.clear();
  m_type_overrides.clear();
  m_wildcard_inst_overrides.clear();

  m_inst_override_queues.clear();
  m_inst_override_name_queues.clear();
  m_override_info.clear();
}

//----------------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------------

uvm_default_factory::~uvm_default_factory()
{
  for( m_overrides_listItT
       it = m_type_overrides.begin();
       it != m_type_overrides.end();
       it++ )
    delete *it; // delete uvm_factory_override objects

  for( m_overrides_listItT
       it = m_wildcard_inst_overrides.begin();
       it != m_wildcard_inst_overrides.end();
       it++ )
    delete *it; // delete uvm_factory_override objects

  for( m_overrides_listItT
       it = m_override_info.begin();
       it != m_override_info.end();
       it++ )
    delete *it; // delete uvm_factory_override objects

  for( m_types_mapItT
       it = m_types.begin();
       it != m_types.end();
       it++ )
    delete it->first; // delete uvm_object_wrapper objects

  for( m_type_names_mapItT
       it = m_type_names.begin();
       it != m_type_names.end();
       it++ )
    delete it->second; // delete uvm_object_wrapper objects

  for( m_inst_override_queues_mapItT
       it = m_inst_override_queues.begin();
       it != m_inst_override_queues.end();
       it++ )
  {

    for (m_overrides_listItT
         itq = it->second->queue.begin();
         itq != it->second->queue.end();
         itq++)
      delete *itq; // delete uvm_factory_override objects

    delete it->first;  // delete uvm_object_wrapper objects
    delete it->second; // delete uvm_factory_queue_class objects
  }

  for( m_inst_override_name_queues_mapItT
       it = m_inst_override_name_queues.begin();
       it != m_inst_override_name_queues.end();
       it++ )
  {
    for (m_overrides_listItT
         itq = it->second->queue.begin();
         itq != it->second->queue.end();
         itq++)
      delete *itq; // delete uvm_factory_override objects

    delete it->second; // delete uvm_factory_queue_class objects
  }

  for( m_comp_t_listItT 
       it = m_comp_t_list.begin();
       it != m_comp_t_list.end(); 
       it++)
    delete (*it); // delete created uvm_component
  
  for( m_obj_t_listItT 
       it = m_obj_t_list.begin();
       it != m_obj_t_list.end(); 
       it++)
    delete (*it); // delete created uvm_object
}

//----------------------------------------------------------------------------
// member function: do_register
//
//! Registers the given proxy object, \p obj, with the factory. The proxy object
//! is a lightweight substitute for the component or object it represents. When
//! the factory needs to create an object of a given type, it calls the proxy's
//! #create_object or #create_component method to do so.
//!
//! When doing name-based operations, the factory calls the proxy's
//! get_type_name method to match against the \p requested_type_name argument in
//! subsequent calls to #create_component_by_name and #create_object_by_name.
//! If the proxy object's #get_type_name method returns the empty string,
//! name-based lookup is effectively disabled.
//----------------------------------------------------------------------------

void uvm_default_factory::do_register( uvm_object_wrapper* obj )
{
  if (obj == NULL)
    uvm_report_fatal("NULLWR", "Attempting to register a NULL object with the factory", UVM_NONE);

  if (!obj->get_type_name().empty() && obj->get_type_name() != "<unknown>")
  {
    if (m_type_names.find(obj->get_type_name()) != m_type_names.end()) // if obj with name exists
    {
      std::ostringstream msg;
      msg << "Type name '" << obj->get_type_name()
          << "' already registered with factory. No string-based lookup "
          << "support for multiple types with the same type name.";
      uvm_report_warning("TPRGED", msg.str(), UVM_NONE);
    }
    else
    {
      m_type_names[obj->get_type_name()] = obj;
    }
  }

  if (m_types.find(obj) != m_types.end()) // if obj exists
  {
    if (!obj->get_type_name().empty() && obj->get_type_name() != "<unknown>")
    {
      std::ostringstream msg;
      msg << "Object type '" << obj->get_type_name()
          << "' already registered with factory. ";
      uvm_report_warning("TPRGED", msg.str(), UVM_NONE);
    }
  }
  else // new object (not in list)
  {
    m_types[obj] = true;

    // If a named override happens before the type is registered, need to copy
    // the override queue.
    // Note: Registration occurs via static initialization, which occurs ahead of
    // procedural (e.g. initial) blocks. There should not be any preexisting overrides.
    if( m_inst_override_name_queues.find(obj->get_type_name()) != m_inst_override_name_queues.end() ) //if exists
    {
       m_inst_override_queues[obj] = new uvm_factory_queue_class();
       m_inst_override_queues[obj]->queue = m_inst_override_name_queues[obj->get_type_name()]->queue;
       m_inst_override_name_queues.erase(obj->get_type_name());
    }

    if( m_wildcard_inst_overrides.size() )
    {
       if( m_inst_override_queues.find(obj) == m_inst_override_queues.end() ) // not found
            m_inst_override_queues[obj] = new uvm_factory_queue_class();
       for( m_overrides_listItT
            it = m_wildcard_inst_overrides.begin();
            it != m_wildcard_inst_overrides.end();
            ++it )
       {
         if(uvm_is_match( (*it)->orig_type_name, obj->get_type_name()))
            m_inst_override_queues[obj]->queue.push_back( *it );
       }
    }
  }
}

//----------------------------------------------------------------------------
// member function: set_type_override_by_type
//
//! Configures the factory to create an object of the override's type whenever
//! a request is made to create an object of the original type using a context
//! that matches \p full_inst_path. The original type is typically a super class
//! of the override type.
//!
//! When overriding by type, the \p original_type and \p override_type are
//! handles to the types' proxy objects. Preregistration is not required.
//----------------------------------------------------------------------------

void uvm_default_factory::set_type_override_by_type( uvm_object_wrapper* original_type,
                                             uvm_object_wrapper* override_type,
                                             bool replace )
{
  bool replaced = false;

  // check that old and new are not the same
  if (original_type == override_type)
  {
    if (original_type->get_type_name().empty() || original_type->get_type_name() == "<unknown>")
      uvm_report_warning("TYPDUP", "Original and override type arguments are identical", UVM_NONE);
    else
    {
      std::ostringstream msg;
      msg << "Original and override type arguments are identical: "
          << original_type->get_type_name() << ".";
      uvm_report_warning("TYPDUP", msg.str(), UVM_NONE);
    }
    return;
  }

  // register the types if not already done so, for the benefit of string-based lookup
  if ( m_types.find(original_type) == m_types.end() )
    do_register(original_type);

  if ( m_types.find(original_type) == m_types.end() )
    do_register(override_type);

  // check for existing type override
  for( m_overrides_listItT
       it = m_type_overrides.begin();
       it != m_type_overrides.end();
       it++ )
  {
    if ( (*it)->orig_type == original_type ||
        ( (*it)->orig_type_name != "<unknown>" &&
          !(*it)->orig_type_name.empty() &&
          (*it)->orig_type_name == original_type->get_type_name()))
    {
      std::ostringstream msg;
      msg << "Original object type '" << original_type->get_type_name()
          << "' already registered to produce '"
          << (*it)->ovrd_type_name << "'." << std::endl;

      if (!replace)
      {
        msg << "Set 'replace' argument to replace the existing entry.";
        uvm_report_info("TPREGD", msg.str(), UVM_MEDIUM);
        return;
      }
      msg << "Replacing with override to produce type '" << override_type->get_type_name()
          << "'.";

      uvm_report_info("TPREGR", msg.str(), UVM_MEDIUM);
      replaced = true;
      (*it)->orig_type = original_type;
      (*it)->orig_type_name = original_type->get_type_name();
      (*it)->ovrd_type = override_type;
      (*it)->ovrd_type_name = override_type->get_type_name();
    }
  }

  // make a new entry
  if (!replaced) {
    uvm_factory_override* override;
    override = new uvm_factory_override( "*",
                                         original_type->get_type_name(),
                                         original_type,
                                         override_type );
    m_type_overrides.push_back(override);
  }
}


//----------------------------------------------------------------------------
// member function: set_type_override_by_name
//
//! Configures the factory to create an object of the override’s type whenever
//! a request is made to create an object of the original type, provided no
//! instance override applies. The original type is typically a super class of
//! the override type.
//!
//! When overriding by name, the \p original_type_name typically refers to a
//! preregistered type in the factory. It may, however, be any arbitrary string.
//! Future calls to any of the create_* methods with the same string and
//! matching instance path will produce the type represented by
//! override_type_name, which must be preregistered with the factory.
//! When replace is set to true, a previous override on original_type_name is
//! replaced, otherwise a previous override, if any, remains intact.
//----------------------------------------------------------------------------

void uvm_default_factory::set_type_override_by_name( const std::string& original_type_name,
                                             const std::string& override_type_name,
                                             bool replace )
{
  bool replaced = false;
  uvm_object_wrapper* original_type = NULL;
  uvm_object_wrapper* override_type = NULL;

  if( m_type_names.find(original_type_name) != m_type_names.end() ) // if exists
    original_type = m_type_names[original_type_name];

  if( m_type_names.find(override_type_name) != m_type_names.end() ) // if exists
    override_type = m_type_names[override_type_name];

  // check that type is registered with the factory
  if (override_type == NULL)
  {
    std::ostringstream msg;
    msg << "Cannot register override for original type '" <<  original_type_name
        << "' because the override type '" << override_type_name
        << "' is not registered with the factory.";
    uvm_report_error("TYPNTF", msg.str(), UVM_NONE);
    return;
  }

  // check that old and new are not the same
  if ( original_type_name == override_type_name )
  {
    std::ostringstream msg;
    msg << "Requested and actual type name arguments are identical: "
        << original_type_name << ". Ignoring this override.";
    uvm_report_warning("TYPDUP", msg.str(), UVM_NONE);
    return;
  }

  for ( m_overrides_listItT
        it = m_type_overrides.begin();
        it != m_type_overrides.end();
        it++ )
  {
    if ( (*it)->orig_type_name == original_type_name)
    {
      if (!replace)
      {
        std::ostringstream msg;
        msg << "Original type '" << original_type_name
            << "' already registered to produce '" << (*it)->ovrd_type_name
            << "'.  Set 'replace' argument to replace the existing entry.";
        uvm_report_info("TPREGD", msg.str(), UVM_MEDIUM);
        return;
      }
        std::ostringstream msg;
        msg << "Original object type '" << original_type_name
            << "' already registered to produce '" << (*it)->ovrd_type_name
            << "'.  Replacing with override to produce type '"
            << override_type_name << "'.";
        uvm_report_info("TPREGR", msg.str() , UVM_MEDIUM);

      replaced = true;
      (*it)->ovrd_type = override_type;
      (*it)->ovrd_type_name = override_type_name;
    } // if
  } // for

  if (original_type == NULL)
    m_lookup_strs[original_type_name] = true;

  if (!replaced)
  {
    uvm_factory_override* override = NULL;
    override = new uvm_factory_override( "*",
                                         original_type_name,
                                         original_type,
                                         override_type );

    m_type_overrides.push_back(override);

    // MB: also commented out in original SV code
    // m_type_names[original_type_name] = override.ovrd_type;
  }
}



//----------------------------------------------------------------------------
// member function: set_inst_override_by_type
//
//! Configures the factory to create an object of the override's type whenever
//! a request is made to create an object of the original type using a context
//! that matches \p full_inst_path. The original type is typically a super class
//! of the override type.
//!
//! When overriding by type, the \p original_type and \p override_type are
//! handles to the types' proxy objects. Preregistration is not required.
//!
//! The \p full_inst_path is matched against the contentation of
//! '\p parent_inst_path + "." + \p name' provided in future create requests. The
//! \p full_inst_path may include wildcards (* and ?) such that a single
//! instance override can be applied in multiple contexts. A \p full_inst_path
//! of "*" is effectively a type override, as it will match all contexts.
//!
//! When the factory processes instance overrides, the instance queue is
//! processed in order of override registrations, and the first override
//! match prevails. Thus, more specific overrides should be registered
//! first, followed by more general overrides.
//----------------------------------------------------------------------------

void uvm_default_factory::set_inst_override_by_type( uvm_object_wrapper* original_type,
                                             uvm_object_wrapper* override_type,
                                             const std::string& full_inst_path )
{
  uvm_factory_override* override;

  // register the types if not already done so
  if ( m_types.find(original_type) == m_types.end() )
    do_register(original_type);

  if ( m_types.find(original_type) == m_types.end() )
    do_register(override_type);

  if ( check_inst_override_exists(original_type,override_type,full_inst_path) )
    return;

  if( m_inst_override_queues.find(original_type) == m_inst_override_queues.end() )
    m_inst_override_queues[original_type] = new uvm_factory_queue_class();

  override = new uvm_factory_override( full_inst_path,
                                       original_type->get_type_name(),
                                       original_type,
                                       override_type);

  m_inst_override_queues[original_type]->queue.push_back(override);
}


//----------------------------------------------------------------------------
// member function: set_inst_override_by_name
//
//! Configures the factory to create an object of the override's type whenever
//! a request is made to create an object of the original type using a context
//! that matches \p full_inst_path. The original type is typically a super class
//! of the override type.
//!
//! When overriding by name, the \p original_type_name typically refers to a
//! preregistered type in the factory. It may, however, be any arbitrary
//! string. Future calls to any of the create_* methods with the same string
//! and matching instance path will produce the type represented by
//! \p override_type_name, which must be preregistered with the factory.
//
//! The \p full_inst_path is matched against the contentation of
//! '\p parent_inst_path + "." + \p name' provided in future create requests. The
//! \p full_inst_path may include wildcards (* and ?) such that a single
//! instance override can be applied in multiple contexts. A \p full_inst_path
//! of "*" is effectively a type override, as it will match all contexts.
//
//! When the factory processes instance overrides, the instance queue is
//! processed in order of override registrations, and the first override
//! match prevails. Thus, more specific overrides should be registered
//! first, followed by more general overrides.
//----------------------------------------------------------------------------

void uvm_default_factory::set_inst_override_by_name( const std::string& original_type_name,
                                             const std::string& override_type_name,
                                             const std::string& full_inst_path )
{
  uvm_object_wrapper* original_type = NULL;
  uvm_object_wrapper* override_type = NULL;
  uvm_factory_override* override = NULL;

  if( m_type_names.find(original_type_name) != m_type_names.end() ) // if exists
    original_type = m_type_names[original_type_name];

  if( m_type_names.find(override_type_name) != m_type_names.end() ) // if exists
    override_type = m_type_names[override_type_name];

  // check that type is registered with the factory
  if (override_type == NULL)
  {
    std::ostringstream msg;
    msg << "Cannot register instance override with type name '"
        << original_type_name << "' and instance path '"
        << full_inst_path << "' because the type it's supposed to produce, '"
        << override_type_name << "', is not registered with the factory.";
    uvm_report_error("TYPNTF", msg.str(), UVM_NONE);
    return;
  }

  if (original_type == NULL)
      m_lookup_strs[original_type_name] = true;

  override = new uvm_factory_override( full_inst_path,
                                       original_type_name,
                                       original_type,
                                       override_type);

  if (original_type != NULL)
  {
    if ( check_inst_override_exists(original_type, override_type, full_inst_path))
      return;

    if( m_inst_override_queues.find(original_type) == m_inst_override_queues.end() )
      m_inst_override_queues[original_type] = new uvm_factory_queue_class();
    m_inst_override_queues[original_type]->queue.push_back(override);
  }
  else
  {
    if (m_has_wildcard(original_type_name))
    {
      for( m_type_names_mapItT
           it = m_type_names.begin();
           it != m_type_names.end();
           it++ )
      {
        std::string s = (it)->first;
        if( uvm_is_match( original_type_name, s ))
          this->set_inst_override_by_name(s, override_type_name, full_inst_path);
      }
      m_wildcard_inst_overrides.push_back(override);
    }
    else
    {
      if( m_inst_override_name_queues.find(original_type_name) == m_inst_override_name_queues.end() )
        m_inst_override_name_queues[original_type_name] = new uvm_factory_queue_class();
      m_inst_override_name_queues[original_type_name]->queue.push_back(override);
    }
  }
}

//----------------------------------------------------------------------------
// Group: Creation
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// member function: create_object_by_type
//----------------------------------------------------------------------------

uvm_object* uvm_default_factory::create_object_by_type( uvm_object_wrapper* requested_type,
                                                const std::string& parent_inst_path,
                                                const std::string& name )
{
  std::string full_inst_path;

  if (parent_inst_path.empty())
    full_inst_path = name;
  else if (!name.empty())
    full_inst_path = parent_inst_path + "." + name;
  else
    full_inst_path = parent_inst_path;

  m_override_info.clear();

  requested_type = find_override_by_type(requested_type, full_inst_path);

  uvm_object* obj = requested_type->create_object(name);
  m_obj_t_list.push_back(obj);

  return obj;
}

//----------------------------------------------------------------------------
// member function: create_component_by_type
//----------------------------------------------------------------------------

uvm_component* uvm_default_factory::create_component_by_type( uvm_object_wrapper* requested_type,
                                                      const std::string& parent_inst_path,
                                                      const std::string& name,
                                                      uvm_component* parent )
{
  std::string full_inst_path;

  if (parent_inst_path.empty())
    full_inst_path = name;
  else if (!name.empty())
    full_inst_path = parent_inst_path + "." + name;
  else
    full_inst_path = parent_inst_path;

  m_override_info.clear();

  requested_type = find_override_by_type(requested_type, full_inst_path);
  
  uvm_component* comp = requested_type->create_component(name, parent);
  m_comp_t_list.push_back(comp);

  return comp; 
}

//----------------------------------------------------------------------------
// member function: create_object_by_name
//----------------------------------------------------------------------------

uvm_object* uvm_default_factory::create_object_by_name( const std::string& requested_type_name,
                                                const std::string& parent_inst_path,
                                                const std::string& name )
{
  uvm_object_wrapper* wrapper;
  std::string inst_path;

  if (parent_inst_path.empty())
    inst_path = name;
  else if (!name.empty())
    inst_path = parent_inst_path + "." + name;
  else
    inst_path = parent_inst_path;

  m_override_info.clear();

  wrapper = find_override_by_name(requested_type_name, inst_path);

  // if no override exists, try to use requested_type_name directly
  if (wrapper == NULL)
  {
    if(m_type_names.find(requested_type_name) == m_type_names.end())
    {
      std::ostringstream msg;
      msg << "Cannot create an object of type '" << requested_type_name
          << "' because it is not registered with the factory.";
      uvm_report_warning("BDTYP", msg.str(), UVM_NONE);
      return NULL;
    }
    wrapper = m_type_names[requested_type_name];
  }

  uvm_object* obj = wrapper->create_object(name);
  m_obj_t_list.push_back(obj);

  return obj;
}

//----------------------------------------------------------------------------
// member function: create_component_by_name
//----------------------------------------------------------------------------

uvm_component* uvm_default_factory::create_component_by_name( const std::string& requested_type_name,
                                                      const std::string& parent_inst_path,
                                                      const std::string& name,
                                                      uvm_component* parent )
{
  uvm_object_wrapper* wrapper;

  std::string inst_path;

  if (parent_inst_path.empty())
    inst_path = name;
  else if (!name.empty())
    inst_path = parent_inst_path + "." + name;
  else
    inst_path = parent_inst_path;

  m_override_info.clear();

  wrapper = find_override_by_name(requested_type_name, inst_path);

  // if no override exists, try to use requested_type_name directly
  if (wrapper == NULL)
  {
    if( m_type_names.find(requested_type_name) == m_type_names.end() ) // not exist
    {
      std::ostringstream msg;
      msg << "Cannot create a component of type '" << requested_type_name
          << "' because it is not registered with the factory.";
      uvm_report_warning("BDTYP", msg.str(), UVM_NONE);
      return NULL;
    }
    wrapper = m_type_names[requested_type_name];
  }
  uvm_component* comp = wrapper->create_component(name, parent);
  m_comp_t_list.push_back(comp);

  return comp;
}

//----------------------------------------------------------------------------
// member function: is_type_name_registered
//
//! Returns true if the object with type type_name can be found in the factory.
//! Otherwise it returns false.
//----------------------------------------------------------------------------

bool uvm_default_factory::is_type_name_registered( const std::string& type_name ) const
{
  return(m_type_names.find(type_name) != m_type_names.end()); // exists
}

//----------------------------------------------------------------------------
// member function: is_type_registered
//
//! Returns true if the object is registered in the factory.
//! Otherwise it returns false.
//----------------------------------------------------------------------------

bool uvm_default_factory::is_type_registered( uvm_object_wrapper* obj ) const
{
  return(m_types.find(obj) != m_types.end()); // exists
}


//----------------------------------------------------------------------------
// Group: Debug
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// member function: debug_create_by_type
//
//! The member function #debug_create_by_type shall perform the same search
//! algorithm as the #create_object_by_type method, but it does not create a new
//! object. Instead, it will provide detailed information about what type of
//! object it would return, listing each override that was applied to arrive at
//! the result. Interpretation of the arguments is the same as with the
//! member function #create_object_by_type.
//----------------------------------------------------------------------------

void uvm_default_factory::debug_create_by_type( uvm_object_wrapper* requested_type,
                                        const std::string& parent_inst_path,
                                        const std::string& name )
{
  m_debug_create("", requested_type, parent_inst_path, name);
}

//----------------------------------------------------------------------------
// member function: debug_create_by_name
//
//! The member function #debug_create_by_name shall perform the same search
//! algorithm as the #create_object_by_name method, but it does not create a new
//! object. Instead, it will provide detailed information about what type of
//! object it would return, listing each override that was applied to arrive at
//! the result. Interpretation of the arguments is the same as with the
//! member function #create_object_by_name.
//----------------------------------------------------------------------------

void uvm_default_factory::debug_create_by_name( const std::string& requested_type_name,
                                        const std::string& parent_inst_path,
                                        const std::string& name )
{
  m_debug_create(requested_type_name, NULL, parent_inst_path, name);
}

//----------------------------------------------------------------------------
// member function: find_override_by_name
//----------------------------------------------------------------------------

uvm_object_wrapper* uvm_default_factory::find_override_by_name( const std::string& requested_type_name,
                                                        const std::string& full_inst_path)
{
  uvm_object_wrapper* rtype = NULL;
  uvm_factory_queue_class* qc = NULL;
  uvm_object_wrapper* override = NULL;

  if ( m_type_names.find(requested_type_name) != m_type_names.end() )
    rtype = m_type_names[requested_type_name];

/*** TODO - was already commented out in UVM-SV implementation
  if(rtype == null) begin
    if(!requested_type_name.empty()) begin
      uvm_report_warning("TYPNTF", {"Requested type name ",
         requested_type_name, " is not registered with the factory. The instance override to ",
         full_inst_path, " is ignored"}, UVM_NONE);
    end
    m_lookup_strs[requested_type_name] = 1;
    return null;
  end
***/

  if (!full_inst_path.empty())
  {
    if(rtype == NULL)
    {
      if( m_inst_override_name_queues.find(requested_type_name) !=
          m_inst_override_name_queues.end() ) // if exists
        qc = m_inst_override_name_queues[requested_type_name];
    }
    else
    {
      if( m_inst_override_queues.find(rtype) != m_inst_override_queues.end() ) // if exists
        qc = m_inst_override_queues[rtype];
    }
    if (qc != NULL)
    {
      for( m_overrides_listItT
           qit = qc->queue.begin();
           qit != qc->queue.end();
           ++qit )
      {
        if ( uvm_is_match( (*qit)->orig_type_name, requested_type_name) &&
             uvm_is_match( (*qit)->full_inst_path, full_inst_path) )
        {
          m_override_info.push_back(*qit);

          if (m_debug_pass)
          {
            if (override == NULL)
            {
              override = (*qit)->ovrd_type;
              (*qit)->selected = true;
            }
          }
          else
          {
            if ( (*qit)->ovrd_type->get_type_name() == requested_type_name)
              return (*qit)->ovrd_type;
            else
              return find_override_by_type( (*qit)->ovrd_type,full_inst_path);
          }
        }
      }
    }
  }

  if( rtype != NULL &&
      (m_inst_override_queues.find(rtype) == m_inst_override_queues.end()) &&
      m_wildcard_inst_overrides.size() )
  {
    m_inst_override_queues[rtype] = new uvm_factory_queue_class();
    for( m_overrides_listItT
         it = m_wildcard_inst_overrides.begin();
         it != m_wildcard_inst_overrides.end();
         ++it )
    {
      if(uvm_is_match( (*it)->orig_type_name, requested_type_name))
        m_inst_override_queues[rtype]->queue.push_back(*it);
    }
  }

  // type override - exact match
  for ( m_overrides_listItT
        it = m_type_overrides.begin();
        it != m_type_overrides.end();
        it++ )
    if ( (*it)->orig_type_name == requested_type_name)
    {
      m_override_info.push_back(*it);

      if (m_debug_pass)
      {
        if (override == NULL)
        {
          override = (*it)->ovrd_type;
          (*it)->selected = true;
        }
      }
      else
        return find_override_by_type( (*it)->ovrd_type, full_inst_path);
    }

  if ( m_debug_pass && override != NULL )
    return find_override_by_type(override, full_inst_path);

  // No override found
  return NULL;
}

//----------------------------------------------------------------------------
// member function: find_override_by_type
//----------------------------------------------------------------------------

uvm_object_wrapper* uvm_default_factory::find_override_by_type( uvm_object_wrapper* requested_type,
                                                        const std::string& full_inst_path)
{
  uvm_object_wrapper* override = NULL;
  uvm_factory_queue_class* qc = NULL;

  if ( m_inst_override_queues.find(requested_type) != m_inst_override_queues.end() )
    qc = m_inst_override_queues[requested_type];

  for( m_overrides_listItT
       it = m_override_info.begin();
       it != m_override_info.end();
       it++ )
  {
    if ( //index != m_override_info.size()-1 &&  //TODO was already commented out in SV
       (*it)->orig_type == requested_type)
    {
      uvm_report_error("OVRDLOOP", "Recursive loop detected while finding override.", UVM_NONE);

      if (!m_debug_pass)
        debug_create_by_type( requested_type, full_inst_path );

      return requested_type;
    }
  }

  // inst override; return first match; takes precedence over type overrides
  if ( !full_inst_path.empty() && qc != NULL )
  {
    for( m_overrides_listItT
         it = qc->queue.begin();
         it != qc->queue.end();
         it++)
    {
      if ( ( (*it)->orig_type == requested_type ||
           ( (*it)->orig_type_name != "<unknown>" &&
             !(*it)->orig_type_name.empty() &&
             (*it)->orig_type_name == requested_type->get_type_name())) &&
             uvm_is_match( (*it)->full_inst_path, full_inst_path)
             )
      {
        m_override_info.push_back(*it);

        if (m_debug_pass) {
          if (override == NULL) {
            override = (*it)->ovrd_type;
            (*it)->selected = true;
          }
        }
        else {
          if ( (*it)->ovrd_type == requested_type)
            return requested_type;
          else
            return find_override_by_type( (*it)->ovrd_type, full_inst_path);
        }
      }
    }
  }

  // type override - exact match
  for( m_overrides_listItT
      it = m_type_overrides.begin();
      it != m_type_overrides.end();
      it++ )
  {
    if ( (*it)->orig_type == requested_type ||
      ( (*it)->orig_type_name != "<unknown>" &&
        !(*it)->orig_type_name.empty() &&
        requested_type != NULL &&
        (*it)->orig_type_name == requested_type->get_type_name()) )
    {
      m_override_info.push_back(*it);

      if (m_debug_pass) {
        if (override == NULL) {
          override = (*it)->ovrd_type;
          (*it)->selected = true;
        }
      }
      else {
        if ((*it)->ovrd_type == requested_type)
          return requested_type;
        else
          return find_override_by_type((*it)->ovrd_type,full_inst_path);
      }
    }
  }


  // type override with wildcard match -- TODO: Already commented-out in SV code
  //foreach (m_type_overrides[index])
  //  if (uvm_is_match(index,requested_type.get_type_name())) begin
  //    m_override_info.push_back(m_inst_overrides[index]);
  //    return find_override_by_type(m_type_overrides[index],full_inst_path);
  //  end

  if ( m_debug_pass && override != NULL )
  {
    if (override == requested_type)
      return requested_type;
    else
      return find_override_by_type(override, full_inst_path);
  }

  return requested_type;
}

//----------------------------------------------------------------------------
// member function: find_wrapper_by_name
//
//! This member function returns the uvm_object_wrapper associated with a given
//! type_name. If not found, it returns NULL.
//----------------------------------------------------------------------------

uvm_object_wrapper* uvm_default_factory::find_wrapper_by_name( const std::string& type_name )
{
  if (m_type_names.find(type_name) != m_type_names.end()) // exists
    return m_type_names[type_name];

  uvm_report_warning("UnknownTypeName", "find_wrapper_by_name: Type name '" + type_name +
      "' not registered with the factory.", UVM_NONE);
  return NULL;
}

//----------------------------------------------------------------------------
// member function: print
//
//! Prints the state of the uvm_factory, including registered types, instance
//! overrides, and type overrides.
//!
//! When all_types is 0, only type and instance overrides are displayed.
//! When all_types is 1 (default), all registered user-defined types are
//! printed as well, provided they have names associated with them. When
//! all_types is 2, the UVM types (prefixed with uvm_) are included in the
//! list of registered types.
//----------------------------------------------------------------------------

void uvm_default_factory::print( int all_types )
{
  std::string key;
  std::map<std::string, uvm_factory_queue_class*> sorted_override_queues;
  std::string tmp;
  std::vector<std::string> qs;
  int id=0;
  uvm_object_wrapper* obj = NULL;

  //sort the override queues
  for ( std::map<uvm_object_wrapper*, uvm_factory_queue_class*>::iterator
        it = m_inst_override_queues.begin();
        it != m_inst_override_queues.end();
        it++ )
  {
    obj = (*it).first;
    tmp = obj->get_type_name();
    if (tmp.empty())
    {
      std::ostringstream str;
      str << "__unnamed_id_" << id++;
      tmp = str.str();
    }
    sorted_override_queues[tmp] = (*it).second;
  }

  for( std::map<std::string, uvm_factory_queue_class*>::iterator
       it = m_inst_override_name_queues.begin();
       it != m_inst_override_name_queues.end();
       it++ )
  {
    sorted_override_queues[(*it).first] = m_inst_override_name_queues[(*it).first];
  }

  qs.push_back("\n#### Print Factory Configuration (*)\n\n");

  // print instance overrides
  if( !m_type_overrides.size() && !sorted_override_queues.size() )
    qs.push_back("No instance or type overrides are registered with this factory\n");
  else
  {
    unsigned int max1 = 14;
    unsigned int max2 = 13;
    unsigned int max3 = 13;
    std::string dash = "---------------------------------------------------------------------------------------------------";
    std::string space= "                                                                                                   ";

    // print instance overrides
    if(sorted_override_queues.size() == 0)
      qs.push_back("No instance overrides are registered with this factory\n");
    else
    {
      for( m_inst_override_name_queues_mapItT
           it = sorted_override_queues.begin();
           it != sorted_override_queues.end();
           it++)
      {
        uvm_factory_queue_class* qc = (*it).second;
        for ( m_overrides_listItT
              lit = qc->queue.begin();
              lit != qc->queue.end();
              lit++)
        {
          if ((*lit)->orig_type_name.length() > max1)
            max1 = (*lit)->orig_type_name.length();
          if ((*lit)->full_inst_path.length() > max2)
            max2=(*lit)->full_inst_path.length();
          if ((*lit)->ovrd_type_name.length() > max3)
            max3=(*lit)->ovrd_type_name.length();
        }
      }
      if (max1 < 14) max1 = 14;
      if (max2 < 13) max2 = 13;
      if (max3 < 13) max3 = 13;

      qs.push_back("Instance Overrides:\n\n");
      qs.push_back("Requested Type" + space.substr(1,max1-14) +
            "  Override Path" + space.substr(1,max2-13) +
            "  Override Type" + space.substr(1,max3-13) + "\n");

      qs.push_back("  " + dash.substr(1,max1) +
           "  " +  dash.substr(1,max2) +
           "  " + dash.substr(1,max3) + "\n" );

      for( m_inst_override_name_queues_mapItT
           it = sorted_override_queues.begin();
           it != sorted_override_queues.end();
           it++)
      {
        uvm_factory_queue_class* qc = (*it).second;
        for ( m_overrides_listItT
              lit = qc->queue.begin();
              lit != qc->queue.end();
              lit++)
        {
          qs.push_back("  " + (*lit)->orig_type_name +
               space.substr(1, max1 - (*lit)->orig_type_name.length()) +
               "  " + (*lit)->full_inst_path +
               space.substr(1, max2 - (*lit)->full_inst_path.length()) +
               "  " + (*lit)->ovrd_type_name + "\n");

        }
      }
    }

    // print type overrides
    if ( m_type_overrides.size() == 0 )
      qs.push_back("\nNo type overrides are registered with this factory\n");
    else
    {
      // Resize for type overrides
      if (max1 < 14) max1 = 14;
      if (max2 < 13) max2 = 13;
      if (max3 < 13) max3 = 13;

      for( m_overrides_listItT
           it = m_type_overrides.begin();
           it != m_type_overrides.end();
           it++ )
      {
        if ((*it)->orig_type_name.length() > max1)
          max1 = (*it)->orig_type_name.length();
        if ((*it)->ovrd_type_name.length() > max2)
          max2=(*it)->ovrd_type_name.length();
      }
      if (max1 < 14) max1 = 14;
      if (max2 < 13) max2 = 13;

      qs.push_back("Type Overrides:\n\n");
      qs.push_back("  Requested Type" + space.substr(1,max1-14) +
           "  Override Type" + space.substr(1,max2-13) + "\n");

      qs.push_back("  " + dash.substr(1,max1) +
           "  " + dash.substr(1,max2) + "\n");

      for( m_overrides_listItT
                 it = m_type_overrides.begin();
                 it != m_type_overrides.end();
                 it++ )
      {
        qs.push_back("  " + (*it)->orig_type_name +
             space.substr(1, max1 - (*it)->orig_type_name.length()) +
             "  " + (*it)->ovrd_type_name + "\n");
      }
    }
  }

  // print all registered types, if all_types >= 1
  if (all_types >= 1 && m_type_names.size() > 0 )
  {
    std::ostringstream str;
    str << "\nAll types registered with the factory: " << m_types.size() << " total\n";
    qs.push_back(str.str());

    bool banner = false;

    for( m_type_names_mapItT
         it = m_type_names.begin();
         it != m_type_names.end();
         it++ )
    {
      // filter out uvm_ classes (if all_types<2) and non-types (lookup strings)
      if( !(all_types < 2 && uvm_is_match("uvm_*", (*it).first)) //&& key == (*it).first.get_type_name()
          )
      {
        if (!banner)
        {
          qs.push_back("  Type Name\n");
          qs.push_back("  ---------\n");
          banner = true;
        }
        qs.push_back("  " + (*it).first + "\n");
      }
    }
  }

  qs.push_back("(*) Types with no associated type name will be printed as <unknown>\n\n####\n\n");

  UVM_INFO("UVM/FACTORY/PRINT", UVM_STRING_QUEUE_STREAMING_PACK(qs), UVM_NONE);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// member function: check_inst_override_exists
//
//! Implementation-defined member function
//----------------------------------------------------------------------------

bool uvm_default_factory::check_inst_override_exists( uvm_object_wrapper* original_type,
                                              uvm_object_wrapper* override_type,
                                              const std::string& full_inst_path )
{
  uvm_factory_override* override = NULL;
  uvm_factory_queue_class* qc = NULL;

  // if exists
  if ( m_inst_override_queues.find(original_type) != m_inst_override_queues.end() )
    qc = m_inst_override_queues[original_type];
  else
    return false;

  m_overrides_listItT it;

  for (it = qc->queue.begin();
       it != qc->queue.end();
       it++)
  {
    override = (*it);

    if (override->full_inst_path == full_inst_path &&
        override->orig_type == original_type &&
        override->ovrd_type == override_type &&
        override->orig_type_name == original_type->get_type_name())
    {
      std::ostringstream msg;
      msg << "Instance override for '" << original_type->get_type_name()
          << "' already exists: override type '"
          << override_type->get_type_name()
          << "' with full_inst_path '" << full_inst_path
          << "'";
      uvm_report_info("DUPOVRD", msg.str(), UVM_HIGH);
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------------------
// member function: m_has_wildcard
//
//! Implementation-defined member function
//----------------------------------------------------------------------------

bool uvm_default_factory::m_has_wildcard( const std::string& nm )
{
  size_t found;
  found = nm.find("*");
  if ( found != std::string::npos ) return true;
  found = nm.find("?");
  if ( found != std::string::npos ) return true;
  return false;
}

//----------------------------------------------------------------------------
// member function: m_debug_create
//
//! Implementation-defined member function
//----------------------------------------------------------------------------

void uvm_default_factory::m_debug_create( const std::string& requested_type_name,
                                  uvm_object_wrapper* requested_type,
                                  const std::string& parent_inst_path,
                                  const std::string& name )
{
  std::string full_inst_path;
  uvm_object_wrapper* result;
  std::string loc_requested_type_name = requested_type_name;

  if (parent_inst_path.empty())
    full_inst_path = name;
  else if (!name.empty())
    full_inst_path = parent_inst_path + "." + name;
  else
    full_inst_path = parent_inst_path;

  m_override_info.clear();

  if (requested_type == NULL)
  {
    if (m_type_names.find(requested_type_name) == m_type_names.end() &&
        m_lookup_strs.find(requested_type_name) == m_lookup_strs.end())
    {
      uvm_report_warning("Factory Warning", "The factory does not recognize '"
        + requested_type_name + "' as a registered type.", UVM_NONE);
      return;
    }
    m_debug_pass = true;

    result = find_override_by_name(requested_type_name, full_inst_path);
  }
  else
  {
    m_debug_pass = true;
    if (m_types.find(requested_type) == m_types.end() ) // if not exists
      do_register(requested_type);

    result = find_override_by_type(requested_type, full_inst_path);

    if (requested_type_name.empty())
      loc_requested_type_name = requested_type->get_type_name();
  }

  m_debug_display(loc_requested_type_name, result, full_inst_path);
  m_debug_pass = false;

  for( m_overrides_listItT
       it = m_override_info.begin();
       it != m_override_info.end();
       it++ )
    (*it)->selected = false;
}

//----------------------------------------------------------------------------
// member function: m_debug_display
//
//! Implementation-defined member function
//----------------------------------------------------------------------------

void uvm_default_factory::m_debug_display( const std::string& requested_type_name,
                                   uvm_object_wrapper* result,
                                   const std::string& full_inst_path )
{
  unsigned int max1 = 13;
  unsigned int max2 = 13;
  unsigned int max3 = 13;
  std::string dash = "---------------------------------------------------------------------------------------------------";
  std::string space= "                                                                                                   ";
  std::vector<std::string> qs;

  qs.push_back("\n#### Factory Override Information (*)\n\n");

  std::ostringstream str;
  str << "Given a request for an object of type '" << requested_type_name
      << "' with an instance path of '" << full_inst_path
      << "', the factory encountered";

  if (m_override_info.size() == 0)
    qs.push_back(str.str() + " no relevant overrides.\n");
  else
  {

    qs.push_back(str.str() + " the following relevant overrides." +
      "An 'x' next to a\n match indicates a match that was ignored.\n\n");

    for( m_overrides_listItT
         it = m_override_info.begin();
         it != m_override_info.end();
         it++ )
    {
      if ( (*it)->orig_type_name.length() > max1)
        max1 = (*it)->orig_type_name.length();

      if ( (*it)->full_inst_path.length() > max2)
        max2 = (*it)->full_inst_path.length();

      if ( (*it)->ovrd_type_name.length() > max3)
        max3 = (*it)->ovrd_type_name.length();
    }

    if (max1 < 13) max1 = 13;
    if (max2 < 13) max2 = 13;
    if (max3 < 13) max3 = 13;

    qs.push_back("  Original Type" + space.substr(1, max1-13) +
         "  Instance Path" + space.substr(1, max2-13) +
         "  Override Type" + space.substr(1,max3-13) + "\n");

    qs.push_back("  " + dash.substr(1,max1) +
         "  " + dash.substr(1,max2) +
         "  " + dash.substr(1,max3) );

    for( m_overrides_listItT
         it = m_override_info.begin();
         it != m_override_info.end();
         it++ )
    {
      std::ostringstream str;
      str << ((*it)->selected ? "  " : "x ")
          << (*it)->orig_type_name
          << space.substr(1, max1 - (*it)->orig_type_name.length() )
          << "  " << (*it)->full_inst_path
          << space.substr(1, max2 - (*it)->full_inst_path.length() )
          << "  " << (*it)->ovrd_type_name
          << space.substr(1, max3 - (*it)->ovrd_type_name.length() );

      if ((*it)->full_inst_path == "*")
        qs.push_back( str.str() + "  <type override>");
      else
        qs.push_back("\n");
    }
    qs.push_back("\n");
  }


  qs.push_back("Result:\n\n");
  qs.push_back("  The factory will produce an object of type '" +
       ( result == NULL ? requested_type_name : result->get_type_name() ) +
       "'\n");

  qs.push_back("\n(*) Types with no associated type name will be printed as <unknown>\n\n####\n\n");

  UVM_INFO("UVM/FACTORY/DUMP", UVM_STRING_QUEUE_STREAMING_PACK(qs), UVM_NONE);
}

//----------------------------------------------------------------------------
// member function: m_delete_object
//
//! Implementation-defined member function
//----------------------------------------------------------------------------

bool uvm_default_factory::m_delete_object(uvm_object* obj)
{
  m_obj_t_listItT it = std::find(m_obj_t_list.begin(), m_obj_t_list.end(), obj);

  if (it == m_obj_t_list.end())
    return false;

  delete (*it);
  m_obj_t_list.erase(it);

  return true;
}

//----------------------------------------------------------------------------
// member function: m_delete_all_objects
//
//! Implementation-defined member function
//----------------------------------------------------------------------------
void uvm_default_factory::m_delete_all_objects()
{
  for( m_obj_t_listItT 
       it = m_obj_t_list.begin();
       it != m_obj_t_list.end(); 
       it++)
    delete (*it);

  m_obj_t_list.clear();
}

//----------------------------------------------------------------------------
// member function: m_delete_component
//
//! Implementation-defined member function
//----------------------------------------------------------------------------

bool uvm_default_factory::m_delete_component( uvm_component* comp )
{
  m_comp_t_listItT it = std::find(m_comp_t_list.begin(), m_comp_t_list.end(), comp);

  if (it == m_comp_t_list.end())
    return false;

  delete (*it);
  m_comp_t_list.erase(it);

  return true;
}

//----------------------------------------------------------------------------
// member function: m_delete_all_components
//
//! Implementation-defined member function
//----------------------------------------------------------------------------
void uvm_default_factory::m_delete_all_components()
{
  for( m_comp_t_listItT 
       it = m_comp_t_list.begin();
       it != m_comp_t_list.end(); 
       it++)
    delete (*it);

  m_comp_t_list.clear();
}

////////////////////

//------------------------------------------------------------------------------
// implementation of global functions to create uvm_object/uvm_component,
// and to set up overrides.
//------------------------------------------------------------------------------
// TODO move to command line processor

void uvm_set_type_override(
  const std::string& original_type_name,
  const std::string& override_type_name,
  bool replace )
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_factory* factory = cs->get_factory();
  factory->set_type_override_by_name(
    original_type_name, override_type_name, replace );
}

void uvm_set_inst_override(
  const std::string& original_type_name,
  const std::string& override_type_name,
  const std::string& full_inst_path )
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_factory* factory = cs->get_factory();
  factory->set_inst_override_by_name(
    original_type_name, override_type_name, full_inst_path );
}

/////////////

} // namespace uvm
