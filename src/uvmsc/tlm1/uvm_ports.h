//----------------------------------------------------------------------
//   Copyright 2012-2013 NXP B.V.
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

#ifndef UVM_PORTS_H_
#define UVM_PORTS_H_

#include <systemc>
#include <tlm>

#include <iostream>
#include <string>
#include <typeinfo>

#include "uvmsc/base/uvm_port_base.h"
namespace uvm {

//---------------------------------------------------------------------------
// macros for the port implementations
// The UVM put/get/peek API has been implemented, as well as the SystemC
// equivalent functions for compatibility reasons
//---------------------------------------------------------------------------

#define UVM_PORT_COMMON(IF) \
  const IF* get_imp() const { \
    const IF* imp = dynamic_cast< const IF* >(this->get_interface()); \
    if (imp == nullptr) UVM_FATAL("TLM1BIND", "No interface bound to TLM port '" + this->get_full_name() + "'."); \
      return imp; } \
  IF* get_imp() { \
    IF* imp = dynamic_cast< IF* >(this->get_interface()); \
    if (imp == nullptr) UVM_FATAL("TLM1BIND", "No interface bound to TLM port '" + this->get_full_name() + "'."); \
      return imp; }

#define UVM_BLOCKING_PUT_PORT_METHODS \
  void put( const T& val ) { get_imp()->put(val); }

#define UVM_BLOCKING_GET_PORT_METHODS \
  void get( T& val )  { val = get_imp()->get(); } \
  T get( tlm::tlm_tag<T> *t = 0 ) { return get_imp()->get(); }

#define UVM_BLOCKING_PEEK_PORT_METHODS \
  void peek( T& val ) const { val = get_imp()->peek(); } \
  T peek( tlm::tlm_tag<T> *t = 0 ) const { return get_imp()->peek(); }

#define UVM_NONBLOCKING_PUT_PORT_METHODS \
  bool try_put(const T& val) { return get_imp()->nb_put(val); } \
  bool can_put() const { return get_imp()->nb_can_put(); } \
  bool nb_put(const T& val) { return get_imp()->nb_put(val); } \
  bool nb_can_put() const { return get_imp()->nb_can_put(); } \
  bool nb_can_put( tlm::tlm_tag<T> *t = 0 ) const { return get_imp()->nb_can_put(); } \
  const sc_core::sc_event& ok_to_put( tlm::tlm_tag<T> *t = 0 ) const { return get_imp()->ok_to_put(); }

#define UVM_NONBLOCKING_GET_PORT_METHODS \
  bool try_get( T& val ) { return get_imp()->nb_get(val); } \
  bool can_get() const { return get_imp()->nb_can_get(); } \
  bool nb_get( T& val ) { return get_imp()->nb_get(val); } \
  bool nb_can_get() const { return get_imp()->nb_can_get(); } \
  bool nb_can_get( tlm::tlm_tag<T> *t = 0 ) const { return get_imp()->nb_can_get(); } \
  const sc_core::sc_event& ok_to_get( tlm::tlm_tag<T> *t = 0 ) const { return get_imp()->ok_to_get(); }

#define UVM_NONBLOCKING_PEEK_PORT_METHODS \
  bool try_peek( T& val ) const { return get_imp()->nb_peek(val); } \
  bool can_peek() const { return get_imp()->nb_can_peek(); } \
  bool nb_peek( T& val ) const { return get_imp()->nb_peek(val); } \
  bool nb_can_peek() const { return get_imp()->nb_can_peek(); } \
  bool nb_can_peek( tlm::tlm_tag<T> *t = 0 ) const { return get_imp()->nb_can_peek(); } \
  const sc_core::sc_event& ok_to_peek( tlm::tlm_tag<T> *t = 0 ) const { return get_imp()->ok_to_peek(); }

//---------------------------------------------------------------------------
// TLM1 blocking ports
//---------------------------------------------------------------------------

template <typename T>
class uvm_blocking_put_port
: public uvm_port_base< tlm::tlm_blocking_put_if<T> >
{
 public:
  uvm_blocking_put_port() :
    uvm_port_base< tlm::tlm_blocking_put_if<T> >(){}

  uvm_blocking_put_port( const std::string& name ) :
    uvm_port_base< tlm::tlm_blocking_put_if<T> >( name ) {}

  UVM_PORT_COMMON(tlm::tlm_blocking_put_if<T>)
  UVM_BLOCKING_PUT_PORT_METHODS

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_blocking_put_port";
  }

  virtual ~uvm_blocking_put_port() {};
};


template < typename T >
class uvm_blocking_get_port
: public uvm_port_base< tlm::tlm_blocking_get_if<T> >
{
 public:
  uvm_blocking_get_port() :
    uvm_port_base< tlm::tlm_blocking_get_if<T> >() {}

  uvm_blocking_get_port( const std::string& name ) :
    uvm_port_base< tlm::tlm_blocking_get_if<T> >( name ) {}

  UVM_PORT_COMMON(tlm::tlm_blocking_get_if<T>)
  UVM_BLOCKING_GET_PORT_METHODS

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_blocking_get_port";
  }

  virtual ~uvm_blocking_get_port(){}

};

template < typename T >
class uvm_blocking_peek_port
: public uvm_port_base< tlm::tlm_blocking_peek_if<T> >
{
 public:
  uvm_blocking_peek_port() :
    uvm_port_base< tlm::tlm_blocking_peek_if<T> >() {}

  uvm_blocking_peek_port( const std::string& name ) :
    uvm_port_base< tlm::tlm_blocking_peek_if<T> >( name ) {}

  UVM_PORT_COMMON(tlm::tlm_blocking_peek_if<T>)
  UVM_BLOCKING_PEEK_PORT_METHODS

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_blocking_peek_port";
  }

  virtual ~uvm_blocking_peek_port(){}
};

template < typename T >
class uvm_blocking_get_peek_port
: public uvm_port_base< tlm::tlm_blocking_get_peek_if<T> >
{
 public:
  uvm_blocking_get_peek_port() :
    uvm_port_base< tlm::tlm_blocking_get_peek_if<T> >() {}

  uvm_blocking_get_peek_port( const std::string& name ) :
    uvm_port_base< tlm::tlm_blocking_get_peek_if<T> >( name ) {}

  UVM_PORT_COMMON(tlm::tlm_blocking_get_peek_if<T>)
  UVM_BLOCKING_GET_PORT_METHODS
  UVM_BLOCKING_PEEK_PORT_METHODS

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_blocking_get_peek_port";
  }

  virtual ~uvm_blocking_get_peek_port(){}
};

//---------------------------------------------------------------------------
// TLM1 non-blocking ports
//---------------------------------------------------------------------------

template <typename T>
class uvm_nonblocking_put_port
: public uvm_port_base< tlm::tlm_nonblocking_put_if<T> >
{
 public:
  uvm_nonblocking_put_port() :
    uvm_port_base< tlm::tlm_nonblocking_put_if<T> >() {}

  uvm_nonblocking_put_port( const std::string& name ) :
    uvm_port_base< tlm::tlm_nonblocking_put_if<T> >( name ) {}

  UVM_PORT_COMMON(tlm::tlm_nonblocking_put_if<T>)
  UVM_NONBLOCKING_PUT_PORT_METHODS

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_nonblocking_put_port";
  }

  virtual ~uvm_nonblocking_put_port() {};
};


template < typename T >
class uvm_nonblocking_get_port
: public uvm_port_base< tlm::tlm_nonblocking_get_if<T> >
{
 public:
  uvm_nonblocking_get_port() :
    uvm_port_base< tlm::tlm_nonblocking_get_if<T> >() {}

  uvm_nonblocking_get_port( const std::string& name ) :
    uvm_port_base< tlm::tlm_nonblocking_get_if<T> >( name ) {}

  UVM_PORT_COMMON(tlm::tlm_nonblocking_get_if<T>)
  UVM_NONBLOCKING_GET_PORT_METHODS

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_nonblocking_get_port";
  }

  virtual ~uvm_nonblocking_get_port(){}

};

template < typename T >
class uvm_nonblocking_peek_port
: public uvm_port_base< tlm::tlm_nonblocking_peek_if<T> >
{
 public:
  uvm_nonblocking_peek_port() :
    uvm_port_base< tlm::tlm_nonblocking_peek_if<T> >() {}

  uvm_nonblocking_peek_port( const std::string& name ) :
    uvm_port_base< tlm::tlm_nonblocking_peek_if<T> >( name ) {}

  UVM_PORT_COMMON(tlm::tlm_nonblocking_peek_if<T>)
  UVM_NONBLOCKING_PEEK_PORT_METHODS

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_nonblocking_peek_port";
  }

  virtual ~uvm_nonblocking_peek_port(){}
};

template < typename T >
class uvm_nonblocking_get_peek_port
: public uvm_port_base< tlm::tlm_nonblocking_get_peek_if<T> >
{
 public:
  uvm_nonblocking_get_peek_port() :
    uvm_port_base< tlm::tlm_nonblocking_get_peek_if<T> >() {}

  uvm_nonblocking_get_peek_port( const std::string& name ) :
    uvm_port_base< tlm::tlm_nonblocking_get_peek_if<T> >( name ) {}

  UVM_PORT_COMMON(tlm::tlm_nonblocking_get_peek_if<T>)
  UVM_NONBLOCKING_GET_PORT_METHODS
  UVM_NONBLOCKING_PEEK_PORT_METHODS

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_nonblocking_get_peek_port";
  }

  virtual ~uvm_nonblocking_get_peek_port(){}
};

//---------------------------------------------------------------------------
// TLM1 ports (with contain both blocking as non-blocking methods)
//---------------------------------------------------------------------------

template <typename T>
class uvm_put_port
: public uvm_port_base< tlm::tlm_put_if<T> >
{
 public:
  uvm_put_port() :
    uvm_port_base< tlm::tlm_put_if<T> >() {}

  uvm_put_port( const std::string& name ) :
    uvm_port_base< tlm::tlm_put_if<T> >( name ) {}

  UVM_PORT_COMMON(tlm::tlm_put_if<T>)
  UVM_NONBLOCKING_PUT_PORT_METHODS
  UVM_BLOCKING_PUT_PORT_METHODS

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_put_port";
  }

  virtual ~uvm_put_port() {};
};

template < typename T >
class uvm_get_port
: public uvm_port_base< tlm::tlm_get_if<T> >
{
 public:
  uvm_get_port() :
    uvm_port_base< tlm::tlm_get_if<T> >() {}

  uvm_get_port( const std::string& name ) :
    uvm_port_base< tlm::tlm_get_if<T> >( name ) {}

  UVM_PORT_COMMON(tlm::tlm_get_if<T>)
  UVM_NONBLOCKING_GET_PORT_METHODS
  UVM_BLOCKING_GET_PORT_METHODS

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_get_port";
  }

  virtual ~uvm_get_port(){}

};

template < typename T >
class uvm_peek_port
: public uvm_port_base< tlm::tlm_peek_if<T> >
{
 public:
  uvm_peek_port() :
    uvm_port_base< tlm::tlm_peek_if<T> >() {}

  uvm_peek_port( const std::string& name ) :
    uvm_port_base< tlm::tlm_peek_if<T> >( name ) {}

  UVM_PORT_COMMON(tlm::tlm_peek_if<T>)
  UVM_NONBLOCKING_PEEK_PORT_METHODS
  UVM_BLOCKING_PEEK_PORT_METHODS

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_peek_port";
  }

  virtual ~uvm_peek_port(){}
};

template < typename T >
class uvm_get_peek_port
: public uvm_port_base< tlm::tlm_get_peek_if<T> >
{
 public:
  uvm_get_peek_port() :
    uvm_port_base< tlm::tlm_get_peek_if<T> >() {}

  uvm_get_peek_port( const std::string& name ) :
    uvm_port_base< tlm::tlm_get_peek_if<T> >( name ) {}

  UVM_PORT_COMMON(tlm::tlm_get_peek_if<T>)
  UVM_NONBLOCKING_GET_PORT_METHODS
  UVM_BLOCKING_GET_PORT_METHODS
  UVM_NONBLOCKING_PEEK_PORT_METHODS
  UVM_BLOCKING_PEEK_PORT_METHODS

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_get_peek_port";
  }

  virtual ~uvm_get_peek_port(){}
};

} // namespace uvm

#endif /* UVM_PORTS_H_ */
