//----------------------------------------------------------------------
//   Copyright 2012-2022 NXP B.V.
//   Copyright 2014-2017 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Synopsys, Inc.
//   Copyright 2018 Robert Bosch GmbH
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

#include <cstdlib>
#include <stdexcept>

#include "uvmsc/base/uvm_root.h"
#include "uvmsc/base/uvm_component.h"
#include "uvmsc/base/uvm_component_name.h"
#include "uvmsc/base/uvm_globals.h"
#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"
#include "uvmsc/macros/uvm_message_defines.h"
#include "uvmsc/phasing/uvm_phase.h"
#include "uvmsc/phasing/uvm_common_phases.h"
#include "uvmsc/phasing/uvm_objection.h"
#include "uvmsc/comps/uvm_test.h"
#include "uvmsc/print/uvm_printer.h"
#include "uvmsc/print/uvm_table_printer.h"
#include "uvmsc/print/uvm_printer_globals.h"
#include "uvmsc/report/uvm_report_handler.h"
#include "uvmsc/report/uvm_report_server.h"
#include "uvmsc/conf/uvm_resource_pool.h"

using namespace sc_core;

// TODO find a way to align version information for Linux and Windows build
#if defined(_MSC_VER)
#include "uvmsc/base/uvm_version.h"
#else
#include "config.h"
#endif


namespace uvm {

//----------------------------------------------------------------------
// CLASS: uvm_root
//
// Implementation
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Initialization static data members
//----------------------------------------------------------------------


bool uvm_root::m_uvm_timeout_overridable = true;

//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------

uvm_root::uvm_root( uvm_component_name nm )
: uvm_component(nm)
{
  m_enable_print_topology = false;
  m_finish_on_completion = true;
  phase_timeout = UVM_DEFAULT_TIMEOUT;
  m_phase_all_done = false;
  m_current_phase = NULL;

  m_uvm_timeout_overridable = true;

  m_rh->set_name("reporter");

  phases_registered = false;

  // create resource pool
  uvm_resource_pool::get();

  // Launch factory service
  //uvm_coreservice_t* cs = uvm_coreservice_t::get();
  //cs->get_factory();

  // print header
  m_uvm_header();
}

//----------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------

uvm_root::~uvm_root()
{
  // delete resource pool
  uvm_resource_pool::cleanup();
}

//----------------------------------------------------------------------
// Group: Simulation control
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: run_test
//
//! Will register the test specified as argument using the factory.
//----------------------------------------------------------------------

void uvm_root::run_test( const std::string& test_name )
{
  // disable SystemC messages
  // TODO make switch available to enable SystemC messages
  sc_core::sc_report_handler::set_actions("/OSCI/SystemC", sc_core::SC_DO_NOTHING);

  // check and register test object
  m_register_test(test_name);

  // start objection mechanism fired as spawned process
  sc_process_handle m_init_objections_proc =
    sc_spawn(sc_bind(&uvm_objection::m_init_objections),
      "m_init_objections_proc");

  uvm_phase::m_register_phases();
  phases_registered = true;

  // call build and connect phase
  uvm_phase::m_prerun_phases();


  // start simulation (of run-phases) here.
  try
  {
    sc_core::sc_start();
  }
  catch( const std::exception& e )
  {
    std::cerr << e.what() << std::endl;
    exit(1); // TODO exit program with error code?
  }

  // Stop simulation when phasing is completed and finish_on_completion is set
  if ( m_finish_on_completion && (sc_core::sc_get_status() == sc_core::SC_RUNNING ||
      sc_core::sc_get_status() == sc_core::SC_PAUSED) )
    sc_core::sc_stop();

  // Clean-up 
  m_unregister_test(test_name);
}

//----------------------------------------------------------------------
// member function: die (virtual)
//
//! This method is called by the report server if a report reaches the maximum
//! quit count or has an UVM_EXIT action associated with it, e.g., as with
//! fatal errors.
//!
//! Calls the member function uvm_component::pre_abort()
//! on the entire #uvm_component hierarchy in a bottom-up fashion.
//! It then call calls #report_summarize and terminates the simulation
//! with sc_stop().
//----------------------------------------------------------------------

void uvm_root::die()
{
  uvm_report_server* l_rs = uvm_report_server::get_server();
  // do the pre_abort callbacks
  m_do_pre_abort();

  l_rs->report_summarize();
  throw std::runtime_error("Simulation terminated by uvm_root::die()");
}


//----------------------------------------------------------------------
// member function: set_timeout
//
//! Specifies the timeout for the simulation. Default is <`UVM_DEFAULT_TIMEOUT>
//!
//! The timeout is simply the maximum absolute simulation time allowed before a
//! ~FATAL~ occurs.  If the timeout is set to 20ns, then the simulation must end
//! before 20ns, or a ~FATAL~ timeout will occur.
//
//! This is provided so that the user can prevent the simulation from potentially
//! consuming too many resources (Disk, Memory, CPU, etc) when the testbench is
//! essentially hung.
//----------------------------------------------------------------------

void uvm_root::set_timeout( const sc_core::sc_time& timeout, bool overridable)
{
  if (!m_uvm_timeout_overridable)
  {
    std::ostringstream str;
    str << "The global timeout setting of "
        << phase_timeout
        << " is not overridable to "
        << timeout
        << " due to a previous setting.";
    uvm_report_info("NOTIMOUTOVR", str.str(), UVM_NONE);
    return;
  }
  m_uvm_timeout_overridable = overridable;
  phase_timeout = timeout;
}

//----------------------------------------------------------------------
// member function: set_finish_on_completion
//
//! The member function set_finish_on_completion shall define how
//! simulation is finalized. If the application did not call this member
//! function or if the argument enable is set to true, it shall execute
//! all end_of_simulation callbacks of the UVM components involved and
//! finish the simulation. If the argument enable is set to false, the
//! simulation shall be finalized without calling the end_of_simulation
//! callbacks.
//! NOTE: An implementation may call the function sc_core::sc_stop as part
//! of the finish_on_completion implementation to enforce finalization of
//! the simulation following the SystemC execution semantics.
//----------------------------------------------------------------------

void uvm_root::set_finish_on_completion( bool enable )
{
  m_finish_on_completion = enable;
}

//----------------------------------------------------------------------
// member function: get_finish_on_completion
//
//! The member function get_finish_on_completion shall return true if the
//! application has not called member function set_finish_on_completion
//! or if the member function was called with the argument enable as true;
//! otherwise it shall return false.
//----------------------------------------------------------------------

bool uvm_root::get_finish_on_completion()
{
  return m_finish_on_completion;
}

//----------------------------------------------------------------------
// Group: Topology
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: find
//
//! Returns the (first) component which name matches the string passed
//! as argument. It will return NULL if no component has been found.
//----------------------------------------------------------------------

uvm_component* uvm_root::find( const std::string& comp_match )
{
  std::vector<uvm_component*> comp_list;

  find_all(comp_match, comp_list);

  if (comp_list.size() > 1)
  {
    std::ostringstream str;
    str << "Found " << comp_list.size() << " components matching '"
        << comp_match << "'. Returning first match, "
        << comp_list[0]->get_full_name() << ".";
    uvm_report_warning("MMATCH", str.str(), UVM_NONE);
  }

  if (comp_list.size() == 0)
  {
    uvm_report_warning("CMPNFD",
      "Component matching '" + comp_match +
       "' was not found in the list of uvm_components", UVM_NONE);
    return NULL;
  }

  return comp_list[0];
}

//----------------------------------------------------------------------
// member function: find_all
//
//! Returns the component handle (find) or list of components handles
//! (find_all) matching a given string. The string may contain the wildcards,
//! * and ?. Strings beginning with '.' are absolute path names. If optional
//! comp arg is provided, then search begins from that component down
//! (default=all components).
//----------------------------------------------------------------------

void uvm_root::find_all( const std::string& comp_match, std::vector<uvm_component*>& comps,
                         uvm_component* comp )
{
  uvm_component* c = comp;

  if (c == NULL)
    c = this;

  m_find_all_recurse(comp_match, comps, c);
}

//----------------------------------------------------------------------
// member function: print_topology
//
//! Print the verification environment's component topology. The
//! printer is a uvm_printer object that controls the format
//! of the topology printout; a null printer prints with the
//! default output.
//----------------------------------------------------------------------

void uvm_root::print_topology( uvm_printer* printer )
{
  std::string s;

  if (printer == NULL)
    printer = uvm_default_printer;

  if (printer == NULL)
    uvm_report_error("NULLPRINTER", "uvm_default_printer is NULL");

  if (m_children.size() == 0)
  {
    uvm_report_warning("EMTCOMP", "print_topology - No UVM components to print.", UVM_NONE);
    return;
  }

  for (m_children_mapcItT it = m_children.begin();
       it != m_children.end();
       it ++)
  {
    if(it->second->print_enabled)
      printer->print_object("", *(it->second));
  }

  UVM_INFO("UVMTOP", "UVM testbench topology:\n" + printer->emit(), UVM_NONE);
}

//----------------------------------------------------------------------
// member function: enable_print_topology
//
//! The member function enable_print_topology shall print the entire
//! testbench topology just after completion of the end_of_elaboration
//! phase, if enabled. By default, the testbench topology is not printed,
//! unless enabled by the application by calling this member function.
//----------------------------------------------------------------------

void uvm_root::enable_print_topology( bool enable )
{
  m_enable_print_topology = enable;
}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------
// member function: before_end_of_elaboration
//
//! Called by SystemC before the end of elaboration
//----------------------------------------------------------------------

void uvm_root::before_end_of_elaboration()
{
}

//----------------------------------------------------------------------
// member function: end_of_elaboration
//
//! Called by SystemC when elaboration is completed
//----------------------------------------------------------------------

void uvm_root::end_of_elaboration()
{
  if (phases_registered)
    uvm_phase::m_run_single_phase("end_of_elaboration");
}

//----------------------------------------------------------------------
// member function: start_of_simulation
//
//! Called by SystemC when simulation is started
//----------------------------------------------------------------------

void uvm_root::start_of_simulation()
{
  if (phases_registered)
    uvm_phase::m_run_phases_process(); // this includes all run phases
}

//----------------------------------------------------------------------
// member function: end_of_simulation
//
//! Called by SystemC when simulation is ended
//----------------------------------------------------------------------

void uvm_root::end_of_simulation()
{
  if (phases_registered)
    uvm_report_info("FINISH", "UVM-SystemC phasing completed; simulation finished", UVM_NONE);
}

//----------------------------------------------------------------------
// member function: m_uvm_get_root (static)
//
//! Implementation defined
//! Get the initialized singleton instance of uvm_root
//----------------------------------------------------------------------

uvm_root* uvm_root::m_uvm_get_root()
{
  static uvm_root* m_root = NULL;

  if (m_root == NULL)
  {
    m_root = new uvm_root(sc_core::sc_module_name("uvm_top"));
    m_root->m_domain = uvm_domain::get_uvm_domain();
  }

  return m_root;
}

//----------------------------------------------------------------------
// member function: get (static)
//
//! Implementation defined
//! Get the uvm_root singleton if it exists, otherwise create it
//----------------------------------------------------------------------

uvm_root* uvm_root::get()
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  return cs->get_root();
}

//----------------------------------------------------------------------
// member function: m_find_all_recurse
//
//! Implementation defined
//----------------------------------------------------------------------

void uvm_root::m_find_all_recurse( const std::string& comp_match, std::vector<uvm_component*>& comps,
                                   uvm_component* comp )
{
  std::string name;

  if (comp->get_first_child(name))
    do
    {
      m_find_all_recurse(comp_match, comps, comp->get_child(name));
    }
    while (comp->get_next_child(name));

  if (uvm_is_match(comp_match, comp->get_full_name()) &&
      !comp->get_name().empty()) /* uvm_top */
    comps.push_back(comp);
}

//----------------------------------------------------------------------
// member function: m_register_test
//
//! Implementation defined
//----------------------------------------------------------------------

void uvm_root::m_register_test( const std::string& test_name )
{
  std::vector<uvm_component*> comp_list;
  uvm_component* uvm_test_top = NULL;

  if (test_name.size() != 0)
  {
    find_all(test_name, comp_list);

    if (comp_list.size() != 0)
    {
      std::ostringstream msg;
      msg << "Object with name '" << test_name
          << "' has already been registered and instantiated in the design hierarchy.";
            uvm_report_warning("RUNTST", msg.str(), UVM_NONE);
    }
  }

  if ( (comp_list.size() == 0) && (test_name.size() != 0) )
  {
    uvm_coreservice_t* cs = uvm_coreservice_t::get();
    uvm_factory* factory = cs->get_factory();

    uvm_test_top = factory->create_component_by_name(
      test_name, "", test_name, NULL);

    if (uvm_test_top == NULL)
    {
      std::ostringstream msg2;
      msg2 << "No UVM test available with name '" << test_name
          << "'. Simulation will be aborted.";
      uvm_report_fatal("RUNTST", msg2.str(), UVM_NONE);
    }
    else
    {
      //TODO test in hierarchy
      //top->m_add_child(uvm_test_top, test_name);
      //std::cout << "test added to hierarchy" << std::endl;
    }
  }

  if(test_name=="")
    uvm_report_info("RNTST", "Running test ...", UVM_LOW);
  else
    if (test_name == uvm_test_top->get_type_name())
      uvm_report_info("RNTST", "Running test " + test_name + "...", UVM_LOW);
    else
      uvm_report_info("RNTST", "Running test " + uvm_test_top->get_type_name() +
        " (via factory override for test '" + test_name + "')...", UVM_LOW);
}

//----------------------------------------------------------------------
// member function: m_uvm_header
//
//! Print header information
//----------------------------------------------------------------------

void uvm_root::m_uvm_header()
{
  static bool lnp = false;
  if (lnp  || (getenv("UVM_SYSTEMC_DISABLE_COPYRIGHT_MESSAGE") != 0 )) {
	  lnp = true;
  } else
  {
#ifdef REVISION
    std::string revision(REVISION);
#else
    std::string revision;
#endif

    std::string::size_type br;
    while ((br = revision.find("$")) != std::string::npos)
      revision.erase(br, 1);
    while ((br = revision.find("Rev:")) != std::string::npos)
      revision = revision.substr(br + 4);

    std::cerr << std::endl << std::endl;
    std::cerr << "   Universal Verification Methodology for SystemC (UVM-SystemC)"
        << std::endl
        << "              Version: " << UVM_VERSION << "  Date: " << UVM_RELEASE_DATE << std::endl;
    std::cerr
        << "          Copyright (c) 2006 - 2023 by all Contributors" << std::endl
        << "            See NOTICE file for all Contributors"  << std::endl
        << "                    ALL RIGHTS RESERVED" << std::endl;
    std::cerr
        << "         Licensed under the Apache License, Version 2.0"
        << std::endl;
    std::cerr << std::endl << std::endl;

    //  regressions check point
    if( std::getenv( "UVMSC_REGRESSION" ) != 0 )
      std::cerr << "UVMSC_REGRESSION_STARTED" << std::endl;

    lnp = true;
  }
}

//----------------------------------------------------------------------
// member function: get_phase_all_done
//
//! Implementation defined
//----------------------------------------------------------------------

bool uvm_root::get_phase_all_done()
{
  return m_phase_all_done;
}

//----------------------------------------------------------------------
// member function: m_unregister_test
//
//! Implementation defined
//----------------------------------------------------------------------

void uvm_root::m_unregister_test( const std::string& test_name )
{
  std::vector<uvm_component*> comp_list;

  if (test_name.size() != 0)
  {
    find_all(test_name, comp_list);

    if (comp_list.size() != 0)
    {
      uvm_coreservice_t* cs = uvm_coreservice_t::get();
      uvm_factory* factory = cs->get_factory();
      for(std::vector<uvm_component*>::iterator 
          it = comp_list.begin(); 
          it != comp_list.end(); 
          ++it)
        factory->m_delete_component(*it);
    }
  }
}

} // namespace uvm

