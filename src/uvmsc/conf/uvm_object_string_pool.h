//-----------------------------------------------------------------------------
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc. 
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2014-2015 NXP B.V.
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
//-----------------------------------------------------------------------------

#ifndef UVM_OBJECT_STRING_POOL_H_
#define UVM_OBJECT_STRING_POOL_H_

#include <map>
#include <memory>
#include <typeinfo>

#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"
#include "uvmsc/base/uvm_void.h"
#include "uvmsc/base/uvm_object.h"
#include "uvmsc/base/uvm_globals.h"
#include "uvmsc/print/uvm_printer.h"

namespace uvm {


//------------------------------------------------------------------------------
// CLASS: uvm_object_string_pool<T>
//
//! This provides a specialization of the generic #std::map<KEY,T> class for
//! an associative array of #uvm_object-based objects indexed by string.
//! Specializations of this class include the #uvm_event_pool (a
//! uvm_object_string_pool storing objects of type #uvm_event) and
//! #uvm_barrier_pool (a #uvm_obejct_string_pool storing objects of type
//! #uvm_barrier).
//------------------------------------------------------------------------------

template <typename T = uvm_object >
class uvm_object_string_pool : public std::map<std::string,T>
{
 public:
  uvm_object_string_pool();

  explicit uvm_object_string_pool( const std::string& name = "" );

  virtual ~uvm_object_string_pool();

  virtual const std::string get_type_name() const;

  static uvm_object_string_pool<T>* get_global_pool();

  static T get_global( const std::string& key );

  virtual T get( const std::string& key );

 virtual void do_delete( const std::string& key );

  virtual void do_print( const uvm_printer& printer ) const;

 protected:
  const static std::string type_name;

}; // class uvm_object_string_pool


/////////////////////////////////////////
// Class implementation starts here
/////////////////////////////////////////

//-----------------------------------------------------------------------------
// Initialization of static data members
//-----------------------------------------------------------------------------

template <typename T>
const std::string uvm_object_string_pool<T>::type_name = "uvm::uvm_obj_str_pool";

//-----------------------------------------------------------------------------
// Constructors
//
//! Creates a new pool with the given \p name.
//-----------------------------------------------------------------------------

template <typename T>
uvm_object_string_pool<T>::uvm_object_string_pool()
  : std::map<std::string,T>()
{
  this->pool.clear();
}

template <typename T>
uvm_object_string_pool<T>::uvm_object_string_pool( const std::string& name )
: std::map<std::string,T>(name)
{
  this->pool.clear();
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------

template <typename T>
uvm_object_string_pool<T>::~uvm_object_string_pool()
{
}

//-----------------------------------------------------------------------------
// member function: get_type_name (virtual)
//
//! Returns the type name of this object.
//-----------------------------------------------------------------------------

template <typename T>
const std::string uvm_object_string_pool<T>::get_type_name() const
{
  return type_name;
}

//-----------------------------------------------------------------------------
// member function: get_global_pool (static)
//
//! Returns the singleton global pool for the item type, T.
//! This allows items to be shared amongst components throughout the
//! verification environment.
//-----------------------------------------------------------------------------

template <typename T>
uvm_object_string_pool<T>* uvm_object_string_pool<T>::get_global_pool()
{
  // Refactored from the former uvm_object_string_pool<T>::m_global_pool global to uvm_coreservice_t.
  std::unique_ptr<uvm_object_string_pool<T> >& gpool =
    uvm_coreservice_t::get()->get_or_create_typed_store<std::unique_ptr<uvm_object_string_pool<T> > >(
      std::string("uvm_object_string_pool::m_global_pool:") + typeid(T).name(), nullptr);

  if (gpool == nullptr)
    gpool.reset(new uvm_object_string_pool("global_object_string_pool"));

  return gpool.get();
}

//-----------------------------------------------------------------------------
// member function: get_global (static)
//
//! Returns the specified item instance from the global item pool.
//-----------------------------------------------------------------------------

template <typename T>
T uvm_object_string_pool<T>::get_global( const std::string& key )
{
  uvm_object_string_pool<T>* gpool;
  gpool = get_global_pool();
  return gpool->get(key);
}

//-----------------------------------------------------------------------------
// member function: get
//
//! Returns the object item at the given string \p key.
//!
//! If no item exists by the given \p key, a new item is created for that key
//! and returned.
//-----------------------------------------------------------------------------

template <typename T>
T uvm_object_string_pool<T>::get( const std::string& key )
{
  if (this->pool.find(key) == this->pool.end() ) // if not exist
  {
    T t = T(); // TODO check validity for all relevant types
    this->pool[key] = t;
  }
  return this->pool[key];
}

//-----------------------------------------------------------------------------
// member function: do_delete
//
//! Removes the item with the given string \p key from the pool.
//-----------------------------------------------------------------------------

template <typename T>
void uvm_object_string_pool<T>::do_delete( const std::string& key )
{
  if (!this->exists(key))
  {
    uvm_report_warning("POOLDEL",
      "do_delete: key '" + key + "' doesn't exist");
    return;
  }
  this->pool.erase(key);
}

//-----------------------------------------------------------------------------
// member function: do_print
//-----------------------------------------------------------------------------

template <typename T>
void uvm_object_string_pool<T>::do_print( const uvm_printer& printer ) const
{
  std::string key;
  printer.print_array_header("pool", this->pool.size(), "aa_object_string");

  typename std::map<std::string,T>:: const_iterator it;
  for ( it = this->pool.begin();
       it != this->pool.end();
       it++)
  {
    std::string key = (*it).first;
    printer.print_object("[" + key + "]", *(it->second), "[");
  }

  printer.print_array_footer();
}

///////////////

// TODO add barrier and event pool as soon as classes are defined

//typedef uvm_object_string_pool<uvm_barrier> uvm_barrier_pool;
//typedef uvm_object_string_pool<uvm_event> uvm_event_pool;


} // namespace uvm

#endif // UVM_OBJECT_STRING_POOL_H_
