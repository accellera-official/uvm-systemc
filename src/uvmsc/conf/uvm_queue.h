//------------------------------------------------------------------------------
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2007-2010 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc.
//   Copyright 2010 Synopsys, Inc.
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
//------------------------------------------------------------------------------

#ifndef UVM_QUEUE_H_
#define UVM_QUEUE_H_

#include <string>
#include <sstream>
#include <list>

#include "uvmsc/base/uvm_object.h"
#include "uvmsc/base/uvm_globals.h"

namespace uvm {

class uvm_resource_base;


//------------------------------------------------------------------------------
// CLASS: uvm_queue<T>
//
//! Implements a class-based dynamic queue. Allows queues to be allocated on
//! demand, and passed and stored by reference.
//------------------------------------------------------------------------------

template <typename T = int>
class uvm_queue : public uvm_object
{
public:
  //typedef uvm_queue<T> this_type;

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  explicit uvm_queue( const std::string& name_ = "" );
  ~uvm_queue() override;

  static uvm_queue<T>* get_global_queue();
  static T get_global ( int index );

  virtual T get ( int index );

  virtual int size() const;

//  virtual void insert( int index, const T& item );

  virtual void do_delete( int index = -1 );

  virtual T pop_front();
  virtual T pop_back();
  virtual void push_front( const T& item );
  virtual void push_back( const T& item );

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

  uvm_object* create( const std::string& name = "" ) override;

  const std::string get_type_name() const override;

  void do_copy( const uvm_object& rhs ) override;

  std::string convert2string() const override;

 public:
  static const std::string type_name;
  static uvm_queue<T>* m_global_queue;

 protected:
  std::list<T> queue;
  typename std::list<T>::iterator queue_it;

}; // class uvm_queue


/////////////////////////////////////////
// Class implementation starts here
/////////////////////////////////////////


//------------------------------------------------------------------------------
// Initialization of static data members
//------------------------------------------------------------------------------

template <typename T>
const std::string uvm_queue<T>::type_name = "uvm::uvm_queue";

template <typename T>
uvm_queue<T>* uvm_queue<T>::m_global_queue = nullptr;

//------------------------------------------------------------------------------
// Constructors
//------------------------------------------------------------------------------

template <typename T>
uvm_queue<T>::uvm_queue( const std::string& name_ )
  : uvm_object( name_ )
{}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------

template <typename T>
uvm_queue<T>::~uvm_queue()
{
  if(m_global_queue)
    delete m_global_queue;
}

//------------------------------------------------------------------------------
// member function: get_global_queue (static)
//
//! Returns the singleton global queue for the item type, T.
//!
//! This allows items to be shared amongst components throughout the
//! verification environment.
//------------------------------------------------------------------------------

template <typename T>
uvm_queue<T>* uvm_queue<T>::get_global_queue()
{
  if (m_global_queue == nullptr)
    m_global_queue = new uvm_queue("global_queue");
  return m_global_queue;
}

//------------------------------------------------------------------------------
// member function: get_global (static)
//
//! Returns the specified item instance from the global item queue.
//------------------------------------------------------------------------------

template <typename T>
T uvm_queue<T>::get_global( int index )
{
  uvm_queue<T>* gqueue;
  gqueue = get_global_queue();
  return gqueue->get(index);
}


//------------------------------------------------------------------------------
// member function: get
//
//! Returns the item at the given \p index.
//! If no item exists by that key, a new item is created with that key
//! and returned.
//------------------------------------------------------------------------------

template <typename T>
T uvm_queue<T>::get( int index )
{
  T default_value = T(); //TODO check what happens for constructors which don't initialize (int);
  if (index >= size() || index < 0)
  {
    std::ostringstream msg;
    msg << "get: given index out of range for queue of size "
        << size() << ". Ignoring get request";
    uvm_report_warning("QUEUEGET", msg.str() );
    return default_value;
  }
  queue_it = queue.begin();
  for (int i = 0; i<index; i++) queue_it++;
  return (*queue_it);
}


//------------------------------------------------------------------------------
// member function: size
//
//! Returns the number of items stored in the queue.
//------------------------------------------------------------------------------

template <typename T>
int uvm_queue<T>::size() const
{
  return (int)queue.size();
}


//------------------------------------------------------------------------------
// member function: insert
//
//! Inserts the item at the given \p index in the queue.
//------------------------------------------------------------------------------

/* TODO insert item in queue
template <typename T>
void uvm_queue<T>::insert ( int index, T item )
{
  if (index >= size() || index < 0)
  {
    std::ostringstream msg;
    msg << "insert: given index out of range for queue of size "
        << size() << ". Ignoring insert request";
    uvm_report_warning("QUEUEINS", msg.str() );
    return;
  }
  queue_it = queue.begin();
  for (int i = 0; i<index; i++) queue_it++;
  queue.insert(queue_it,item);
}
*/

//------------------------------------------------------------------------------
// member function: do_delete (was called delete in UVM-SV)
//
//! Removes the item at the given \p index from the queue; if \p index is
//! not provided, the entire contents of the queue are deleted.
//!
//! note: original UVM function delete, but this is a reserved C++ word.
//------------------------------------------------------------------------------

template <typename T>
void uvm_queue<T>::do_delete( int index )
{
  if (index >= size() || index < -1)
  {
    std::ostringstream msg;
    msg << "do_delete: given index out of range for queue of size "
        << size() << ". Ignoring delete request";
    uvm_report_warning("QUEUEDEL", msg.str() );
    return;
  }
  if (index == -1)
    queue.clear();
  else
  {
    queue_it = queue.begin();
    for (int i=0; i<index; i++) queue_it++; //TODO check whether i=0 is correct as initialization
    queue.erase(queue_it);
  }
}


//------------------------------------------------------------------------------
// member function: pop_front
//
//! Removes and returns the first element of the queue,
//! or nullptr if the queue is empty.
//------------------------------------------------------------------------------

template <typename T>
T uvm_queue<T>::pop_front()
{
  if (queue.size() != 0 )
  {
    queue_it = queue.begin();
    T val = *queue_it;
    queue.pop_front();
    return val;
  }
  else
    return T(); // TODO shouldn't it be T();
}


//------------------------------------------------------------------------------
// member function: pop_back
//
//! Removes and returns the last element of the queue,
//! or nullptr if the queue is empty.
//------------------------------------------------------------------------------

template <typename T>
T uvm_queue<T>::pop_back()
{
  if (queue.size() != 0 )
  {
    queue_it = queue.end();
    T val = *queue_it;
    queue.pop_back();
    return val;
  }
  else
    return T(); // TODO shouldn't it be T();
}


//------------------------------------------------------------------------------
// member function: push_front
//
//! Inserts the given \p item at the front of the queue.
//------------------------------------------------------------------------------

template <typename T>
void uvm_queue<T>::push_front( const T& item )
{
  queue.push_front(item);
}

//------------------------------------------------------------------------------
// member function: push_back
//
//! Inserts the given \p item at the back of the queue.
//------------------------------------------------------------------------------

template <typename T>
void uvm_queue<T>::push_back( const T& item )
{
  queue.push_back(item);
}

//------------------------------------------------------------------------------
// member function: create
//
//! Creates and return a new queue with name \p name
//------------------------------------------------------------------------------

template <typename T>
uvm_object* uvm_queue<T>::create(const std::string& name )
{
  uvm_queue<T>* v;
  v = new uvm_queue(name);
  return v;
}

//------------------------------------------------------------------------------
// member function: get_type_name
//
//! Return the type name of the object
//------------------------------------------------------------------------------

template <typename T>
const std::string uvm_queue<T>::get_type_name() const
{
  return uvm_queue<T>::type_name;
}

//------------------------------------------------------------------------------
// member function: do_copy
//
//! Make copy of object
//------------------------------------------------------------------------------

template <typename T>
void uvm_queue<T>::do_copy(const uvm_object& rhs )
{
  uvm_object::do_copy(rhs);
  const uvm_queue<T>* p = dynamic_cast<const uvm_queue<T>*>(&rhs);
  if (p == nullptr)
    return;
  queue = p->queue;
}

//------------------------------------------------------------------------------
// member function: convert2string
//
//! Convert information in queue to string
//------------------------------------------------------------------------------

template <typename T>
std::string uvm_queue<T>::convert2string() const
{
  std::ostringstream str;
  typename std::list<T>::const_iterator queue_it;

  for( queue_it = queue.begin();
      queue_it != queue.end();
      queue_it++ )
    str << *queue_it;
  return str.str();
}


} // namespace uvm

#endif // UVM_QUEUE_H_

