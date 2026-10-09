//----------------------------------------------------------------------
//   Copyright 2011 Cypress Semiconductor
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2011 Cadence Design Systems, Inc.
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2013-2014 Fraunhofer-Gesellschaft zur Foerderung
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

#include "uvmsc/conf/uvm_resource_base.h"
#include "uvmsc/conf/uvm_resource_pool.h"
#include "uvmsc/conf/uvm_resource_types.h"
#include "uvmsc/conf/uvm_resource_options.h"
#include "uvmsc/misc/uvm_misc.h"
#include "uvmsc/base/uvm_globals.h"
#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"
#include "uvmsc/print/uvm_line_printer.h"
#include "uvmsc/macros/uvm_message_defines.h"
#include "uvmsc/macros/uvm_string_defines.h"


namespace uvm {

namespace {

bool& m_has_wildcard_names_ref()
{
  return uvm_coreservice_t::get()->get_uvm_resource_pool_m_has_wildcard_names();
}

} // namespace


//----------------------------------------------------------------------
// class uvm_resource_pool implementation
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// initialize static members
//----------------------------------------------------------------------

// Refactored from the former uvm_resource_pool::rp and
// uvm_resource_pool::m_has_wildcard_names globals to uvm_coreservice_t.

//----------------------------------------------------------------------
// constructor
//----------------------------------------------------------------------

uvm_resource_pool::uvm_resource_pool()
{
  // empty
}

//----------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------

uvm_resource_pool::~uvm_resource_pool()
{
  for( rtab_mapItT
       it = rtab.begin();
       it != rtab.end();
       it++ )
    delete it->second;

  for( ttab_mapItT
       it = ttab.begin();
       it != ttab.end();
       it++ )
  {
    delete it->first;
    delete it->second;
  }

  for( get_record_list_cItT
       it = get_record.begin();
       it != get_record.end();
       it++ )
    delete (*it);

  while(!rsrc_list.empty())
    delete rsrc_list.back(), rsrc_list.pop_back();
}

//----------------------------------------------------------------------
// member function: get
//
//! Return singleton to resource pool
//----------------------------------------------------------------------

uvm_resource_pool* uvm_resource_pool::get()
{
  return uvm_coreservice_t::get()->get_uvm_resource_pool_rp();
}

//----------------------------------------------------------------------
// member function: spell_check
//
//! Invokes the spell checker for a string s.  The universe of
//! correctly spelled strings -- i.e. the dictionary -- is the name
//! map.
//----------------------------------------------------------------------

bool uvm_resource_pool::spell_check( const std::string& s ) const
{
  // TODO spelling checker
  //return uvm_spell_chkr<uvm_resource_types::rsrc_q_t>::check(rtab, s);
  return false; //dummy for now
}

//----------------------------------------------------------------------
// member function: set
//
//! Add a new resource to the resource pool. The resource is inserted
//! into both the name map and type map so it can be located by
//! either.
//----------------------------------------------------------------------

void uvm_resource_pool::set( uvm_resource_base* rsrc,
                             uvm_resource_types::override_t override )
{
  uvm_resource_types::rsrc_q_t* rq;
  std::string name;
  uvm_resource_base* type_handle;

  // If resource handle is null then there is nothing to do.
  if(rsrc == nullptr)
    return;

  // insert into the name map.  Resources with empty names are
  // anonymous resources and are not entered into the name map

  name = rsrc->get_name();
  if(!name.empty())
  {
    if ( rtab.find(name) != rtab.end() )
      rq = rtab[name];
    else
      rq = new uvm_resource_types::rsrc_q_t();

    // Insert the resource into the queue associated with its name.
    // If we are doing a name override then insert it in the front of
    // the queue, otherwise insert it in the back.

    if( override & uvm_resource_types::NAME_OVERRIDE )
      rq->push_front(rsrc);
    else
      rq->push_back(rsrc);

    rtab[name] = rq;
  }

  // insert into the type map
  type_handle = rsrc->get_type_handle();

  if( ttab.find(type_handle) != ttab.end() )
    rq = ttab[type_handle];
  else
    rq = new uvm_resource_types::rsrc_q_t();

  // insert the resource into the queue associated with its type.  If
  // we are doing a type override then insert it in the front of the
  // queue, otherwise insert it in the back of the queue.

  if( override & uvm_resource_types::TYPE_OVERRIDE )
    rq->push_front(rsrc);
  else
    rq->push_back(rsrc);

  ttab[type_handle] = rq;

  //optimization for name lookups. Since most environments never
  //use wildcarded names, don't want to incurr a search penalty
  //unless a wildcarded name has been used.

  if(rsrc->m_is_regex_name)
    m_has_wildcard_names_ref() = true;
}

//----------------------------------------------------------------------
// member function: set_override
//
//! The resource provided as an argument will be entered into the pool
//! and will override both by name and type.
//----------------------------------------------------------------------

 void uvm_resource_pool::set_override( uvm_resource_base* rsrc )
 {
   // logic or will set value to 1 --> only TYPE_OVERRIDE; changed back to bitwise or (solves also warning with gcc 8.2)
   // TODO should this be split in set_name_override and set_type_override calls?
   set( rsrc, (uvm_resource_types::NAME_OVERRIDE | uvm_resource_types::TYPE_OVERRIDE) );
 }

//----------------------------------------------------------------------
// member function: set_name_override
//
//! The resource provided as an argument will entered into the pool
//! using normal precedence in the type map and will override the name.
//----------------------------------------------------------------------

void uvm_resource_pool::set_name_override( uvm_resource_base* rsrc )
{
  set( rsrc, uvm_resource_types::NAME_OVERRIDE );
}

//----------------------------------------------------------------------
// member function: set_type_override
//
//! The resource provided as an argument will be entered into the pool
//! using noraml precedence in the name map and will override the type.
//----------------------------------------------------------------------

void uvm_resource_pool::set_type_override( uvm_resource_base* rsrc )
{
  set( rsrc, uvm_resource_types::TYPE_OVERRIDE );
}


//----------------------------------------------------------------------
// member function: push_get_record
//
//! Insert a new record into the get history list.
//----------------------------------------------------------------------

void uvm_resource_pool::push_get_record( const std::string& name,
                                         const std::string& scope,
                                         uvm_resource_base* rsrc )
{
  get_t* impt;

  // if auditing is turned off then there is no reason
  // to save a get record
  if( !uvm_resource_options::is_auditing() )
    return;

  impt = new get_t();

  impt->name  = name;
  impt->scope = scope;
  impt->rsrc  = rsrc;
  impt->t     = sc_core::sc_time_stamp();

  get_record.push_back(impt);
}

//----------------------------------------------------------------------
// member function: dump_get_records
//
//! Format and print the get history list.
//----------------------------------------------------------------------

void uvm_resource_pool::dump_get_records() const
{
  get_t* record;
  bool success;
  std::vector<std::string> qs;

  qs.push_back("--- resource get records ---\n");

  for ( get_record_list_cItT
        it = get_record.begin();
        it != get_record.end();
        it++ )
  {
    record = (*it);
    success = (record->rsrc != nullptr);

    std::ostringstream str;
    str << "get: name=" << record->name
        << " scope=" << record->scope
        << " " << ((success)?"success":"fail")
        << " @" << record->t << "\n";
    qs.push_back(str.str());
  }

  UVM_INFO("UVM/RESOURCE/GETRECORD", UVM_STRING_QUEUE_STREAMING_PACK(qs), UVM_NONE);
}

//----------------------------------------------------------------------
// member function: lookup_name
//
//! Lookup resources by \p name. Returns a queue of resources that
//! match the \p name, \p scope, and \p type_handle.  If no resources
//! match the queue is returned empty. If \p rpterr is set then a
//! warning is issued if no matches are found, and the spell checker is
//! invoked on \p name.  If \p type_handle is null then a type check is
//! not made and resources are returned that match only \p name and
//! \p scope.
//----------------------------------------------------------------------

uvm_resource_types::rsrc_q_t* uvm_resource_pool::lookup_name(
  const std::string& scope,
  const std::string& name,
  uvm_resource_base* type_handle,
  bool rpterr) const
{
  uvm_resource_types::rsrc_q_t* rq;
  uvm_resource_types::rsrc_q_t* q = new uvm_resource_types::rsrc_q_t();
  // uvm_resource_base* rsrc;
  uvm_resource_base* r;

  // resources with empty names are anonymous and do not exist in the name map
  if(name.empty())
    return q;

  // Does an entry in the name map exist with the specified name?
  // If not, then we're done
  if((rpterr && !spell_check(name)) || (!rpterr && (rtab.find(name)==rtab.end()) ))
    return q;

  rq = rtab.find(name)->second; // rq = rtab[name];

  for(int i = 0; i < (int)rq->size(); i++)
  {
    r = rq->get(i);
    // does the scope and type match?
    if( r->match_scope(scope) &&
        ((type_handle == nullptr) || (r->get_type_handle() == type_handle)))
      q->push_back(r);
  }

  return q;
}

//----------------------------------------------------------------------
// member function: get_highest_precedence
//
//! Traverse a queue, \p q, of resources and return the one with the highest
//! precedence.  In the case where there exists more than one resource
//! with the highest precedence value, the first one that has that
//! precedence will be the one that is returned.
//----------------------------------------------------------------------

// TODO: use reference as argument, not pointer
uvm_resource_base* uvm_resource_pool::get_highest_precedence( uvm_resource_types::rsrc_q_t* q ) const
{
  uvm_resource_base* rsrc;
  uvm_resource_base* r;
  unsigned int prec;

  if(q->size() == 0)
    return nullptr;

  // get the first resources in the queue
  rsrc = q->get(0);
  prec = rsrc->precedence;

  // start searching from the second resource
  for(int i = 1; i < q->size(); i++)
  {
    r = q->get(i);
    if(r->precedence > prec)
    {
      rsrc = r;
      prec = r->precedence;
    }
  }

  return rsrc;
}

//----------------------------------------------------------------------
// member function: sort_by_precedence
//
//! Given a list of resources, obtained for example from #lookup_scope,
//! sort the resources in  precedence order. The highest precedence
//! resource will be first in the list and the lowest precedence will
//! be last. Resources that have the same precedence and the same name
//! will be ordered by most recently set first.
//----------------------------------------------------------------------

// TODO: use reference as argument, not pointer
void uvm_resource_pool::sort_by_precedence( uvm_resource_types::rsrc_q_t* q)
{
  typedef std::map<int,uvm_resource_types::rsrc_q_t* > all_listT;
  typedef all_listT::iterator all_listItT;
  all_listT all;

  uvm_resource_base* r;

  for(int i=0; i<q->size(); ++i)
  {
    r = q->get(i);
    if( all.find(r->precedence) == all.end() )
       all[r->precedence] = new uvm_resource_types::rsrc_q_t();

    all[r->precedence]->push_front(r); // since we will push_front in the final
  }

  q->do_delete();

  for( all_listItT it = all.begin(); it != all.end(); it++ )
  {
    for(int j = 0; j<(*it).second->size(); j++)
    {
      r = (*it).second->get(j);
      q->push_front(r);
    }
  }
}

//----------------------------------------------------------------------
// member function: get_by_name
//
//! Lookup a resource by \p name, \p scope, and \p type_handle.  Whether
//! the get succeeds or fails, save a record of the get attempt.  The
//! \p rpterr flag indicates whether to report errors or not.
//! Essentially, it serves as a verbose flag.  If set then the spell
//! checker will be invoked and warnings about multiple resources will
//! be produced.
//----------------------------------------------------------------------

uvm_resource_base* uvm_resource_pool::get_by_name( const std::string& scope,
                                                   const std::string& name,
                                                   uvm_resource_base* type_handle,
                                                   bool rpterr )
{
  uvm_resource_types::rsrc_q_t* q;
  uvm_resource_base* rsrc;

  q = lookup_name(scope, name, type_handle, rpterr);

  if(q->size() == 0)
  {
    push_get_record(name, scope, nullptr);
    return nullptr;
  }

  rsrc = get_highest_precedence(q);
  push_get_record(name, scope, rsrc);
  return rsrc;
}

//----------------------------------------------------------------------
// member function: lookup_type
//
//! Lookup resources by type. Return a queue of resources that match
//! the \p type_handle and \p scope. If no resources match then the returned
//! queue is empty.
//----------------------------------------------------------------------

uvm_resource_types::rsrc_q_t* uvm_resource_pool::lookup_type( const std::string& scope,
                                                              uvm_resource_base* type_handle) const
{
  uvm_resource_types::rsrc_q_t* q = new uvm_resource_types::rsrc_q_t();
  uvm_resource_types::rsrc_q_t* rq;
  uvm_resource_base* r;

  if(type_handle == nullptr || (ttab.find(type_handle)==ttab.end()) )
    return q;

  rq = ttab.find(type_handle)->second; // rq = ttab[type_handle];

  for(int i = 0; i < rq->size(); i++)
  {
    r = rq->get(i);
    if(r->match_scope(scope))
      q->push_back(r);
  }

  return q;
}

//----------------------------------------------------------------------
// member function: get_by_type
//
//! Lookup a resource by \p type_handle and \p scope. Insert a record into
//! the get history list whether or not the get succeeded.
//----------------------------------------------------------------------

uvm_resource_base* uvm_resource_pool::get_by_type( const std::string& scope,
                                                   uvm_resource_base* type_handle )
{
  uvm_resource_types::rsrc_q_t* q;
  uvm_resource_base* rsrc;

  q = lookup_type(scope, type_handle);

  if(q->size() == 0)
  {
    push_get_record("<type>", scope, nullptr);
    return nullptr;
  }

  rsrc = q->get(0);
  push_get_record("<type>", scope, rsrc);
  return rsrc;
}

//----------------------------------------------------------------------
// member function: lookup_regex_names
//
//! This utility function answers the question, for a given \p name,
//! \p scope, and \p type_handle, what are all of the resources with a
//! matching name (where the resource name may be a regular
//! expression), a matching scope (where the resource scope may be a
//! regular expression), and a matching type? \p name and \p scope are
//! explicit values.
//----------------------------------------------------------------------

uvm_resource_types::rsrc_q_t* uvm_resource_pool::lookup_regex_names(
  const std::string& scope,
  const std::string& name,
  uvm_resource_base* type_handle)
{
  uvm_resource_types::rsrc_q_t* rq;
  uvm_resource_types::rsrc_q_t* result_q;
  uvm_resource_base* r;
  std::string re;

  //For the simple case where no wildcard names exist, then we can
  //just return the queue associated with name.

  if(!m_has_wildcard_names_ref())
  {
    result_q = lookup_name(scope, name, type_handle, false);
    return result_q;
  }

  result_q = new uvm_resource_types::rsrc_q_t();
  rsrc_list.push_back(result_q); // store to delete later

  for ( rtab_mapItT it = rtab.begin(); it != rtab.end(); it++ )
  {
    re = (*it).first;
    rq = (*it).second;

    for(int i = 0; i < rq->size(); i++)
    {
      r = rq->get(i);
      if( uvm_re_match( uvm_glob_to_re(re),name) == 0)
        // does the scope match?
        if(r->match_scope(scope) &&
          // does the type match?
          ((type_handle == nullptr) || (r->get_type_handle() == type_handle)))
          result_q->push_back(r);
    }
  }
  return result_q;
}

//----------------------------------------------------------------------
// member function: lookup_regex
//
//! Looks for all the resources whose name matches the regular
//! expression argument and whose scope matches the current scope.
//----------------------------------------------------------------------

uvm_resource_types::rsrc_q_t* uvm_resource_pool::lookup_regex(
  const std::string& re,
  const std::string& scope )
{
  uvm_resource_types::rsrc_q_t* rq;
  uvm_resource_types::rsrc_q_t* result_q;
  uvm_resource_base* r;
  std::string name;
  std::string regex;

  regex = uvm_glob_to_re(re);
  result_q = new uvm_resource_types::rsrc_q_t();

  for( rtab_mapItT it = rtab.begin(); it != rtab.end(); it++ )
  {
    name = (*it).first;
    if(uvm_re_match(regex, name))
      continue;

    rq = rtab[name];
    for(int i = 0; i < rq->size(); i++)
    {
      r = rq->get(i);
      if(r->match_scope(scope))
        result_q->push_back(r);
    }
  }

  return result_q;
}

//----------------------------------------------------------------------
// member function: lookup_scope
//
//! This is a utility function that answers the question: For a given
//! \p scope, what resources are visible to it?  Locate all the resources
//! that are visible to a particular scope.  This operation could be
//! quite expensive, as it has to traverse all of the resources in the
//! database.
//----------------------------------------------------------------------

uvm_resource_types::rsrc_q_t* uvm_resource_pool::lookup_scope(
  const std::string& scope )
{
  uvm_resource_types::rsrc_q_t* rq;
  uvm_resource_base* r;

  uvm_resource_types::rsrc_q_t* q;
  q = new uvm_resource_types::rsrc_q_t();

  // iterate in reverse order for the special case of autoconfig
  // of arrays. The array name with no [] needs to be higher priority.
  // This has no effect on manual accesses.
  for( rtab_maprItT it = rtab.rbegin(); it != rtab.rend(); it++ )
  {
    rq = rtab[it->first];
    for(int i = 0; i < rq->size(); ++i)
    {
      r = rq->get(i);
      if(r->match_scope(scope))
        q->push_back(r);
    }
  }
  return q;
}

//----------------------------------------------------------------------
// member function- set_priority_queue
//
//! This function handles the mechanics of moving a resource to either
//! the front or back of the queue.
//----------------------------------------------------------------------

void uvm_resource_pool::set_priority_queue( uvm_resource_base* rsrc,
                                            uvm_resource_types::rsrc_q_t* q,
                                            uvm_resource_types::priority_e pri)
{
  uvm_resource_base* r = nullptr;
  std::string msg;
  std::string name = rsrc->get_name();
  int i;

  for( i = 0; i < q->size(); i++ )
  {
    r = q->get(i);
    if(r == rsrc)
     break;
  }

  if(r != rsrc) 
  {
    std::ostringstream msg;
    msg << "Handle for resource named "<< name
        << " is not in the name name; cannot change its priority.";
    uvm_report_error("NORSRC", msg.str() );
    return;
  }

  q->do_delete(i);

  switch(pri)
  {
    case uvm_resource_types::PRI_HIGH: q->push_front(rsrc); break;
    case uvm_resource_types::PRI_LOW:  q->push_back(rsrc); break;
  }
}

//----------------------------------------------------------------------
// member function: set_priority_type
//
//! Change the priority of the \p rsrc based on the value of \p pri, the
//! priority enum argument. This function changes the priority only in
//! the type map, leaving the name map untouched.
//----------------------------------------------------------------------

void uvm_resource_pool::set_priority_type( uvm_resource_base* rsrc,
                                           uvm_resource_types::priority_e pri)
{
  uvm_resource_base* type_handle;
  std::string msg;
  uvm_resource_types::rsrc_q_t* q;

  if(rsrc == nullptr)
  {
    uvm_report_warning("NULLRASRC", "attempting to change the serach priority of a nullptr resource");
    return;
  }

  type_handle = rsrc->get_type_handle();

  if(ttab.find(type_handle)==ttab.end())
  {
    std::ostringstream msg;
    msg << "Type handle for resource named " << rsrc->get_name()
        << " not found in type map; cannot change its search priority.";
    uvm_report_error("RNFTYPE", msg.str() );
    return;
  }

  q = ttab[type_handle];
  set_priority_queue(rsrc, q, pri);
}

//----------------------------------------------------------------------
// member function: set_priority_name
//
//! Change the priority of the \p rsrc based on the value of \p pri, the
//! priority enum argument.  This function changes the priority only in
//! the name map, leaving the type map untouched.
//----------------------------------------------------------------------

void uvm_resource_pool::set_priority_name( uvm_resource_base* rsrc,
                                           uvm_resource_types::priority_e pri)
{
  std::string name;
  std::string msg;
  uvm_resource_types::rsrc_q_t* q;

  if(rsrc == nullptr)
  {
    uvm_report_warning("NULLRASRC", "Attempting to change the search priority of a nullptr resource.");
    return;
  }

  name = rsrc->get_name();
  if( rtab.find(name) == rtab.end() )
  {
    std::ostringstream msg;
    msg << "Resource named '" << name << "' not found in name map; cannot change its search priority.";
    uvm_report_error("RNFNAME", msg.str() );
    return;
  }

  q = rtab[name];
  set_priority_queue(rsrc, q, pri);
}

//--------------------------------------------------------------------
// member function: set_priority
//
//! Change the search priority of the \p rsrc based on the value of \p pri,
//! the priority enum argument.  This function changes the priority in
//! both the name and type maps.
//--------------------------------------------------------------------

void uvm_resource_pool::set_priority( uvm_resource_base* rsrc,
                                      uvm_resource_types::priority_e pri)
{
  set_priority_type(rsrc, pri);
  set_priority_name(rsrc, pri);
}

//--------------------------------------------------------------------
// member function: find_unused_resources
//
//! Locate all the resources that have at least one write and no reads
//--------------------------------------------------------------------

uvm_resource_types::rsrc_q_t* uvm_resource_pool::find_unused_resources() const
{
  uvm_resource_types::rsrc_q_t* rq;
  uvm_resource_types::rsrc_q_t* q = new uvm_resource_types::rsrc_q_t();
  uvm_resource_base* r;
  uvm_resource_types::access_t a;
  int reads;
  int writes;

  for( rtab_mapcItT it = rtab.begin(); it != rtab.end(); it++ )
  {
    rq = (*it).second;
    for( int i = 0; i < rq->size(); i++ )
    {
      r = rq->get(i);
      reads = 0;
      writes = 0;
      for( r->access_mapIt = r->access.begin(); r->access_mapIt != r->access.end(); r->access_mapIt++ )
      {
        a = (*(r->access_mapIt)).second;
        reads += a.read_count;
        writes += a.write_count;
      }

      if(writes > 0 && reads == 0)
        q->push_back(r);
    }
  }

  return q;
}


//--------------------------------------------------------------------
// member function: print_resources
//
//! Print the resources that are in a single queue, \p rq. This is a utility
//! function that can be used to print any collection of resources
//! stored in a queue.  The \p audit flag determines whether or not the
//! audit trail is printed for each resource along with the name,
//! value, and scope regular expression.
//--------------------------------------------------------------------

void uvm_resource_pool::print_resources( uvm_resource_types::rsrc_q_t* rq,
                                         bool audit ) const
{
  uvm_resource_base* r;
  uvm_line_printer* printer = new uvm_line_printer();
  int size = 0;

  printer->knobs.separator.clear(); // no separators used
  printer->knobs.full_name = 0;
  printer->knobs.identifier = 0;
  printer->knobs.type_name = 0;
  printer->knobs.reference = 0;

  if(rq != nullptr)
    size = rq->size();

  if(rq == nullptr || size == 0)
  {
    uvm_report_info("UVM/RESOURCE/PRINT","<none>", UVM_NONE);
    return;
  }

  for(int i = 0; i < size; i++)
  {
    r = rq->get(i);
    r->print(printer);
    if(audit == true)
      r->print_accessors();
  }

  delete printer;
}

//--------------------------------------------------------------------
// member function: dump
//
//! Dump the entire resource pool.  The resource pool is traversed and
//! each resource is printed.  The utility function print_resources()
//! is used to initiate the printing. If the \p audit bit is set then
//! the audit trail is dumped for each resource.
//--------------------------------------------------------------------

void uvm_resource_pool::dump( bool audit ) const
{
  uvm_resource_types::rsrc_q_t* rq;
  std::string name;

  UVM_INFO("UVM/RESOURCE/DUMP", "\n=== resource pool ===", UVM_NONE);

  for( rtab_mapcItT it = rtab.begin(); it != rtab.end(); it++ )
  {
    rq = (*it).second;
    print_resources(rq, audit);
  }

  UVM_INFO("UVM/RESOURCE/DUMP", "=== end of resource pool ===", UVM_NONE);
}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------
// member function: cleanup
//
//! Implementation defined
//! Delete resource pool and free the memory
//----------------------------------------------------------------------

void uvm_resource_pool::cleanup()
{
  uvm_coreservice_t::get()->reset_uvm_resource_pool_rp();
}


} // namespace uvm
