//----------------------------------------------------------------------
//   Copyright 2011 Cypress Semiconductor
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2011 Cadence Design Systems, Inc.
//   Copyright 2012-2014 NXP B.V.
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

#ifndef UVM_RESOURCE_POOL_H_
#define UVM_RESOURCE_POOL_H_
#include <map>

#include "uvmsc/conf/uvm_queue.h"
#include "uvmsc/conf/uvm_resource_types.h"
#include "uvmsc/base/uvm_object.h"


namespace uvm {

// forward class references
class uvm_resource_base;
class get_t;
class uvm_default_coreservice_t;

//----------------------------------------------------------------------
// Class: uvm_resource_pool
//
//! The global (singleton) resource database.
//----------------------------------------------------------------------

class uvm_resource_pool
{
  friend class uvm_root;
  friend class uvm_default_coreservice_t;

 public:

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  static uvm_resource_pool* get();

  bool spell_check( const std::string& s ) const;

  //--------------------------------------------------------------------------
  // Group: Set
  //--------------------------------------------------------------------------

  void set( uvm_resource_base* rsrc,
            uvm_resource_types::override_t override = 0);

  void set_override( uvm_resource_base* rsrc );
  void set_name_override( uvm_resource_base* rsrc );
  void set_type_override( uvm_resource_base* rsrc );

  void push_get_record( const std::string& name,
                        const std::string& scope,
                        uvm_resource_base* rsrc);

  void dump_get_records() const;

  //--------------------------------------------------------------------------
  // Group: Lookup
  //--------------------------------------------------------------------------

  uvm_resource_types::rsrc_q_t* lookup_name( const std::string& scope,
                                             const std::string& name,
                                             uvm_resource_base* type_handle,
                                             bool rpterr = true ) const;

  uvm_resource_base* get_highest_precedence( uvm_resource_types::rsrc_q_t* q ) const;

  static void sort_by_precedence( uvm_resource_types::rsrc_q_t* q );

  uvm_resource_base* get_by_name( const std::string& scope,
                                  const std::string& name,
                                  uvm_resource_base* type_handle,
                                  bool rpterr = true );

  uvm_resource_types::rsrc_q_t* lookup_type( const std::string& scope,
                                             uvm_resource_base* type_handle ) const;

  uvm_resource_base* get_by_type( const std::string& scope,
                                  uvm_resource_base* type_handle );

  uvm_resource_types::rsrc_q_t* lookup_regex_names( const std::string& scope,
                                                    const std::string& name,
                                                    uvm_resource_base* type_handle = nullptr );

  uvm_resource_types::rsrc_q_t* lookup_regex( const std::string& re,
                                              const std::string& scope );

  uvm_resource_types::rsrc_q_t* lookup_scope( const std::string& scope );

  //--------------------------------------------------------------------------
  // Group: Set Priority
  //--------------------------------------------------------------------------

  void set_priority_queue( uvm_resource_base* rsrc,
                           uvm_resource_types::rsrc_q_t* q,
                           uvm_resource_types::priority_e pri );

  void set_priority_type( uvm_resource_base* rsrc,
                          uvm_resource_types::priority_e pri );

  void set_priority_name( uvm_resource_base* rsrc,
                          uvm_resource_types::priority_e pri );

  void set_priority( uvm_resource_base* rsrc,
                     uvm_resource_types::priority_e pri );

  //--------------------------------------------------------------------------
  // Group: Debug
  //--------------------------------------------------------------------------

  uvm_resource_types::rsrc_q_t* find_unused_resources() const;

  void print_resources( uvm_resource_types::rsrc_q_t* rq,
                        bool audit = false ) const;

  void dump( bool audit = false ) const;

 private:

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

 static void cleanup();

  // data members

  typedef std::map<std::string, uvm_resource_types::rsrc_q_t* > rtab_mapT;
  typedef rtab_mapT::iterator rtab_mapItT;
  typedef rtab_mapT::const_iterator rtab_mapcItT;
  typedef rtab_mapT::reverse_iterator rtab_maprItT;
  rtab_mapT rtab;

  typedef std::map<uvm_resource_base*, uvm_resource_types::rsrc_q_t* > ttab_mapT;
  typedef ttab_mapT::iterator ttab_mapItT;
  ttab_mapT ttab;

  typedef std::list<get_t*> get_record_listT;
  typedef get_record_listT::const_iterator get_record_list_cItT;
  get_record_listT get_record;  // history (list) of gets

 std::vector<uvm_resource_types::rsrc_q_t* > rsrc_list;

 protected:
  uvm_resource_pool();

 public:
  ~uvm_resource_pool();

}; // class uvm_resource_pool


//----------------------------------------------------------------------
// static global resource pool handle
//----------------------------------------------------------------------
// TODO where do we need this?
//const uvm_resource_pool* uvm_resources = uvm_resource_pool::get();


} // namespace uvm


#endif // UVM_RESOURCE_POOL_H_
