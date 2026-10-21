//----------------------------------------------------------------------
//   Copyright 2011 Cypress Semiconductor
//   Copyright 2010-2011 Mentor Graphics Corporation
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
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

#ifndef UVM_CONFIG_DB_H_
#define UVM_CONFIG_DB_H_

#include <string>
#include <vector>
#include <map>
#include <typeinfo>

#include "uvmsc/conf/uvm_resource.h"
#include "uvmsc/conf/uvm_queue.h"
#include "uvmsc/conf/uvm_resource_db.h"
#include "uvmsc/conf/uvm_config_db_options.h"
#include "uvmsc/base/uvm_globals.h"
#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"
#include "uvmsc/base/uvm_root.h"
#include "uvmsc/phasing/uvm_phase.h"

namespace uvm {


// forward class references
class uvm_root;

//----------------------------------------------------------------------
// Class: m_uvm_waiter
//
//! Implementation-defined class for config waiters
//----------------------------------------------------------------------

class m_uvm_waiter
{
 public:
  std::string inst_name;
  std::string field_name;
  sc_core::sc_event trigger;

  m_uvm_waiter( const std::string& inst_name_,
                const std::string& field_name_ ) :
    inst_name(inst_name_), field_name(field_name_)
  {}

  std::string convert2string() const
  {
    std::string str = "(" + inst_name + ", " + field_name + ")";
    return str;
  }

  friend std::ostream& operator<< ( std::ostream& os, const m_uvm_waiter& obj );

 private:
  // Disabled
  m_uvm_waiter( const m_uvm_waiter& );

}; // class m_uvm_waiter

inline std::ostream& operator<< ( std::ostream& os, const m_uvm_waiter& obj )
{
  os << obj.convert2string();
  return os;
}

//----------------------------------------------------------------------
// Class: uvm_config_db
//
//! The uvm_config_db class provides the basic interface
//! that is used for configuring instances derived from class uvm_component.
//----------------------------------------------------------------------

template <typename T>
class uvm_config_db : public uvm_resource_db<T>
{
 public:

  typedef std::map< uvm_component*, std::map< std::string, uvm_resource<T>* > > rsc_t;

  uvm_config_db();
  virtual ~uvm_config_db();

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  static void set( uvm_component* cntxt,
                   const std::string& inst_name,
                   const std::string& field_name,
                   const T& value );

  static bool get( uvm_component* cntxt,
                   const std::string& inst_name,
                   const std::string& field_name,
                   T& value );

  static bool exists( uvm_component* cntxt,
                      const std::string& inst_name,
                      const std::string& field_name,
                      bool spell_chk = false );

  static void wait_modified( uvm_component* cntxt,
                             const std::string& inst_name,
                             const std::string& field_name );

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

 private:

  static uvm_resource<T>* m_get_resource_match( uvm_component* cntxt,
                                                const std::string& field_name,
                                                const std::string& inst_name);


  // helper function to initialize static member
  static rsc_t init()
  {
    rsc_t m;
    return m;
  }

  static rsc_t& m_rsc_store()
  {
    return uvm_coreservice_t::get()->get_or_create_typed_store<rsc_t>(
      std::string("uvm_config_db::m_rsc:") + typeid(T).name(), init());
  }

  // Internal lookup of config settings so they can be reused
  // The context has a pool that is keyed by the inst/field name.
  typedef std::map< std::string, uvm_queue<m_uvm_waiter*>* > waiters_mapT;
  typedef waiters_mapT::iterator waiters_mapItT;

  static waiters_mapT& m_waiters_store()
  {
    return uvm_coreservice_t::get()->get_or_create_typed_store<waiters_mapT>(
      std::string("uvm_config_db::m_waiters:") + typeid(T).name(), waiters_mapT());
  }

  // Internal waiter list for wait_modified

}; // class uvm_config_db

//----------------------------------------------------------------------
// Convenience typedefs
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Typedef: uvm_config_int
//
// Convenience type for uvm_config_db#(uvm_bitstream_t)
//----------------------------------------------------------------------

typedef uvm_config_db<uvm_bitstream_t> uvm_config_int;

//----------------------------------------------------------------------
// Typedef: uvm_config_string
//
// Convenience type for uvm_config_db#(string)
//----------------------------------------------------------------------

typedef uvm_config_db<std::string> uvm_config_string;

//----------------------------------------------------------------------
// Typedef: uvm_config_object
//
// Convenience type for uvm_config_db#(uvm_object)
//----------------------------------------------------------------------

typedef uvm_config_db<uvm_object*> uvm_config_object;

//----------------------------------------------------------------------
// Typedef: uvm_config_wrapper
//
// Convenience type for uvm_config_db#(uvm_object_wrapper)
//----------------------------------------------------------------------

typedef uvm_config_db<uvm_object_wrapper*> uvm_config_wrapper;


//----------------------------------------------------------------------
// Initialization of static data members
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: set (static)
//
//! Create a new or update an existing configuration setting for
//! \p field_name in \p inst_name from \p cntxt.
//! The setting is made at \p cntxt, with the full name of \p cntxt
//! added to the inst_name. If \p cntxt is nullptr then \p inst_name
//! provides the complete scope information of the setting.
//! \p field_name is the target field. Both \p inst_name and \p field_name
//! may be glob style or regular expression style expressions.
//----------------------------------------------------------------------

template <typename T>
void uvm_config_db<T>::set( uvm_component* cntxt,
                            const std::string& inst_name,
                            const std::string& field_name,
                            const T& value )
{
    uvm_root* top = nullptr;
    uvm_phase* curr_phase = nullptr;
    uvm_resource<T>* r = nullptr;
    uvm_coreservice_t* cs = nullptr;

    bool exists = false;
    std::string loc_instname = inst_name; // local instance name, input is const
    std::string setstr = "set";

    // TODO
    //take care of random stability during allocation
    //process p = process::self();
    //string rstate = p.get_randstate();

    cs = uvm_coreservice_t::get();
    top = cs->get_root();

    curr_phase = top->m_current_phase;

    if( cntxt == nullptr )
      cntxt = top;
    if( inst_name.empty() )
      loc_instname = cntxt->get_full_name();
    else
    {
      if( (!cntxt->get_full_name().empty()) && (cntxt != top) )
        loc_instname = cntxt->get_full_name() + "." + inst_name;
    }
    r = m_get_resource_match(cntxt, field_name, loc_instname);

    if(r == nullptr)
    {
      std::map< std::string, uvm_resource<T>* > pool;
      std::string key = loc_instname+field_name;
      m_rsc_store()[cntxt] = pool;
      r = new uvm_resource<T>(field_name, loc_instname);
      pool[key] = r;
    }
    else
    {
      exists = true;
      setstr +=" (override)";
    }

    if(curr_phase != nullptr && curr_phase->get_name() == "build")
      r->precedence = uvm_resource_base::default_precedence_ref() - (cntxt->get_depth());
    else
      r->precedence = uvm_resource_base::default_precedence_ref();

    uvm_object* obj = dynamic_cast<uvm_object*>(cntxt);
    r->write(value, obj);

    if(exists)
    {
      uvm_resource_pool* rp = uvm_resource_pool::get();
      rp->set_priority_name(r, uvm_resource_types::PRI_HIGH);
    }
    else
    {
      // Doesn't exist yet, so put it in resource db at the head.
      r->set_override();
    }

    if(m_waiters_store().find(field_name) != m_waiters_store().end()) // if exists
    {
      m_uvm_waiter* w;
      for(int i = 0; i < m_waiters_store()[field_name]->size(); i++)
      {
        w = m_waiters_store()[field_name]->get(i);
        if( uvm_re_match( uvm_glob_to_re(inst_name), w->inst_name ) )
          w->trigger.notify(sc_core::SC_ZERO_TIME);
      }
    }

    // TODO
    //p.set_randstate(rstate);

    if(uvm_config_db_options::is_tracing())
      uvm_resource_db<T>::m_show_msg("CFGDB/SET", "Configuration", setstr, inst_name, field_name, dynamic_cast<uvm_object*>(cntxt), r);

}

//----------------------------------------------------------------------
// member function: get
//
//! Get the value for \p field_name in \p inst_name, using component \p cntxt as
//! the starting search point. \p inst_name is an explicit instance name
//! relative to \p cntxt and may be an empty string if the \p cntxt is the
//! instance that the configuration object applies to. \p field_name
//! is the specific field in the scope that is being searched for.
//----------------------------------------------------------------------

template <typename T>
bool uvm_config_db<T>::get( uvm_component* cntxt,
                            const std::string& inst_name,
                            const std::string& field_name,
                            T& value )

{
  uvm_resource<T>* r = nullptr;
  uvm_resource_pool* rp = uvm_resource_pool::get();
  uvm_resource_types::rsrc_q_t* rq = nullptr;
  uvm_root* top = nullptr;
  uvm_coreservice_t* cs = nullptr;

  std::string loc_instname = inst_name;  // local instance name, input is const

  cs = uvm_coreservice_t::get();
  top = cs->get_root();

  if( cntxt == nullptr )
    cntxt = top;
  if( inst_name.empty() )
    loc_instname = cntxt->get_full_name();
  else
    if( (!cntxt->get_full_name().empty()) && (cntxt != top) )
      loc_instname = cntxt->get_full_name() + "." + inst_name;

  rq = rp->lookup_regex_names(loc_instname, field_name, uvm_resource<T>::get_type() );
  r = uvm_resource<T>::get_highest_precedence(rq);

  if(r == nullptr)
  {
    /* TODO make part of debug
    std::ostringstream msg;
    msg << "Resource '" << field_name
        << "' not found in configuration database using instance path '"
        << loc_instname << "'. Returning without value update.";
    uvm_report_warning("CFGDB/GET", msg.str(), UVM_NONE);
    */
    return false;
  }

  if(uvm_config_db_options::is_tracing())
    uvm_resource_db<T>::m_show_msg("CFGDB/GET", "Configuration", "read", loc_instname, field_name, dynamic_cast<uvm_object*>(cntxt), r);

  uvm_object* obj = dynamic_cast<uvm_object*>(cntxt);
  value = r->read(obj);

  return true;
}

//----------------------------------------------------------------------
// member function: exists
//
//! Check if a value for \p field_name is available in \p inst_name, using
//! component \p cntxt as the starting search point. \p inst_name is an explicit
//! instance name relative to \p cntxt and may be an empty string if the
//! \p cntxt is the instance that the configuration object applies to.
//! \p field_name is the specific field in the scope that is being searched for.
//! The \p spell_chk argument can be set to true to turn spell checking on if it
//! is expected that the field should exist in the database. The function
//! returns true if a config parameter exists and false if it doesn't exist.
//----------------------------------------------------------------------

template <typename T>
bool uvm_config_db<T>::exists( uvm_component* cntxt,
                               const std::string& inst_name,
                               const std::string& field_name,
                               bool spell_chk )
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();

  std::string loc_inst_name = inst_name;
  if(cntxt == nullptr)
    cntxt = cs->get_root();

  if(inst_name.empty())
    loc_inst_name = cntxt->get_full_name();
  else
    if(!cntxt->get_full_name().empty())
      loc_inst_name = cntxt->get_full_name() + "." + inst_name;

  return (uvm_resource_db<T>::get_by_name(loc_inst_name, field_name, spell_chk) != nullptr);
}

//----------------------------------------------------------------------
// member function: wait_modified (static)
//
//! Wait for a configuration setting to be set for \p field_name
//! in \p cntxt and \p inst_name. The task blocks until a new configuration
//! setting is applied that effects the specified field.
//----------------------------------------------------------------------

template <typename T>
void uvm_config_db<T>::wait_modified( uvm_component* cntxt,
                                      const std::string& inst_name,
                                      const std::string& field_name )
{
  // TODO process
  //  process p = process::self();
  //  string rstate = p.get_randstate();
  uvm_coreservice_t* cs = uvm_coreservice_t::get();

  m_uvm_waiter* waiter = nullptr;
  std::string loc_inst_name = inst_name;

  if(cntxt == nullptr)
    cntxt = cs->get_root();

  if(cntxt != cs->get_root())
  {
    if(!inst_name.empty())
      loc_inst_name = cntxt->get_full_name() + "." + inst_name;
    else
      loc_inst_name = cntxt->get_full_name();
  }

  waiter = new m_uvm_waiter(loc_inst_name, field_name);

  if(m_waiters_store().find(field_name) == m_waiters_store().end() ) // not exist
    m_waiters_store()[field_name] = new uvm_queue<m_uvm_waiter*>;

  m_waiters_store()[field_name]->push_back(waiter);

  //p.set_randstate(rstate); // TODO randstate

  // wait on the waiter to trigger
  sc_core::wait(waiter->trigger);

  // Remove the waiter from the waiter list
  for(int i = 0; i < m_waiters_store()[field_name]->size(); ++i)
  {
     if(m_waiters_store()[field_name]->get(i) == waiter)
     {
       m_waiters_store()[field_name]->do_delete(i); // TODO also delete from memory?
       break;
     }
  }
}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------
// member function: m_get_resource_match
//
//! Implementation-defined member function
//----------------------------------------------------------------------

template <typename T>
uvm_resource<T>* uvm_config_db<T>::m_get_resource_match( uvm_component* cntxt,
                                                         const std::string& field_name,
                                                         const std::string& inst_name)
{
  std::map< std::string, uvm_resource<T>* > pool;
  std::string lookup;

  if( m_rsc_store().find(cntxt) == m_rsc_store().end() )
    return nullptr;

  lookup = inst_name+field_name;
  pool = m_rsc_store()[cntxt];

  if(pool.find(lookup) == pool.end()) return nullptr;

  return (pool.find(lookup))->second;
}

} // namespace uvm

#endif // UVM_CONFIG_DB_H_
