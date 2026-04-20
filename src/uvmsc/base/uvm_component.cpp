//----------------------------------------------------------------------
//   Copyright 2012-2021 NXP B.V.
//   Copyright 2013-2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
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
#include <string>
#include <cstring>
#include <algorithm>

#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif
#include <systemc>
#include "sysc/kernel/sc_dynamic_processes.h"

#include "uvmsc/base/uvm_root.h"
#include "uvmsc/base/uvm_component.h"
#include "uvmsc/base/uvm_component_name.h"
#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"
#include "uvmsc/factory/uvm_factory.h"
#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/phasing/uvm_common_phases.h"
#include "uvmsc/phasing/uvm_phase.h"
#include "uvmsc/conf/uvm_config_db.h"
#include "uvmsc/seq/uvm_sequencer_base.h"
#include "uvmsc/print/uvm_printer.h"
#include "uvmsc/macros/uvm_message_defines.h"
#include "uvmsc/report/uvm_report_object.h"

using namespace sc_core;

namespace uvm {

namespace {

bool& print_config_matches_ref()
{
  return uvm_coreservice_t::get()->get_uvm_component__print_config_matches();
}

} // namespace

//----------------------------------------------------------------------------
// Class: uvm_component
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Initialization of data members
//----------------------------------------------------------------------------
// Refactored from the former uvm_component::global_timeout_spawned_ global to uvm_coreservice_t.
// Refactored from the former uvm_component::_print_config_matches global to uvm_coreservice_t.

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

uvm_component::uvm_component( uvm_component_name nm )
: sc_module( nm.name() ), uvm_report_object((std::string)nm.name())
{
  // prevent recursive registration of uvm_root module
  bool top = ( strstr(nm, "uvm_top") != nullptr );

  set_name(std::string(nm));

  start_report_handler(this->name());

  if (!top) {

    print_enabled = true;

    sc_object* sc_parent = this->get_parent_object();
    uvm_component* uvm_parent = dynamic_cast<uvm_component*>(sc_parent);

    if ( uvm_parent == nullptr)
    {
      std::ostringstream str;
      str << "The parent of UVM component '" << nm << "' is not a UVM component. uvm_top is used instead.";
      uvm_report_info("NOPARENT", str.str(), UVM_HIGH);

      uvm_coreservice_t* cs = uvm_coreservice_t::get();
      uvm_root* root = cs->get_root();
      m_comp_parent = root;
    }
    else
      m_comp_parent = uvm_parent;

    set_report_verbosity_level(m_comp_parent->get_report_verbosity_level());

    if (!m_comp_parent->m_add_child(this))
    {
      std::ostringstream str;
      str << "Unable to add child '" << nm
          << "' to parent '" << m_comp_parent->get_name() << "'.";
      UVM_WARNING("NOPARENT", str.str());
      m_comp_parent = nullptr;
    }
    else
    {
      //m_domain = m_comp_parent->m_domain; //!inherit domains from parents (or uvm_top)
      uvm_domain::get_common_domain();
      m_domain = uvm_domain::get_uvm_domain();
    }

  }
  else
  {

    print_enabled = false;

  }

  //!default settings
  m_build_done = false;
  m_phasing_active = 0;
  recording_detail = UVM_NONE;
  m_current_phase = nullptr;
  m_children.clear();
}


//----------------------------------------------------------------------------
// member function: get_parent (virtual)
//
//! Returns a handle to this component's parent, or null if it has no parent.
//----------------------------------------------------------------------------

uvm_component* uvm_component::get_parent() const
{
  return m_comp_parent;
}

//----------------------------------------------------------------------------
// member function: get_full_name (virtual)
//
//! Returns the full hierarchical name of this object. The default
//! implementation concatenates the hierarchical name of the parent, if any,
//! with the leaf name of this object, as given by uvm_object::get_name.
//----------------------------------------------------------------------------

const std::string uvm_component::get_full_name() const
{
  // Note--Implementation choice to construct full name once since the
  // full name may be used often for lookups.
  if (m_name.empty())
    return get_name();
  else
    return m_name;
}


//----------------------------------------------------------------------------
// member function: get_children
//
//! This function populates the end of the children array with the
//! list of this component's children.
//----------------------------------------------------------------------------

void uvm_component::get_children( std::vector<uvm_component*>& children ) const
{
  for( m_children_mapcItT it = m_children.begin();
       it != m_children.end();
       it++ )
    children.push_back((*it).second);
}


//----------------------------------------------------------------------------
// member function: get_child
//
//! Member function to iterate through his component's children, if any.
//----------------------------------------------------------------------------

uvm_component* uvm_component::get_child( const std::string& name ) const
{
  if (m_children.find(name) != m_children.end() ) // exists
    return m_children.find(name)->second;

  std::ostringstream str;
  str << "Component with name '" << name
      << "' is not a child of component '" << get_full_name() << "'.";
  uvm_report_warning("NOCHILD",str.str());
  return nullptr;
}


//----------------------------------------------------------------------------
// member function: get_next_child
//
//! Member function to iterate through his component's children, if any.
//----------------------------------------------------------------------------

int uvm_component::get_next_child( std::string& name ) const
{
  m_children_cnt++;

  if (m_children_cnt >= m_children.size())
    return 0;

  m_children_mapcItT it = m_children.begin();

  for (unsigned int i = 0; i < m_children_cnt; i++ )
    it++;

  name = it->first;

  if (!name.empty()) return 1;
    else return 0;
}


//----------------------------------------------------------------------------
// member function: get_first_child
//
//! Member function to iterate through his component's children, if any.
//----------------------------------------------------------------------------

int uvm_component::get_first_child( std::string& name ) const
{
  m_children_mapcItT it = m_children.begin();
  m_children_cnt = 0;

  if (m_children.size() == 0)
    return 0;

  name = it->first;

  if (!name.empty()) return 1;
    else return 0;
}


//----------------------------------------------------------------------------
// member function: get_num_children
//
//! Returns the number of this component's children.
//----------------------------------------------------------------------------


int uvm_component::get_num_children() const
{
  return (int)m_children.size();
}


//----------------------------------------------------------------------------
// member function: has_child
//
//! Returns true if this component has a child with the given name, and false otherwise.
//----------------------------------------------------------------------------


bool uvm_component::has_child( const std::string& name) const
{
  return (m_children.find(name) != m_children.end());
}


//----------------------------------------------------------------------------
// member function: lookup
//
//! Looks for a component with the given hierarchical name relative to this
//! component. If the given name is preceded with a '.' (dot), then the search
//! begins relative to the top level (absolute lookup). The handle of the
//! matching component is returned, else nullptr. The name must not contain
//! wildcards.
//----------------------------------------------------------------------------

uvm_component* uvm_component::lookup( const std::string& name ) const
{
  std::string leaf, remainder;
  const uvm_component* comp;

  const uvm_root* top;
  uvm_coreservice_t* cs;
  cs = uvm_coreservice_t::get();
  top = cs->get_root();

  comp = this;

  m_extract_name(name, leaf, remainder);

  if (leaf.empty())
  {
    comp = top; //!absolute lookup
    m_extract_name(remainder, leaf, remainder);
  }

  if (!comp->has_child(leaf))
  {
    UVM_WARNING("Lookup Error", "Cannot find child " + leaf);
    return nullptr;
  }

  if(!remainder.empty())
    return comp->m_children.find(leaf)->second->lookup(remainder);

  return comp->m_children.find(leaf)->second;
}


//----------------------------------------------------------------------------
// member function: get_depth
//
//! Returns the component's depth from the root level. uvm_top has a
//! depth of 0. The test and any other top level components have a depth
//! of 1, and so on.
//----------------------------------------------------------------------------

unsigned int uvm_component::get_depth() const
{
  std::string nm = name();
  if( nm.size() == 0 ) return 0;
  int depth;
  depth = count(nm.begin(), nm.end(), '.');

  return depth+1;
}

//----------------------------------------------------------------------------
// Group: Phasing Interface
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// member function: build_phase
//
//! Any override should call build_phase(uvm_phase&) of its base class to execute
//! the automatic configuration of fields registered in the component by calling
//! apply_config_settings.
//! To turn off automatic configuration for a component,
//! do not call the build_phase(uvm_phase&) of its base class
//!
//! This member function should never be called directly.
//----------------------------------------------------------------------------

void uvm_component::build_phase( uvm_phase& phase )
{
  m_build_done = true;
  apply_config_settings(print_config_matches_ref());
  if(m_phasing_active == 0)
    uvm_report_warning("BUILD", "The member function build_phase() has been called explicitly, outside of the phasing system. This may lead to unexpected behavior.");
}

//----------------------------------------------------------------------------
// member function: connect_phase
//
//! Placeholder for the connect phase base class implementation
//!
//! This member function should never be called directly.
//----------------------------------------------------------------------------

void uvm_component::connect_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function: end_of_elaboration_phase
//
//! Placeholder for the sc_module::end_of_elaboration_phase base class implementation
//!
//! This member function should never be called directly.
//----------------------------------------------------------------------------

void uvm_component::end_of_elaboration_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function: start_of_simulation_phase
//
//! Placeholder for the sc_module::start_of_simulation_phase base class implementation
//!
//! This member function should never be called directly.
//----------------------------------------------------------------------------

void uvm_component::start_of_simulation_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function: run phase.
//
//! The main thread of execution.
//! The component should not declare run_phase as a thread process - it is
//! automatically spawned by uvm_component's constructor.
//!
//! The member function should never be called directly.
//----------------------------------------------------------------------------

void uvm_component::run_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function: pre_reset_phase
//
//! Placeholder for the pre_reset_phase base class implementation
//!
//! This member function should not be called directly.
//----------------------------------------------------------------------------

void uvm_component::pre_reset_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function: reset_phase
//
//! Placeholder for the reset_phase base class implementation
//!
//! This member function should not be called directly.
//----------------------------------------------------------------------------

void uvm_component::reset_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function: post_reset_phase
//
//! Placeholder for the post_reset_phase base class implementation
//!
//! This member function should not be called directly.
//----------------------------------------------------------------------------

void uvm_component::post_reset_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function: pre_configure_phase
//
//! Placeholder for the pre_configure_phase base class implementation
//!
//! This member function should not be called directly.
//----------------------------------------------------------------------------

void uvm_component::pre_configure_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function: configure_phase
//
//! Placeholder for the configure_phase base class implementation
//!
//! This member function should not be called directly.
//----------------------------------------------------------------------------

void uvm_component::configure_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function: post_configure_phase
//
//! Placeholder for the post_configure_phase base class implementation
//!
//! This member function should not be called directly.
//----------------------------------------------------------------------------

void uvm_component::post_configure_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function: pre_main_phase
//
//! Placeholder for the pre_main_phase base class implementation
//!
//! This member function should not be called directly.
//----------------------------------------------------------------------------

void uvm_component::pre_main_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function: main_phase
//
//! Placeholder for the main_phase base class implementation
//!
//! This member function should not be called directly.
//----------------------------------------------------------------------------

void uvm_component::main_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function: post_main_phase
//
//! Placeholder for the post_main_phase base class implementation
//!
//! This member function should not be called directly.
//----------------------------------------------------------------------------

void uvm_component::post_main_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function: pre_shutdown_phase
//
//! Placeholder for the pre_shutdown_phase base class implementation
//!
//! This member function should not be called directly.
//----------------------------------------------------------------------------

void uvm_component::pre_shutdown_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function:shutdown_phase
//
//! Placeholder for the shutdown_phase base class implementation
//!
//! This member function should not be called directly.
//----------------------------------------------------------------------------

void uvm_component::shutdown_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function: post_shutdown_phase
//
//! Placeholder for the post_shutdown_phase base class implementation
//!
//! This member function should not be called directly.
//----------------------------------------------------------------------------

void uvm_component::post_shutdown_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function: extract_phase
//
//! Placeholder for the extract_phase base class implementation
//!
//! This member function should not be called directly.
//----------------------------------------------------------------------------

void uvm_component::extract_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function: check_phase
//
//! Placeholder for the check_phase base class implementation
//!
//! This member function should not be called directly.
//----------------------------------------------------------------------------

void uvm_component::check_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function: report_phase
//
//! Placeholder for the report_phase base class implementation
//!
//! This member function should not be called directly.
//----------------------------------------------------------------------------

void uvm_component::report_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function: final_phase
//
//! Placeholder for the final_phase base class implementation
//!
//! This member function should not be called directly.
//----------------------------------------------------------------------------

void uvm_component::final_phase( uvm_phase& phase )
{}

//----------------------------------------------------------------------------
// member function: phase_started
//
//! Invoked at the start of each phase. The \p phase argument specifies
//! the phase being started. Any threads spawned in this callback are
//! not affected when the phase ends.
//----------------------------------------------------------------------------

void uvm_component::phase_started( uvm_phase& phase )
{
  // placeholder for application to use this phase
}

//----------------------------------------------------------------------------
// member function: phase_ready_to_end
//
//! Invoked when all objections to ending the given \p phase and all
//! sibling phases have been dropped, thus indicating that ~phase~ is
//! ready to begin a clean exit. Sibling phases are any phases that
//! have a common successor phase in the schedule plus any phases that
//! sync'd to the current phase. Components needing to consume delta
//! cycles or advance time to perform a clean exit from the phase
//! may raise the phase's objection.
//----------------------------------------------------------------------------

void uvm_component::phase_ready_to_end( uvm_phase& phase )
{
  // placeholder for application to use this phase
}

//----------------------------------------------------------------------------
// member function: phase_ended
//
//! Invoked at the end of each phase. The \p phase argument specifies
//! the phase that is ending.  Any threads spawned in this callback are
//! not affected when the phase ends.
//----------------------------------------------------------------------------
void uvm_component::phase_ended( uvm_phase& phase )
{
  // placeholder for application to use this phase
}

//--------------------------------------------------------------------
//!phase / schedule / domain API
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// member function: set_domain
//
//! Apply a phase domain to this component and, if \p hier is set,
//! recursively to all its children.
//!
//! Calls the virtual define_domain method, which derived components can
//! override to augment or replace the domain definition of its base class.
//--------------------------------------------------------------------

void uvm_component::set_domain( uvm_domain& domain, int hier )
{
  m_domain = &domain;
  define_domain(domain);

  if (hier)
    for ( m_children_mapcItT it = m_children.begin(); it != m_children.end(); it++ )
      (*it).second->set_domain(domain);
}

//--------------------------------------------------------------------
// member function: get_domain
//
//! Return handle to the phase domain set on this component
//--------------------------------------------------------------------

uvm_domain* uvm_component::get_domain() const
{
  return m_domain;
}


//--------------------------------------------------------------------
// member function: define_domain
//
//! Builds custom phase schedules into the provided domain handle.
//!
//! This member function is called by set_domain(),
//! which integrators use to specify
//! this component belongs in a domain apart from the default 'uvm' domain.
//--------------------------------------------------------------------

void uvm_component::define_domain( uvm_domain& domain )
{
  m_schedule = domain.find_by_name("uvm_sched");

  if (m_schedule == nullptr)
  {
    m_schedule = new uvm_phase("uvm_sched", UVM_PHASE_SCHEDULE);
    m_schedule_list.push_back(m_schedule); // TODO keep track of added schedules (to be deleted afterwards)
    uvm_domain::add_uvm_phases(m_schedule);
    domain.add(m_schedule);

    m_common = uvm_domain::get_common_domain();
    if (m_common->find(&domain, false) == nullptr)
      m_common->add(&domain, m_common->find(uvm_run_phase::get()) );
  }
  else
    uvm_report_warning("SETDOM", "User-specified domain '" + domain.get_name() + "' already exists for component '"
      + get_name() + "' and will be ignored", UVM_DEBUG);
}


//--------------------------------------------------------------------
// member function: set_phase_imp
//
//! Override the default implementation for a phase on this component (tree) with a
//! custom one, which must be created as a singleton object extending the default
//! one and implementing required behavior in exec and traverse methods
//!
//! The optional argument \p hier specifies whether to apply the custom functor
//! to the whole tree or just this component.
//--------------------------------------------------------------------

void uvm_component::set_phase_imp( uvm_phase* phase, uvm_phase* imp, int hier )
{
  m_phase_imps[phase] = imp;
  if (hier)
    for ( m_children_mapcItT it = m_children.begin(); it != m_children.end(); it++ )
      (*it).second->set_phase_imp(phase,imp,hier);
}


//--------------------------------------------------------------------
// member function: suspend
//
//! Suspend the component.
//! A suspended component can be subsequently resumed using resume().
//--------------------------------------------------------------------

bool uvm_component::suspend()
{
  if (m_run_handle.valid() && !m_run_handle.terminated())
  {
    m_run_handle.suspend(SC_INCLUDE_DESCENDANTS);
    return true;
  }
  else
    return false;
}

//--------------------------------------------------------------------
// member function: resume
//
//! Resume the component.
//! Member function to resume a component that was previously suspended
//! using suspend.
//! A component may start in the suspended state and
//! may need to be explicitly resumed.
//--------------------------------------------------------------------

bool uvm_component::resume()
{
  if (m_run_handle.valid() && !m_run_handle.terminated())
  {
    m_run_handle.resume(SC_INCLUDE_DESCENDANTS);
    return true;
  }
  else
    return false;
}


//----------------------------------------------------------------------------
// Configuration interface
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// member function: check_config_usage
//
//! Check all configuration settings in a components configuration table
//! to determine if the setting has been used, overridden or not used.
//! When \p recurse is true (default), configuration for this and all child
//! components are recursively checked. This function is automatically
//! called in the check phase, but can be manually called at any time.
//----------------------------------------------------------------------------

void uvm_component::check_config_usage( bool recurse )
{
  // TODO - do we really need this?
}

//----------------------------------------------------------------------------
// member function: apply_config_settings
//
//! Searches for all config settings matching this component's instance path.
//! For each match, the appropriate set_*_local method is called using the
//! matching config setting's field_name and value. Provided the set_*_local
//! method is implemented, the component property associated with the
//! field_name is assigned the given value.
//----------------------------------------------------------------------------

void uvm_component::apply_config_settings( bool verbose )
{
  // TODO - do we really need this?
}

//----------------------------------------------------------------------------
// member function: print_config
//
//! Print_config_settings prints all configuration information for this
//! component, as set by previous calls to set_config_* and exports to
//! the resources pool.  The settings are printing in the order of
//! their precedence.
//! If recurse is set to true, then configuration information for all
//! children and below are printed as well.
//! if audit is set to true, then the audit trail for each resource is printed
//! along with the resource name and value
//----------------------------------------------------------------------------

void uvm_component::print_config( bool recurse, bool audit ) const
{
  uvm_resource_pool* rp = uvm_resource_pool::get();

  uvm_report_info("CFGPRT","visible resources:", UVM_INFO);

  rp->print_resources(rp->lookup_scope(get_full_name()), audit);

  if(recurse)
  {
    uvm_component* c;
    for( m_children_mapcItT it = m_children.begin();
         it != m_children.end();
         it++ )
    {
      c = (*it).second;
      c->print_config(recurse, audit);
    }
  }
}

//----------------------------------------------------------------------------
// member function: print_config_with_audit
//
//! Operates the same as print_config except that the audit argument is
//! forced to true. This interface makes user code a bit more readable as
//! it avoids multiple arbitrary bit settings in the argument list.
//!
//! If recurse is set to true, then configuration information for all
//! children and below are printed as well.
//----------------------------------------------------------------------------

void uvm_component::print_config_with_audit( bool recurse ) const
{
  print_config(recurse, true);
}

//----------------------------------------------------------------------------
// member function: print_config_matches
//
//! If enabled, all configuration getters will print info about
//! matching configuration settings as they are being applied.
//----------------------------------------------------------------------------

void uvm_component::print_config_matches( bool enable )
{
  if (enable)
    print_config_matches_ref() = true;
}


//----------------------------------------------------------------------------
// Group: Objection Interface
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// member function: raised
//
//! The member function raised shall be called when this or a descendant of this component
//! instance raises the specfied \p objection. The \p source_obj is the object
//! that originally raised the objection.
//! The \p description is optionally provided by the \p source_obj to give a
//! reason for raising the objection. The \p count indicates the number of
//! objections raised by the \p source_obj.
//----------------------------------------------------------------------------

void uvm_component::raised( uvm_objection* objection,
                            uvm_object* source_obj,
                            const std::string& description,
                            int count )
{
  // callback for application
}


//----------------------------------------------------------------------------
// member function: dropped
//
//! The member function dropped shall be called when this or a descendant of this component
//! instance drops the specfied \p objection. The \p source_obj is the object
//! that originally dropped the objection.
//! The \p description is optionally provided by the \p source_obj to give a
//! reason for dropping the objection. The \p count indicates the number of
//! objections dropped by the the \p source_obj.
//----------------------------------------------------------------------------

void uvm_component::dropped( uvm_objection* objection,
                             uvm_object* source_obj,
                             const std::string& description,
                             int count )
{
  // callback for application
}

//----------------------------------------------------------------------------
// member function: all_dropped
//
//! The member function all_droppped shall be called when all objections have been
//! dropped by this component and all its descendants.  The \p source_obj is the
//! object that dropped the last objection.
//! The \p description is optionally provided by the \p source_obj to give a
//! reason for raising the objection. The \p count indicates the number of
//! objections dropped by the the \p source_obj.
//----------------------------------------------------------------------------

void uvm_component::all_dropped( uvm_objection* objection,
                                 uvm_object* source_obj,
                                 const std::string& description,
                                 int count )
{
  // callback for application
}

//----------------------------------------------------------------------------
// Group: Factory Interface
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// member function: create_component
//
//! A convenience function for uvm_factory::create_component_by_name,
//! this method calls upon the factory to create a new child component
//! whose type corresponds to the preregistered type name, \p requested_type_name,
//! and instance name, \p name.
//----------------------------------------------------------------------------

uvm_component* uvm_component::create_component( const std::string& requested_type_name,
                                                const std::string& name )
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  auto factory = cs->get_factory();

  return factory->create_component_by_name( requested_type_name,
                                            get_full_name(),
                                            name, this );
}


//----------------------------------------------------------------------------
// member function: create_object
//
//! A convenience function for uvm_factory::create_object_by_name,
//! this method calls upon the factory to create a new object
//! whose type corresponds to the preregistered type name,
//! \p requested_type_name, and instance name, \p name.
//----------------------------------------------------------------------------

uvm_object* uvm_component::create_object( const std::string& requested_type_name,
                                          const std::string& name )
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  auto factory = cs->get_factory();

  return factory->create_object_by_name( requested_type_name,
                                         get_full_name(),
                                         name );
}

//----------------------------------------------------------------------------
// member function: set_type_override_by_type (static)
//
//! A convenience function for uvm_factory::set_type_override_by_type, this
//! member function registers a factory override for components and objects created at
//! this level of hierarchy or below.
//----------------------------------------------------------------------------

void uvm_component::set_type_override_by_type( uvm_object_wrapper* original_type,
                                               uvm_object_wrapper* override_type,
                                               bool replace )
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  auto factory = cs->get_factory();

  factory->set_type_override_by_type( original_type, override_type, replace );
}

//----------------------------------------------------------------------------
// member function: set_inst_override_by_type
//
//! A convenience function for uvm_factory::set_inst_override_by_type, this
//! member function registers a factory override for components and objects created at
//! this level of hierarchy or below.
//----------------------------------------------------------------------------

void uvm_component::set_inst_override_by_type( const std::string& relative_inst_path,
                                               uvm_object_wrapper* original_type,
                                               uvm_object_wrapper* override_type )
{
  std::string full_inst_path = prepend_name(relative_inst_path);
  // TODO note: for some reason, the parameter order is slightly different between component and factory

  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  auto factory = cs->get_factory();

  factory->set_inst_override_by_type( original_type, override_type, full_inst_path );
}

//----------------------------------------------------------------------------
// member function: set_type_override (static)
//
//! A convenience function for uvm_factory::set_type_override_by_name,
//! this method configures the factory to create an object of type
//! \p override_type_name whenever the factory is asked to produce a type
//! represented by \p original_type_name.
//----------------------------------------------------------------------------

void uvm_component::set_type_override( const std::string& original_type_name,
                                       const std::string& override_type_name,
                                       bool replace )
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  auto factory = cs->get_factory();

  factory->set_type_override_by_name( original_type_name, override_type_name, replace );
}

//----------------------------------------------------------------------------
// member function: set_inst_override
//
//! A convenience function for uvm_factory::set_inst_override_by_name, this
//! method registers a factory override for components created at this level
//! of hierarchy or below.
//----------------------------------------------------------------------------

void uvm_component::set_inst_override( const std::string& relative_inst_path,
                                       const std::string& original_type_name,
                                       const std::string& override_type_name )
{
  std::string path = prepend_name(relative_inst_path);

  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  auto factory = cs->get_factory();

  factory->set_inst_override_by_name( original_type_name, override_type_name, path  );
}

//----------------------------------------------------------------------------
// member function: print_override_info
//
//! This factory debug method performs the same lookup process as #create_object
//! and #create_component, but instead of creating an object, it prints
//! information about what type of object would be created given the
//! provided arguments.
//----------------------------------------------------------------------------

void uvm_component::print_override_info( const std::string& requested_type_name,
                                         const std::string& name )
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  auto factory = cs->get_factory();

  factory->debug_create_by_name(requested_type_name, get_full_name(), name);
}

//----------------------------------------------------------------------------
// Group: Hierarchical Reporting Interface
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// member function: set_report_id_verbosity_hier

//! The member function #set_report_id_verbosity_hier shall recursively associate
//! the specified \p verbosity with reports of the given \p id. A verbosity associated
//! with a particular severity-id pair, using member function
//! #set_report_severity_id_verbosity_hier, shall take precedence over a verbosity
//! associated by this member function.
//----------------------------------------------------------------------------

void uvm_component::set_report_id_verbosity_hier( const std::string& id,
                                                  int verbosity )
{
  set_report_id_verbosity(id, verbosity);
  for( m_children_mapcItT it = m_children.begin();
       it != m_children.end();
       it++ )
    (*it).second->set_report_id_verbosity_hier(id, verbosity);
}


//----------------------------------------------------------------------------
// member function: set_report_severity_id_verbosity_hier
//
//! The member function #set_report_severity_id_verbosity_hier shall recursively
//! associate the specified \p verbosity with reports of the given \p severity with
//! \p id pair. An verbosity associated with a particular severity-id pair takes
//! precedence over an verbosity associated with id, which takes precedence
//! over a verbosity associated with a severity.
//----------------------------------------------------------------------------

void uvm_component::set_report_severity_id_verbosity_hier( uvm_severity severity,
                                                           const std::string& id,
                                                           int verbosity )
{
  set_report_severity_id_verbosity(severity, id, verbosity);
  for( m_children_mapcItT it = m_children.begin();
       it != m_children.end();
       it++ )
    (*it).second->set_report_severity_id_verbosity_hier(severity, id, verbosity);
}

//----------------------------------------------------------------------------
// member function: set_report_severity_action_hier
//
//! The member function #set_report_severity_action_hier shall recursively
//! associate the specified \p action with reports of the given \p severity.
//! An action associated with a particular severity-id pair shall take
//! precedence over an action associated with id, which shall take precedence
//! over an action associated with a severity as defined in this member function.
//----------------------------------------------------------------------------

void uvm_component::set_report_severity_action_hier( uvm_severity severity,
                                                     uvm_action action )
{
  set_report_severity_action(severity, action);
  for( m_children_mapcItT it = m_children.begin();
       it != m_children.end();
       it++ )
    (*it).second->set_report_severity_action_hier(severity, action);
}

//----------------------------------------------------------------------------
// member function: set_report_id_action_hier

//! The member function #set_report_id_action_hier shall recursively associate
//! the specified \p action with reports of the given \p id. An action associated
//! with a particular severity-id pair shall take precedence over an action
//! associated with id as defined in this member function.
//----------------------------------------------------------------------------

void uvm_component::set_report_id_action_hier( const std::string& id,
                                               uvm_action action )
{
  set_report_id_action(id, action);
  for( m_children_mapcItT it = m_children.begin();
       it != m_children.end();
       it++ )
    (*it).second->set_report_id_action_hier(id, action);
}

//----------------------------------------------------------------------------
// member function: set_report_severity_id_action_hier
//
//! The member function #set_report_severity_id_action_hier shall recursively
//! associate the specified \p action with reports of the given \p severity with
//! \p id pair. An action associated with a particular severity-id pair shall
//! take precedence over an action associated with id, which shall take
//! precedence over an action associated with a severity.
//----------------------------------------------------------------------------

void uvm_component::set_report_severity_id_action_hier( uvm_severity severity,
                                                        const std::string& id,
                                                        uvm_action action )
{
  set_report_severity_id_action(severity, id, action);
  for( m_children_mapcItT it = m_children.begin();
       it != m_children.end();
       it++ )
    (*it).second->set_report_severity_id_action_hier(severity, id, action);
}


//----------------------------------------------------------------------------
// member function: set_report_default_file_hier
//
//! The member function #set_report_default_file_hier shall recursively associate
//! the report to the default \p file descriptor. A file associated with a particular
//! severity-id pair shall take precedence over a file associated with id, which
//! shall take precedence over a file associated with a severity, which shall take
//! precedence over the default file descriptor as defined in this member function.
//----------------------------------------------------------------------------

void uvm_component::set_report_default_file_hier( UVM_FILE file )
{
  set_report_default_file(file);
  for( m_children_mapcItT it = m_children.begin();
       it != m_children.end();
       it++ )
    (*it).second->set_report_default_file_hier(file);
}


//----------------------------------------------------------------------------
// member function: set_report_severity_file_hier
//
//! The member function #set_report_severity_file_hier shall recursively associate
//! the specified \p file descriptor with reports of the given \p severity. A file
//! associated with a particular severity-id pair shall take precedence over
//! a file associated with id, which shall take precedence over a file associated
//! with a severity as defined in this member function.
//----------------------------------------------------------------------------

void uvm_component::set_report_severity_file_hier( uvm_severity severity,
                                                   UVM_FILE file )
{
  set_report_severity_file(severity, file);
  for( m_children_mapcItT it = m_children.begin();
       it != m_children.end();
       it++ )
    (*it).second->set_report_severity_file_hier(severity, file);
}

//----------------------------------------------------------------------------
// member function: set_report_id_file_hier
//
//! The member function #set_report_id_file_hier shall recursively associate
//! the specified \p file descriptor with reports of the given \p id. A file
//! associated with a particular severity-id pair shall take precedence over
//! a file associated with id as defined in this member function.
//----------------------------------------------------------------------------

void uvm_component::set_report_id_file_hier( const std::string& id,
                                             UVM_FILE file)
{
  set_report_id_file(id, file);
  for( m_children_mapcItT it = m_children.begin();
       it != m_children.end();
       it++ )
    (*it).second->set_report_id_file_hier(id, file);
}

//----------------------------------------------------------------------------
// member function: set_report_severity_id_file_hier
//
//! The member function #set_report_severity_id_file_hier shall recursively
//! associate the specified \p file descriptor with reports of the given \p severity
//! and \p id pair. A file associated with a particular severity-id pair shall
//! take precedence over a file associated with id, which shall take precedence
//! over a file associated with a severity, which shall take precedence over
//! the default file descriptor.
//----------------------------------------------------------------------------

void uvm_component::set_report_severity_id_file_hier( uvm_severity severity,
                                                      const std::string& id,
                                                      UVM_FILE file )
{
  set_report_severity_id_file(severity, id, file);
  for( m_children_mapcItT it = m_children.begin();
       it != m_children.end();
       it++ )
    (*it).second->set_report_severity_id_file_hier(severity, id, file);
}


//----------------------------------------------------------------------------
// member function: set_report_verbosity_level_hier
//
//! The member function #set_report_verbosity_level_hier shall recursively set
//! the maximum \p verbosity level for reports for this component and all those
//! below it. Any report from this component subtree whose verbosity exceeds
//! this maximum will be ignored.
//----------------------------------------------------------------------------

void uvm_component::set_report_verbosity_level_hier( int verbosity )
{
  set_report_verbosity_level(verbosity);
  for( m_children_mapcItT it = m_children.begin();
      it != m_children.end();
      it++ )
    (*it).second->set_report_verbosity_level_hier(verbosity);
}

//----------------------------------------------------------------------------
// member function: pre_abort (virtual)
//
//! The member function #pre_abort shall be executed when the message system is executing a
//! uvm::UVM_EXIT action. The exit action causes an immediate termination of
//! the simulation, but the pre_abort callback hook gives components an
//! opportunity to provide additional information to the user before
//! the termination happens.
//! The pre_abort() callback hooks are called in a bottom-up fashion.
//----------------------------------------------------------------------------

void uvm_component::pre_abort()
{
  // callback for application
}


//----------------------------------------------------------------------------
// Group: Recording Interface
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// member function: accept_tr
//
//! This function marks the acceptance of a transaction, ~tr~, by this
//! component.
//----------------------------------------------------------------------------

void uvm_component::accept_tr( const uvm_transaction& tr,
                               const sc_time& accept_time )
{
  //TODO - add transaction recording
}

//----------------------------------------------------------------------------
// member function: do_accept_tr
//
//! The <accept_tr> method calls this function to accommodate any user-defined
//! post-accept action. Implementations should call super.do_accept_tr to
//! ensure correct operation.
//----------------------------------------------------------------------------

void uvm_component::do_accept_tr( const uvm_transaction& tr )
{
  //TODO - add transaction recording
}

//----------------------------------------------------------------------------
// member function: begin_tr
//
//! This function marks the start of a transaction, \p tr, by this component.
//----------------------------------------------------------------------------

int uvm_component::begin_tr( const uvm_transaction& tr,
                             const std::string& stream_name,
                             const std::string& label,
                             const std::string& desc,
                             const sc_time& begin_time,
                             int parent_handle )
{
  // TODO - add transaction recording
  return 0; //!dummy
}

//----------------------------------------------------------------------------
// member function: begin_child_tr
//
//! This function marks the start of a child transaction, \p tr, by this
//! component. Its operation is identical to that of #begin_tr, except that
//! an association is made between this transaction and the provided parent
//! transaction.
//! This association is vendor-specific.
//----------------------------------------------------------------------------

int uvm_component::begin_child_tr( const uvm_transaction& tr,
                                   int parent_handle,
                                   const std::string& stream_name,
                                   const std::string& label,
                                   const std::string& desc,
                                   const sc_time& begin_time )
{
  // TODO - add transaction recording
  return 0; // dummy
}

//----------------------------------------------------------------------------
// member function: do_begin_tr
//
//! The member functions #begin_tr and #begin_child_tr shall call this function to
//! accommodate any user-defined post-begin action. Implementations should call
//! the base class member function to ensure correct operation.
//----------------------------------------------------------------------------

void uvm_component::do_begin_tr( const uvm_transaction& tr,
                                 const std::string& stream_name,
                                 int tr_handle )
{
  // TODO - add transaction recording
}

//----------------------------------------------------------------------------
// member function: end_tr
//
//! This function marks the end of a transaction, \p tr, by this component.
//----------------------------------------------------------------------------

void uvm_component::end_tr( const uvm_transaction& tr,
                            const sc_time& end_time,
                            bool free_handle )
{
  // TODO - add transaction recording

  uvm_transaction* t;
  t = const_cast<uvm_transaction*>(&tr);
  t->end_tr(end_time, free_handle); // TODO make const method to avoid const_casting?
}

//----------------------------------------------------------------------------
// member function: do_end_tr
//
//! The member function #end_tr calls this function to accommodate any user-defined
//! post-end action. Implementations should call this base class member
//! function to ensure correct operation.
//----------------------------------------------------------------------------

void uvm_component::do_end_tr( const uvm_transaction& tr,
                               int tr_handle )
{
  // TODO - add transaction recording
}

//----------------------------------------------------------------------------
// member function: record_error_tr
//
//! This function marks an error transaction by a component. Properties of the
//! given #uvm_object, \p info, as implemented in its member function
//! uvm_object::do_record, are recorded to the transaction database.
//----------------------------------------------------------------------------

int uvm_component::record_error_tr( const std::string& stream_name,
                                    uvm_object* info,
                                    const std::string& label,
                                    const std::string& desc,
                                    const sc_time& error_time,
                                    bool keep_active )
{
  // TODO - add transaction recording
  return 0; // dummy
}

//----------------------------------------------------------------------------
// member function: record_event_tr
//
//! This function marks an event transaction by a component.
//----------------------------------------------------------------------------

int uvm_component::record_event_tr( const std::string& stream_name,
                                    uvm_object* info,
                                    const std::string& label,
                                    const std::string& desc,
                                    const sc_time& event_time,
                                    bool keep_active  )
{
  // TODO - add transaction recording
  return 0; // dummy
}



////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------

uvm_component::~uvm_component()
{
  // clean memory of dynamically added elements
  while(!m_schedule_list.empty())
    delete m_schedule_list.back(), m_schedule_list.pop_back();

  while(!m_config_object_wrapper_list.empty())
    delete m_config_object_wrapper_list.back(), m_config_object_wrapper_list.pop_back();
}


//----------------------------------------------------------------------------
// member function: print_config_settings - DEPRECATED
//
//! Called without arguments, print_config_settings prints all configuration
//! information for this component, as set by previous calls to set_config_*.
//! The settings are printing in the order of their precedence.
//----------------------------------------------------------------------------

void uvm_component::print_config_settings( const std::string& field,
                                           uvm_component* comp,
                                           bool recurse )
{
  UVM_FATAL("DEPRD", "This function has been deprecated.  Use print_config instead.");
}

//----------------------------------------------------------------------------
// member function: do_print (override)
//
// Implementation defined
//----------------------------------------------------------------------------

void uvm_component::do_print( const uvm_printer& printer ) const
{
  std::string v;
  uvm_object::do_print(printer);

  // It is printed only if its value is other than the default (UVM_NONE)
  if(recording_detail != UVM_NONE)
  {
    int size = 0 ; // TODO get real size of recording_detail?
    const char* separator = ".";
    switch (recording_detail)
    {
      case UVM_LOW : printer.print_generic("recording_detail", "uvm_verbosity",
        size, "UVM_LOW"); break;
      case UVM_MEDIUM : printer.print_generic("recording_detail", "uvm_verbosity",
        size, "UVM_MEDIUM"); break;
      case UVM_HIGH : printer.print_generic("recording_detail", "uvm_verbosity",
        size, "UVM_HIGH"); break;
      case UVM_FULL : printer.print_generic("recording_detail", "uvm_verbosity",
        size, "UVM_FULL"); break;
      default : printer.print_field_int("recording_detail", recording_detail,
        size, UVM_DEC, separator, "integer"); break;
    } //!switch
  } //!if
}

//----------------------------------------------------------------------------
// member function: set_name
//
//! Implementation defined
//
//! Renames this component to ~name~ and recalculates all descendants'
//! full names.
//----------------------------------------------------------------------------

void uvm_component::set_name( const std::string& name )
{
  if(!m_name.empty())
  {
    uvm_report_error("INVSTNM",
      "It is illegal to change the name of a component. The component name will not be changed to '"+
      std::string(name) + "'.");
    return;
  }
  uvm_object::set_name(name);
  m_set_full_name();
}

//----------------------------------------------------------------------------
// member function: before_end_of_elaboration (virtual)
//
//! Implementation defined - not part of UVM standard
//! note: directly called by SystemC kernel
//----------------------------------------------------------------------------

void uvm_component::before_end_of_elaboration()
{}

//----------------------------------------------------------------------------
// member function: end_of_elaboration (virtual)
//
//! Implementation defined - not part of UVM standard
//! note: directly called by SystemC kernel
//----------------------------------------------------------------------------

void uvm_component::end_of_elaboration()
{}

//----------------------------------------------------------------------------
// member function: start_of_simulation (virtual)
//
//! Implementation defined - not part of UVM standard
//! note: directly called by SystemC kernel
//----------------------------------------------------------------------------

void uvm_component::start_of_simulation()
{}

//----------------------------------------------------------------------------
// member function: end_of_simulation (virtual)
//
//! Implementation defined - not part of UVM standard
//! note: directly called by SystemC kernel
//----------------------------------------------------------------------------

void uvm_component::end_of_simulation()
{}

//----------------------------------------------------------------------------
// member function: m_apply_verbosity_settings
//
//! Implementation defined
//----------------------------------------------------------------------------

void uvm_component::m_apply_verbosity_settings( uvm_phase* phase )
{
  // TODO
}

//----------------------------------------------------------------------------
// member function: m_set_run_handle
//
//! Implementation defined
//----------------------------------------------------------------------------

void uvm_component::m_set_run_handle( sc_process_handle h )
{
  m_run_handle = h;
}

//----------------------------------------------------------------------------
// member function: prepend_name
//
//! Implementation defined
//----------------------------------------------------------------------------

std::string uvm_component::prepend_name( const std::string& s )
{
  std::string n = name();
  if (!s.empty()) {
    n += std::string(".");
    n += s;
  }
  return n;
}


//----------------------------------------------------------------------------
// member function: m_add_child
//
//! Implementation defined
//----------------------------------------------------------------------------

bool uvm_component::m_add_child( uvm_component* child, const std::string& name )
{
  std::string chname;

  if (name.empty())
    chname = child->get_name();

  if ( ( m_children.find(chname) != m_children.end() ) // if exists
    && m_children[chname] != child )
  {
    std::ostringstream str;
    str << "A child with the name '" << chname
        << "' (type=" << m_children[chname]->get_type_name()
        << ") already exists.";
    uvm_report_warning("BDCHLD",str.str(), UVM_NONE);
    return false;
  }

  if (m_children_by_handle.find(child) != m_children_by_handle.end() ) // exists
  {
    std::ostringstream str;
    str << "A child with the name '" << chname
        << "' already exists in parent under name '"
        << m_children_by_handle[child]->get_name();
    uvm_report_warning("BDCHLD",str.str(), UVM_NONE);
    return false;
  }

  m_children[chname] = child;
  m_children_by_handle[child] = child;

  return true;
}

//----------------------------------------------------------------------------
// member function: kind (virtual, SystemC API)
//
//! Added for compatibility reasons
//----------------------------------------------------------------------------

const char* uvm_component::kind() const
{
  return "uvm::uvm_component";
}

//----------------------------------------------------------------------------
// member function: get_type_name
//
//! Implementation defined
//! Return the type name of this class - e.g., "my_component".
//! Inherits this pure virtual method from base class #uvm_typed.
//----------------------------------------------------------------------------

const std::string uvm_component::get_type_name() const
{
  return std::string(kind());
}


//----------------------------------------------------------------------------
// member function: m_set_full_name
//
//! Implementation defined
//----------------------------------------------------------------------------

void uvm_component::m_set_full_name()
{
  m_name = std::string(this->name()); //!use SystemC method
}


//----------------------------------------------------------------------------
// member function: m_extract_name
//
//! Implementation defined
//----------------------------------------------------------------------------

void uvm_component::m_extract_name( const std::string& name,
                                    std::string& leaf,
                                    std::string& remainder ) const
{
  unsigned int i, len;
  std::string extract_str;
  len = (int)name.length();

  for( i = 0; i < len; i++ )
  {
    if( name[i] == '.' )
      break;
  }

  if( i == len )
  {
    leaf = name;
    remainder = "";
    return;
  }

  leaf = name.substr( 0 , i );
  remainder = name.substr( i + 1 , len - i );

  return;
}

//----------------------------------------------------------------------------
// m_do_pre_abort
//
//! Implementation defined
//----------------------------------------------------------------------------

void uvm_component::m_do_pre_abort()
{
  for( m_children_mapcItT it = m_children.begin();
       it != m_children.end();
       it++ )
    (*it).second->m_do_pre_abort();
  pre_abort();
}


} // namespace uvm
