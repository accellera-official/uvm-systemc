//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2013-2015 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc.
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

#ifndef UVM_CALLBACKS_H_
#define UVM_CALLBACKS_H_

#include <iostream>
#include <string>

#include "uvmsc/base/uvm_root.h"
#include "uvmsc/base/uvm_object.h"
#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"
#include "uvmsc/cb/uvm_typeid.h"
#include "uvmsc/cb/uvm_typed_callbacks.h"
#include "uvmsc/cb/uvm_callback.h"
#include "uvmsc/macros/uvm_callback_defines.h"

#include "uvmsc/reg/uvm_reg_cbs.h"
#include "uvmsc/reg/uvm_vreg_cbs.h"
#include "uvmsc/reg/uvm_vreg_field_cbs.h"

namespace uvm {


// forward class declaration
class uvm_component;

//----------------------------------------------------------------------
// CLASS: uvm_callbacks<T,CB>
//
//! The class #uvm_callbacks provides a base class for implementing callbacks,
//! which are typically used to modify or augment component behavior without
//! changing the component class. To work effectively, the developer of the
//! component class defines a set of "hook" methods that enable users to
//! customize certain behaviors of the component in a manner that is controlled
//! by the component developer. The integrity of the component's overall behavior
//! is intact, while still allowing certain customizable actions by the user.
//!
//! To enable compile-time type-safety, the class is parameterized on both the
//! user-defined callback interface implementation as well as the object type
//! associated with the callback. The object type-callback type pair are
//! associated together using the macro #UVM_REGISTER_CB to define
//! a valid pairing; valid pairings are checked when a user attempts to add
//! a callback to an object.
//!
//! To provide the most flexibility for end-user customization and reuse, it
//! is recommended that the component developer also define a corresponding set
//! of virtual method hooks in the component itself. This affords users the ability
//! to customize via inheritance/factory overrides as well as callback object
//! registration. The implementation of each virtual method would provide the
//! default traversal algorithm for the particular callback being called. Being
//! virtual, users can define subtypes that override the default algorithm,
//! perform tasks before and/or after calling the base class method to execute any
//! registered callbacks, or to not call the base implementation, effectively
//! disabling that particular hook. A demonstration of this methodology is
//! provided in an example included in the kit.
//----------------------------------------------------------------------

template <typename T = uvm_object, typename CB = uvm_callback>
class uvm_callbacks : public uvm_typed_callbacks<T>
{
 public:

  // Parameter: T
  //
  // This type parameter specifies the base object type with which the
  // callback type CB objects will be registered. This object must be
  // a derivative of #uvm_object.

  // Parameter: CB
  //
  // This type parameter specifies the base callback type that will be
  // managed by this callback class. The callback type is typically a
  // interface class, which defines one or more virtual method prototypes 
  // that users can override in subtypes. This type must be a derivative
  // of #uvm_callback.
  
  typedef uvm_typed_callbacks<T> base_type; // C++ expected typedef
  typedef uvm_typed_callbacks<T> super_type; // SV expected typedef
  typedef uvm_callbacks<T,CB>    this_type;

  uvm_callbacks(); // Constructor

  //----------------------------------------------------------------------
  // Group: Add/delete interface
  //----------------------------------------------------------------------

  static void add( T* obj, uvm_callback* cb, uvm_apprepend ordering = UVM_APPEND );

  static void add_by_name( const std::string& name,
                           uvm_callback* cb,
                           uvm_component* root,
                           uvm_apprepend ordering = UVM_APPEND );

  static void do_delete( T* obj, uvm_callback* cb );

  static void delete_by_name( const std::string& name,
                              uvm_callback* cb,
                              uvm_component* root );

  //----------------------------------------------------------------------
  // Group: Iterator Interface
  //----------------------------------------------------------------------

  static CB* get_first( int& itr, T* obj );

  static CB* get_last( int& itr, T* obj );

  static CB* get_next( int& itr, T* obj );

  static CB* get_prev( int& itr, T* obj );

  //----------------------------------------------------------------------
  // Group: Debug
  //----------------------------------------------------------------------

  static void display( T* obj = nullptr );

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

  //TODO move to private?
  static uvm_callbacks<T,CB>* get();

  static bool m_register_pair( const std::string& tname = "", const std::string& cbname = "");

 private:

  static void m_get_q( uvm_queue<uvm_callback*>*& q, T* obj );

  bool m_is_registered( uvm_object* obj, uvm_callback* cb ) override;

  bool m_is_for_me( uvm_callback* cb ) override;

  // data members

  // Singleton instance is used for type checking
  static uvm_callbacks<T,CB>* m_inst;

  // typeinfo

  static uvm_typeid_base* m_typeid;
  static uvm_typeid_base* m_cb_typeid;

  static std::string m_typename;
  static std::string m_cb_typename;

  // TODO reporting for callbacks
  //static uvm_report_object reporter = new("cb_tracer");

  // static uvm_callbacks<T, uvm_callback>* m_base_inst;
  static uvm_typed_callbacks<T> * m_base_inst;

  bool m_registered;

}; // class uvm_callbacks

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// static data member initialization
//----------------------------------------------------------------------

template <typename T, typename CB>
uvm_callbacks<T,CB>* uvm_callbacks<T,CB>::m_inst = nullptr;

template <typename T, typename CB>
uvm_typeid_base* uvm_callbacks<T,CB>::m_typeid = nullptr;

template <typename T, typename CB>
uvm_typeid_base* uvm_callbacks<T,CB>::m_cb_typeid = nullptr;

template <typename T, typename CB>
std::string uvm_callbacks<T,CB>::m_typename;

template <typename T, typename CB>
std::string uvm_callbacks<T,CB>::m_cb_typename;

template <typename T, typename CB>
uvm_typed_callbacks<T>* uvm_callbacks<T,CB>::m_base_inst = nullptr;

//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------

template <typename T, typename CB>
uvm_callbacks<T,CB>::uvm_callbacks()
{
  m_registered = false;
}

//----------------------------------------------------------------------
// Group: Add/delete interface
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: add (static)
//
// Registers the given callback object, ~cb~, with the given
// ~obj~ handle. The ~obj~ handle can be null, which allows
// registration of callbacks without an object context. If
// ~ordreing~ is UVM_APPEND (default), the callback will be executed
// after previously added callbacks, else  the callback
// will be executed ahead of previously added callbacks. The ~cb~
// is the callback handle; it must be non-null, and if the callback
// has already been added to the object instance then a warning is
// issued. Note that the CB parameter is optional. For example, the
// following are equivalent:
//
//| uvm_callbacks<my_comp>::add(comp_a, cb);
//| uvm_callbacks<my_comp, my_callback>::add(comp_a,cb);
//----------------------------------------------------------------------

template <typename T, typename CB>
void uvm_callbacks<T,CB>::add( T* obj, uvm_callback* cb, uvm_apprepend ordering )
{
  uvm_queue<uvm_callback*>* q = nullptr;
  uvm_object* bobj = nullptr;

  std::string nm;
  std::string tnm;

  // last-minute registration of the object callback
  // this differs from UVMSV since we cannot register using the macro
  // because the static method cannot be initialize within the class declaration
  if( obj != nullptr )
    obj->m_register_cb();

  get();

  if( cb == nullptr )
  {
    if( obj == nullptr )
      nm = "(*)";
    else
      nm = obj->get_full_name();

    if (!m_base_inst->m_typename.empty())
      tnm = m_base_inst->m_typename;
    else
    {
      if (obj != nullptr)
        tnm = obj->get_type_name();
      else
        tnm = "uvm_object";
    }

    uvm_report_error("CBUNREG",
        "nullptr callback object cannot be registered with object " +
        nm + " (" + tnm + ")", UVM_NONE);
    return;
  }

  if (!m_base_inst->check_registration(obj, cb))
  {
    //std::cout << "not registered obj/cb" << std::endl;
    if (obj == nullptr)
      nm = "(*)";
    else
      nm = obj->get_full_name();

    if (!m_base_inst->m_typename.empty())
      tnm = m_base_inst->m_typename;
    else
    {
      if(obj != nullptr)
        tnm = obj->get_type_name();
      else
        tnm = "uvm_object";
    }

    uvm_report_warning("CBUNREG",
        "Callback " + cb->get_name() + " cannot be registered with object " +
        nm + " because callback type " + cb->get_type_name() +
        " is not registered with object type " + tnm, UVM_NONE);
    return;
  }

  if(obj == nullptr)
  {
    if ( uvm_typed_callbacks<T>::m_cb_find(uvm_typed_callbacks<T>::m_t_inst->m_tw_cb_q, cb) != -1)
    {
      if (!m_base_inst->m_typename.empty())
        tnm = m_base_inst->m_typename;
      else tnm = "uvm_object";

      uvm_report_warning("CBPREG",
          "Callback object " + cb->get_name() +
          " is already registered with type " + tnm, UVM_NONE);
    }
    else
    {
      std::ostringstream str;
      str << "Add ("
          << uvm_apprepend_name[ordering]
          << ") typewide callback "
          << cb->get_name()
          << " for type "
          << m_base_inst->m_typename;
      UVM_CB_TRACE_NOOBJ(cb,str.str())
      uvm_typed_callbacks<T>::m_t_inst->m_add_tw_cbs(cb, ordering);
    }
  }
  else
  {
    std::ostringstream str;
    str << "Add ("
        << uvm_apprepend_name[ordering]
        << ") callback "
        << cb->get_name()
        << " to object "
        << obj->get_full_name();
    UVM_CB_TRACE_NOOBJ(cb, str.str())


    bobj =  dynamic_cast<uvm_object*>(obj);
    if (bobj == nullptr)
      uvm_report_error("CBPREG",
        "Callback object cannot be registered with object " +
        nm + " because " + nm + " is not derived from type uvm_object.", UVM_NONE);
    else
        q = (*m_base_inst->m_pool)[bobj];

    if (q == nullptr)
    {
      q = new uvm_queue<uvm_callback*>();
      (*m_base_inst->m_pool)[bobj] = q;
    }

    if(q->size() == 0)
    {
      // Need to make sure that registered report catchers are added. This
      // way users don't need to set up uvm_report_object as a super type.
      uvm_report_object* o;
      o = dynamic_cast<uvm_report_object*>(obj);
      if( o != nullptr )
      {
        uvm_queue<uvm_callback*>* qr;

        uvm_callbacks<uvm_report_object, uvm_callback>::get();
        qr = uvm_callbacks<uvm_report_object, uvm_callback>::m_t_inst->m_tw_cb_q;
        for(int i=0; i<qr->size(); ++i)
          q->push_back(qr->get(i));
      }

      for( int i=0; i<uvm_typed_callbacks<T>::m_t_inst->m_tw_cb_q->size(); ++i)
        q->push_back(uvm_typed_callbacks<T>::m_t_inst->m_tw_cb_q->get(i));
    }

    //check if already exists in the queue
    if( uvm_typed_callbacks<T>::m_cb_find(q, cb) != -1)
    {
      uvm_report_warning("CBPREG", "Callback object " + cb->get_name() + " is already registered" +
          " with object " + obj->get_full_name(), UVM_NONE);
    }
    else
    {
      uvm_typed_callbacks<T>::m_cb_find_name(q, cb->get_name(), "object instance " + obj->get_full_name() );
      if(ordering == UVM_APPEND)
        q->push_back(cb);
      else
        q->push_front(cb);
    }
  }
}


//----------------------------------------------------------------------
// member function: add_by_name (static)
//
// Registers the given callback object, ~cb~, with one or more uvm_components.
// The components must already exist and must be type T or a derivative. As
// with <add> the CB parameter is optional. ~root~ specifies the location in
// the component hierarchy to start the search for ~name~. See <uvm_root::find_all>
// for more details on searching by name.
//----------------------------------------------------------------------

template <typename T, typename CB>
void uvm_callbacks<T,CB>::add_by_name( const std::string& name,
                                       uvm_callback* cb,
                                       uvm_component* root,
                                       uvm_apprepend ordering )
{
  std::vector<uvm_component*> cq;
  uvm_root* top = nullptr;
  uvm_coreservice_t* cs = nullptr;
  T* t = nullptr;

  get();

  top = cs->get_root();

  if(cb == nullptr)
  {
     uvm_report_error("CBUNREG", "nullptr callback object cannot be registered with object(s) " +
       name, UVM_NONE);
     return;
  }

  std::ostringstream str;
  str << "Add ("
      << uvm_apprepend_name[ordering]
      << ") callback "
      << cb->get_name()
      << " by name to object(s) "
      << name;
  UVM_CB_TRACE_NOOBJ(cb, str.str())

  top->find_all(name, cq, root);
  if( cq.size() == 0)
  {
    uvm_report_warning("CBNOMTC", "add_by_name failed to find any components matching the name " +
      name + ", callback " + cb->get_name() + " will not be registered.", UVM_NONE);
  }

  for( unsigned int i = 0; i < cq.size(); i++ )
  {
    t = dynamic_cast<T*>(cq[i]);
    if(t != nullptr)
      add(t, cb, ordering);
  }
}


//----------------------------------------------------------------------
// member function: do_delete (was: delete)  (static)
//
// Deletes the given callback object, ~cb~, from the queue associated with
//  the given ~obj~ handle. The ~obj~ handle can be null, which allows
// de-registration of callbacks without an object context.
// The ~cb~ is the callback handle; it must be non-null, and if the callback
// has already been removed from the object instance then a warning is
// issued. Note that the CB parameter is optional. For example, the
// following are equivalent:
//
//| uvm_callbacks<my_comp>::do_delete(comp_a, cb);
//| uvm_callbacks<my_comp, my_callback>::do_delete(comp_a,cb);
//----------------------------------------------------------------------

template <typename T, typename CB>
void uvm_callbacks<T,CB>::do_delete( T* obj, uvm_callback* cb )
{
  uvm_object* b_obj;
  uvm_queue<uvm_callback*>* q  = nullptr;
  bool found = false;
  int pos = -1;

  get();

  if(obj == nullptr)
  {
    UVM_CB_TRACE_NOOBJ(cb,"Delete typewide callback " + cb->get_name() + " for type "
                     + m_base_inst.m_typename )
    found = uvm_typed_callbacks<T>::m_t_inst->m_delete_tw_cbs(cb);
  }
  else
  {
    b_obj = dynamic_cast<uvm_object*>(obj);

    if (b_obj==nullptr)
      UVM_FATAL("CB/DELETE", "Cannot delete this callback object.");

    UVM_CB_TRACE_NOOBJ(cb, "Delete callback " + cb->get_name() + " from object %0s "
                   + obj->get_full_name() )
    q = (*m_base_inst->m_pool)[b_obj];
    pos = uvm_typed_callbacks<T>::m_cb_find(q, cb);
    if(pos != -1)
    {
      q->do_delete(pos);
      found = true;
    }
  }
  if(!found)
  {
	  std::string nm;

    if(obj == nullptr)
      nm = "(*)";
    else
      nm = obj->get_full_name();

    uvm_report_warning("CBUNREG", "Callback " + cb->get_name() + " cannot be removed from object " +
        nm + " because it is not currently registered to that object.", UVM_NONE);
  }
}


//----------------------------------------------------------------------
// member function: delete_by_name (static)
//
// Removes the given callback object, ~cb~, associated with one or more
// uvm_component callback queues. As with <delete> the CB parameter is
// optional. ~root~ specifies the location in the component hierarchy to start
// the search for ~name~. See <uvm_root::find_all> for more details on searching
// by name.
//----------------------------------------------------------------------

template <typename T, typename CB>
void uvm_callbacks<T,CB>::delete_by_name( const std::string& name,
                                          uvm_callback* cb,
                                          uvm_component* root )
{
  std::vector<uvm_component*> cq;
  uvm_root* top = nullptr;
  uvm_coreservice_t* cs = nullptr;
  T* t = nullptr;

  get();

  top = cs->get_root();

  UVM_CB_TRACE_NOOBJ(cb, "Delete callback " + cb->get_name() + " by name from object(s) "
                     + name )

  top->find_all(name, cq, root);

  if(cq.size() == 0)
  {
    uvm_report_warning("CBNOMTC",
        "delete_by_name failed to find any components matching the name " +
      name + ", callback " + cb->get_name() + " will not be unregistered.", UVM_NONE);
  }

  for( unsigned int i = 0; i < cq.size(); i++ )
  {
    t = dynamic_cast<T*>(cq[i]);
    if( t != nullptr)
       do_delete(t, cb);
  }
}


//----------------------------------------------------------------------
// Group: Iterator Interface
//
// This set of functions provide an iterator interface for callback queues. A facade
// class, <uvm_callback_iter> is also available, and is the generally preferred way to
// iterate over callback queues.
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// member function: get_first (static)
//
// Returns the first enabled callback of type CB which resides in the queue for ~obj~.
// If ~obj~ is null then the typewide queue for T is searched. ~itr~ is the iterator;
// it will be updated with a value that can be supplied to <get_next> to get the next
// callback object.
//
// If the queue is empty then null is returned.
//
// The iterator class <uvm_callback_iter> may be used as an alternative, simplified,
// iterator interface.
//----------------------------------------------------------------------

template <typename T, typename CB>
CB* uvm_callbacks<T,CB>::get_first( int& itr, T* obj )
{
  uvm_queue<uvm_callback*>* q = nullptr;
  CB* cb = nullptr;

  get();
  m_get_q(q, obj);

  for(itr = 0; itr < q->size(); ++itr)
  {
    cb = dynamic_cast<CB*>(q->get(itr));
    if( (cb != nullptr) && cb->callback_mode() )
       return cb;
  }

  return nullptr;
}

//----------------------------------------------------------------------
// member function: get_last (static)
//
// Returns the last enabled callback of type CB which resides in the queue for ~obj~.
// If ~obj~ is null then the typewide queue for T is searched. ~itr~ is the iterator;
// it will be updated with a value that can be supplied to <get_prev> to get the previous
// callback object.
//
// If the queue is empty then null is returned.
//
// The iterator class <uvm_callback_iter> may be used as an alternative, simplified,
// iterator interface.
//----------------------------------------------------------------------

template <typename T, typename CB>
CB* uvm_callbacks<T,CB>::get_last( int& itr, T* obj )
{
  uvm_queue<uvm_callback*>* q = nullptr;
  CB* cb = nullptr;

  get();
  m_get_q(q, obj);

  for(itr = q->size()-1; itr>=0; --itr)
  {
    cb = dynamic_cast<CB*>(q->get(itr));
    if ( (cb != nullptr) && cb->callback_mode() )
       return cb;
  }

  return nullptr;
}


//----------------------------------------------------------------------
// member function: get_next (static)
//
// Returns the next enabled callback of type CB which resides in the queue for ~obj~,
// using ~itr~ as the starting point. If ~obj~ is null then the typewide queue for T
// is searched. ~itr~ is the iterator; it will be updated with a value that can be
// supplied to <get_next> to get the next callback object.
//
// If no more callbacks exist in the queue, then null is returned. <get_next> will
// continue to return null in this case until <get_first> or <get_last> has been used to reset
// the iterator.
//
// The iterator class <uvm_callback_iter> may be used as an alternative, simplified,
// iterator interface.
//----------------------------------------------------------------------

template <typename T, typename CB>
CB* uvm_callbacks<T,CB>::get_next( int& itr, T* obj )
{
  uvm_queue<uvm_callback*>* q = nullptr;
  CB* cb = nullptr;

  get();
  m_get_q(q, obj);

  for( itr = itr+1; itr < q->size(); ++itr)
  {
    cb = dynamic_cast<CB*>(q->get(itr));
    if ( (cb != nullptr) && cb->callback_mode() )
       return cb;
  }

  return nullptr;
}

//----------------------------------------------------------------------
// member function: get_prev (static)
//
// Returns the previous enabled callback of type CB which resides in the queue for ~obj~,
// using ~itr~ as the starting point. If ~obj~ is null then the typewide queue for T
// is searched. ~itr~ is the iterator; it will be updated with a value that can be
// supplied to <get_prev> to get the previous callback object.
//
// If no more callbacks exist in the queue, then null is returned. <get_prev> will
// continue to return null in this case until <get_first> or <get_last> has been used to reset
// the iterator.
//
// The iterator class <uvm_callback_iter> may be used as an alternative, simplified,
// iterator interface.
//----------------------------------------------------------------------

template <typename T, typename CB>
CB* uvm_callbacks<T,CB>::get_prev( int& itr, T* obj )
{
  uvm_queue<uvm_callback*>* q = nullptr;
  CB* cb = nullptr;

  get();
  m_get_q(q, obj);

  for( itr = itr-1; itr >= 0; --itr)
  {
    cb = dynamic_cast<CB*>(q->get(itr));
    if( (cb != nullptr) && cb->callback_mode() )
       return cb;
  }

  return nullptr;
}

//----------------------------------------------------------------------
// Group: Debug
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: display
//
// This function displays callback information for ~obj~. If ~obj~ is
// null, then it displays callback information for all objects
// of type ~T~, including typewide callbacks.
//----------------------------------------------------------------------

template <typename T, typename CB>
void uvm_callbacks<T,CB>::display( T* obj  )
{
  get();
  base_type::display(obj);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------
// member function: get (static)
//
// Implementation defined
//----------------------------------------------------------------------

template <typename T, typename CB>
uvm_callbacks<T,CB>* uvm_callbacks<T,CB>::get()
{
  if (m_inst == nullptr)
  {
    uvm_typeid_base* cb_base_type = nullptr;

    base_type::m_initialize();

    cb_base_type = uvm_typeid<uvm_callback>::get();
    m_cb_typeid  = uvm_typeid<CB>::get();
    m_typeid     = uvm_typeid<T>::get();

    m_inst = new uvm_callbacks<T,CB>();

    if (cb_base_type == m_cb_typeid)
    {
      m_base_inst = dynamic_cast< uvm_callbacks<T, uvm_callback>* >(m_inst);

      // The base inst in the super class gets set to this base inst
      uvm_typed_callbacks<T>::m_t_inst = m_base_inst;
      uvm_typeid_base::set_cb(m_typeid, m_inst);
      uvm_typeid_base::set_typeid(uvm_callbacks_base::m_b_inst, m_typeid);
    }
    else
    {
      m_base_inst = uvm_callbacks<T,uvm_callback>::get();
      m_base_inst->m_this_type.push_back(m_inst);
    }

    if (m_inst == nullptr)
      UVM_FATAL("CB/INTERNAL","get(): m_inst is nullptr");
  }

  return m_inst;
}

//----------------------------------------------------------------------
// member function: m_get_q (static)
//
// Implementation defined
//----------------------------------------------------------------------

template <typename T, typename CB>
void uvm_callbacks<T,CB>::m_get_q( uvm_queue<uvm_callback*>*& q, T* obj )
{
  uvm_object* bobj = nullptr;
  bobj =  dynamic_cast<uvm_object*>(obj);

  if (bobj == nullptr)
    UVM_FATAL("CB/INTERNAL","Cannot retrieve callback queue.");

  if(m_base_inst->m_pool->find(bobj) == m_base_inst->m_pool->end()) //no instance specific
  {
    q = ((bobj == nullptr) ? uvm_typed_callbacks<T>::m_t_inst->m_tw_cb_q : uvm_typed_callbacks<T>::m_t_inst->m_get_tw_cb_q(bobj));
  }
  else
  {
    q = (*m_base_inst->m_pool)[bobj];

    if(q == nullptr)
    {
      q = new uvm_queue<uvm_callback*>();
      (*m_base_inst->m_pool)[bobj] = q;
    }
  }
}

//----------------------------------------------------------------------
// member function: m_register_pair (static)
//
// Implementation defined
//
// Register valid callback type
//----------------------------------------------------------------------

template <typename T, typename CB>
bool uvm_callbacks<T,CB>::m_register_pair( const std::string& tname, const std::string& cbname )
{
  this_type* inst = get();

  m_typename = tname;
  base_type::m_typename = tname;
  m_typeid->type_name = tname;

  m_cb_typename = cbname;
  m_cb_typeid->type_name = cbname;

  inst->m_registered = true;

  return true;
}

//----------------------------------------------------------------------
// member function: m_is_registered (virtual)
//
// Implementation defined
//----------------------------------------------------------------------

template <typename T, typename CB>
bool uvm_callbacks<T,CB>::m_is_registered( uvm_object* obj, uvm_callback* cb )
{
  if( m_is_for_me(cb) && uvm_typed_callbacks<T>::m_am_i_a(obj) )
    return m_registered;

  return false;
}

//----------------------------------------------------------------------
// member function: m_is_for_me (virtual)
//
// Implementation defined
// Does type check to see if the callback is valid for this type
//----------------------------------------------------------------------

template <typename T, typename CB>
bool uvm_callbacks<T,CB>::m_is_for_me( uvm_callback* cb )
{
  CB* this_cb = nullptr;
  this_cb = dynamic_cast<CB*>(cb);
  return( this_cb != nullptr );
}


} // namespace uvm

#endif // UVM_CALLBACKS_H_
