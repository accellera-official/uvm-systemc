//----------------------------------------------------------------------
//   Copyright 2011 Cypress Semiconductor
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2011 Cadence Design Systems, Inc.
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2013 Fraunhofer-Gesellschaft zur Foerderung
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

#ifndef UVM_RESOURCE_H_
#define UVM_RESOURCE_H_

#include "uvmsc/base/uvm_object.h"
#include "uvmsc/conf/uvm_resource_types.h"
#include "uvmsc/conf/uvm_resource_base.h"
#include "uvmsc/conf/uvm_resource_pool.h"
#include "uvmsc/conf/uvm_resource_converter.h"
#include "uvmsc/macros/uvm_message_defines.h"
#include "uvmsc/base/uvm_object_globals.h"

namespace uvm {

 // forward class references
class uvm_resource_base;
class uvm_resource_types;

//----------------------------------------------------------------------
// Class: uvm_resource<T>
//
//! Parameterized resource.  Provides essential access methods to read
//! from and write to the resource database.
//----------------------------------------------------------------------

template <typename T = int>
class uvm_resource : public uvm_resource_base
{
  template <typename TYPE> friend class uvm_config_db;
  template <typename TYPE> friend class uvm_resource_db;

 public:
  typedef uvm_resource<T>* this_type;

  //--------------------------------------------------------------------
  // Group: Type Interface
  //--------------------------------------------------------------------

  static uvm_resource<T>* get_type();

  uvm_resource_base* get_type_handle() const override;

  //--------------------------------------------------------------------
  // Group: Set/Get Interface
  //--------------------------------------------------------------------

  void set();

  void set_override( uvm_resource_types::override_e override = uvm_resource_types::BOTH_OVERRIDE ); // TODO default was "0b11"

  static uvm_resource<T>* get_by_name( const std::string& scope,
                                       const std::string& name,
                                       bool rpterr = true );

  static uvm_resource<T>* get_by_type( const std::string& scope,
                                       uvm_resource_base* type_handle );

  //--------------------------------------------------------------------
  // Group: Read/Write Interface
  //--------------------------------------------------------------------

  // TODO check if standard initialization with nullptr is neccessary (potential problems with MSVC)
  T read( uvm_object*& accessor ); // = nullptr

  void write( const T& t, uvm_object*& accessor); // = nullptr

  //--------------------------------------------------------------------
  // Group: Priority
  //--------------------------------------------------------------------

  void set_priority( uvm_resource_types::priority_e pri ) override;

  static uvm_resource<T>* get_highest_precedence( uvm_resource_types::rsrc_q_t* q );


  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

 private:

  // constructor
  uvm_resource( const std::string& name_ = "", const std::string& scope_ = "" );
  ~uvm_resource() override;

  static m_uvm_resource_converter<T>* m_get_converter();

  static void m_set_converter(m_uvm_resource_converter<T>* r2s);

  std::string convert2string() const override;

  void set_object() { is_object = true; }

  // data members

  // Singleton used to convert this resource to a string
  static m_uvm_resource_converter<T>* m_r2s;

  // singleton handle that represents the type of this resource
  static this_type my_type;

 protected:
  T val;

  bool is_object;
};

//----------------------------------------------------------------------
// Initialize static data members
//----------------------------------------------------------------------

template <typename T>
uvm_resource<T>* uvm_resource<T>::my_type = uvm_resource<T>::get_type();

template <typename T>
m_uvm_resource_converter<T>* uvm_resource<T>::m_r2s = 0;

//----------------------------------------------------------------------
// constructor
//----------------------------------------------------------------------

template <typename T>
uvm_resource<T>::uvm_resource( const std::string& name_,
                               const std::string& scope_ )
  : uvm_resource_base( name_, scope_ )
{
  is_object = false;
}

//----------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------

template <typename T>
uvm_resource<T>::~uvm_resource()
{
}

//----------------------------------------------------------------------
// member function: m_get_converter
//
//! Get the conversion policy class that specifies how to convert the value
//! of a resource of this type to a string
//----------------------------------------------------------------------

template <typename T>
m_uvm_resource_converter<T>* uvm_resource<T>::m_get_converter()
{
  if ( m_r2s == nullptr )
    m_r2s = new m_uvm_resource_converter<T>();
  return m_r2s;
}

//----------------------------------------------------------------------
// member function: m_set_converter
//
//! Specify how to convert the value of a resource of this type to a string
//! If not specified (or set to nullptr),
//! a default converter that display the name of the resource type is used.
//! Default conversion policies are specified for the built-in type.
//----------------------------------------------------------------------

template <typename T>
void uvm_resource<T>::m_set_converter(m_uvm_resource_converter<T>* r2s)
{
  m_r2s = r2s;
}

//----------------------------------------------------------------------
// member function: convert2string()
//
//! Convert to string
//----------------------------------------------------------------------
template <typename T>
std::string uvm_resource<T>::convert2string() const
{
  m_get_converter(); // make sure the converter is there
  return m_r2s->convert2string(val);
}

//----------------------------------------------------------------------
// member function: get_type
//
//----------------------------------------------------------------------

template <typename T>
uvm_resource<T>* uvm_resource<T>::get_type()
{
  if ( my_type == nullptr )
    my_type = new uvm_resource<T>();
  return my_type;
}

//----------------------------------------------------------------------
// member function: get_type_handle
//
//! Returns the static type handle of this resource in a polymorphic
//! fashion.  The return type of get_type_handle() is
//! #uvm_resource_base.  This function is not static and therefore can
//! only be used by instances of a parameterized resource.
//----------------------------------------------------------------------

template <typename T>
uvm_resource_base* uvm_resource<T>::get_type_handle() const
{
  return get_type();
}

//----------------------------------------------------------------------
// member function: set
//
//! Simply put this resource into the global resource pool
//----------------------------------------------------------------------

template <typename T>
void uvm_resource<T>::set()
{
  uvm_resource_pool* rp = uvm_resource_pool::get();
  rp->set(this);
}

//----------------------------------------------------------------------
// member function: set_override
//
//! Put a resource into the global resource pool as an override.  This
//! means it gets put at the head of the list and is searched before
//! other existing resources that occupy the same position in the name
//! map or the type map.  The default is to override both the name and
//! type maps. However, using the \p override argument you can specify
//! that either the name map or type map is overridden.
//----------------------------------------------------------------------

template <typename T>
void uvm_resource<T>::set_override( uvm_resource_types::override_e override )
{
  uvm_resource_pool* rp = uvm_resource_pool::get();
  rp->set(this, override);
}

//----------------------------------------------------------------------
// member function: get_by_name
//
//! looks up a resource by \p name in the name map. The first resource
//! with the specified nam, whose type is the current type, and is
//! visible in the specified \p scope is returned, if one exists.  The
//! \p rpterr flag indicates whether or not an error should be reported
//! if the search fails.  If \p rpterr is set to one then a failure
//! message is issued, including suggested spelling alternatives, based
//! on resource names that exist in the database, gathered by the spell
//! checker.
//----------------------------------------------------------------------

template <typename T>
uvm_resource<T>* uvm_resource<T>::get_by_name( const std::string& scope,
                                               const std::string& name,
                                               bool rpterr )
{
  uvm_resource_pool* rp = uvm_resource_pool::get();
  uvm_resource_base* rsrc_base;
  uvm_resource<T>* rsrc;
  std::string msg;

  rsrc_base = rp->get_by_name(scope, name, my_type, rpterr);
  if( rsrc_base == nullptr )
    return nullptr;

  rsrc = dynamic_cast<uvm_resource<T>*>(rsrc_base);
  if(rsrc == nullptr)
  {
    if(rpterr)
    {
      std::ostringstream msg;
      msg << "Resource with name " << name << " in scope " << scope << " has incorrect type.";
      UVM_WARNING("RSRCTYPE", msg.str() );
    }
    return nullptr;
  }

  return rsrc;
}

//----------------------------------------------------------------------
// member function: get_by_type
//
//! Looks up a resource by \p type_handle in the type map. The first resource
//! with the specified \p type_handle that is visible in the specified \p scope is
//! returned, if one exists. nullptr is returned if there is no resource matching
//! the specifications.
//----------------------------------------------------------------------

template <typename T>
uvm_resource<T>* uvm_resource<T>::get_by_type( const std::string& scope,
                                               uvm_resource_base* type_handle )
{
  uvm_resource_pool* rp = uvm_resource_pool::get();
  uvm_resource_base* rsrc_base;
  uvm_resource<T>* rsrc;
  std::string msg;

  if( type_handle == nullptr )
    return nullptr;

  rsrc_base = rp->get_by_type(scope, type_handle);
  if( rsrc_base == nullptr )
    return nullptr;

  rsrc = dynamic_cast<uvm_resource<T>*>(rsrc_base);
  if(rsrc == nullptr)
  {
    std::ostringstream msg;
    msg << "Resource with specified type handle in scope " << scope << " was not located.";
    UVM_WARNING("RSRCNF", msg.str() );
    return nullptr;
  }
  return rsrc;
}

//----------------------------------------------------------------------
// member function: read
//
//! Return the object stored in the resource container.  If an \p accessor
//! object is supplied then also update the accessor record for this
//! resource.
//----------------------------------------------------------------------

template <typename T>
T uvm_resource<T>::read( uvm_object*& accessor )
{
  record_read_access(accessor);
  return val;
}

//----------------------------------------------------------------------
// member function: write
//
//! Modify the object stored in this resource container.  If the
//! resource is read-only then issue an error message and return
//! without modifying the object in the container.  If the resource is
//! not read-only and an \p accessor object has been supplied then also
//! update the accessor record.  Lastly, replace the object value in the
//! container with the value supplied as the  argument, \p t, and
//! release any processes blocked on uvm_resource_base::wait_modified.
//----------------------------------------------------------------------

template <typename T>
void uvm_resource<T>::write( const T& t, uvm_object*& accessor )
{
  if(is_read_only())
  {
    std::ostringstream msg;
    msg << "resource "<< get_name() << " is read only -- cannot modify.";
    uvm_report_error("resource", msg.str() );
    return;
  }

  record_write_access(accessor);

  // set the value and flag the modification
  val = t;
  modified.notify(sc_core::SC_ZERO_TIME);
}


//----------------------------------------------------------------------
// member function: set priority
//
//! Change the search priority of the resource based on the value of
//! the priority enum argument, \p pri.
//----------------------------------------------------------------------

template <typename T>
void uvm_resource<T>::set_priority( uvm_resource_types::priority_e pri )
{
  uvm_resource_pool* rp = uvm_resource_pool::get();
  rp->set_priority(this, pri);
}

//----------------------------------------------------------------------
// member function: get_highest_precedence
//
//! In a queue of resources, locate the first one with the highest
//! precedence whose type is T.  This function is static so that it can
//! be called from anywhere.
//----------------------------------------------------------------------

template <typename T>
uvm_resource<T>* uvm_resource<T>::get_highest_precedence( uvm_resource_types::rsrc_q_t* q )
{
  uvm_resource<T>* rsrc = nullptr;
  uvm_resource<T>* r = nullptr;
  unsigned int prec = 0;
  unsigned int first = 0;

  if(q->size() == 0)
    return nullptr;

  // Locate first resources in the queue whose type is T
  for(first = 0; first < (unsigned)q->size(); first++)
  {
    rsrc = dynamic_cast<uvm_resource<T>* >(q->get(first));
    if (rsrc != nullptr)
      break;
  }

  // no resource in the queue whose type is T
  if(rsrc == nullptr)
    return nullptr;
  prec = rsrc->precedence;

  // start searching from the next resource after the first resource
  // whose type is T
  for(unsigned int i = first+1; i < (unsigned)q->size(); i++)
  {
    r = dynamic_cast<uvm_resource<T>* >(q->get(i));
    if( r!= nullptr )
    {
      if(r->precedence > prec)
      {
        rsrc = r;
        prec = r->precedence;
      }
    }
  }

  return rsrc;
}

} // namespace uvm

#endif // UVM_RESOURCE_H_
