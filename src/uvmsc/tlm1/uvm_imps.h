//----------------------------------------------------------------------
//   Copyright 2014 NXP B.V.
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

#ifndef UVM_IMPS_H_
#define UVM_IMPS_H_

#include <systemc>
#include <tlm.h>

//#include "base/uvm_globals.h"
//#include "macros/uvm_message_defines.h"

namespace uvm {

//---------------------------------------------------------------------------
// macros for the imp implementations
//
// The UVM put/get/peek API has been implemented, as well as the SystemC
// equivalent functions for compatibility reasons
//---------------------------------------------------------------------------

#define UVM_BLOCKING_PUT_IMP_METHODS \
  void put( const T& val ) { m_imp->put(val); }

#define UVM_BLOCKING_GET_IMP_METHODS \
  void get( T& val )  { val = m_imp->get(); } \
  T get( tlm::tlm_tag<T> *t = 0 ) { return m_imp->get(); }

#define UVM_BLOCKING_PEEK_IMP_METHODS \
  void peek( T& val ) const { val = m_imp->peek(); } \
  T peek( tlm::tlm_tag<T> *t = 0 ) const { return m_imp->peek(); }

#define UVM_NONBLOCKING_PUT_IMP_METHODS \
  bool try_put(const T& val) { return m_imp->try_put(val); } \
  bool can_put() { return m_imp->can_put(); } \
  bool nb_put(const T& val) { return m_imp->try_put(val); } \
  bool nb_can_put() { return m_imp->can_put(); } \
  bool nb_can_put( tlm::tlm_tag<T> *t = 0 ) const { return m_imp->can_put(); } \
  const sc_core::sc_event& ok_to_put( tlm::tlm_tag<T> *t = 0 ) const { return this->e; /* non-functional in uvmsc */ }

#define UVM_NONBLOCKING_GET_IMP_METHODS \
  bool try_get( T& val ) { return m_imp->try_get(val); } \
  bool can_get() { return m_imp->can_get(); } \
  bool nb_get( T& val ) { return m_imp->try_get(val); } \
  bool nb_can_get() { return m_imp->can_get(); } \
  bool nb_can_get( tlm::tlm_tag<T> *t = 0 ) const { return m_imp->can_get(); } \
  const sc_core::sc_event& ok_to_get( tlm::tlm_tag<T> *t = 0 ) const { return this->e; /* non-functional in uvmsc */ }

#define UVM_NONBLOCKING_PEEK_IMP_METHODS \
  bool try_peek( T& val ) const { return m_imp->try_peek(val); } \
  bool can_peek() const { return m_imp->can_peek(); } \
  bool nb_peek( T& val ) const { return m_imp->try_peek(val); } \
  bool nb_can_peek() const { return m_imp->can_peek(); } \
  bool nb_can_peek( tlm::tlm_tag<T> *t = 0 ) const { return m_imp->can_peek(); } \
  const sc_core::sc_event& ok_to_peek( tlm::tlm_tag<T> *t = 0 ) const { return this->e; /* non-functional in uvmsc */ }

#define UVM_BIND_TO_INTERFACE(IF) \
  IF* interface = dynamic_cast< IF* >(this); \
  this->m_connect(*interface);

#define BINDIMP_MSG "Not allowed to bind interface or port to IMP "

#define UVM_NO_BIND_AND_CONNECT_METHODS \
  template <typename IF> void connect( IF& ) { UVM_FATAL("BINDIMP", BINDIMP_MSG + this->get_name()); } \
  template <typename IF> void connect( uvm_export_base<IF>& ) { UVM_FATAL("BINDIMP", BINDIMP_MSG + this->get_name()); } \
  template <typename IF> void bind( IF& ) { UVM_FATAL("BINDIMP", BINDIMP_MSG + this->get_name()); } \
  template <typename IF> void bind( uvm_export_base<IF>& ) { UVM_FATAL("BINDIMP", BINDIMP_MSG + this->get_name()); } \
  template <typename IF> void operator()( IF& ) { UVM_FATAL("BINDIMP", BINDIMP_MSG + this->get_name()); } \
  template <typename IF> void operator()( uvm_export_base<IF>& ) { UVM_FATAL("BINDIMP", BINDIMP_MSG + this->get_name()); }

//---------------------------------------------------------------------------
// TLM1 blocking implementation ports (imps)
//---------------------------------------------------------------------------

template <typename T = int, typename IMP = int>
class uvm_blocking_put_imp
: public uvm_export_base< tlm::tlm_blocking_put_if<T> >,
  public virtual tlm::tlm_blocking_put_if<T>
{
  using uvm::uvm_export_base< tlm::tlm_blocking_put_if<T> >::connect;
  using sc_core::sc_export< tlm::tlm_blocking_put_if<T> >::bind;

 public:
  uvm_blocking_put_imp( IMP* imp ) :
    uvm_export_base< tlm::tlm_blocking_put_if<T> >(), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_blocking_put_if<T>)
  }

  uvm_blocking_put_imp( const std::string& name, IMP* imp ) :
    uvm_export_base< tlm::tlm_blocking_put_if<T> >( name ), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_blocking_put_if<T>)
  }

  UVM_BLOCKING_PUT_IMP_METHODS

  UVM_NO_BIND_AND_CONNECT_METHODS

  virtual const std::string get_type_name() const
  {
    return "uvm::uvm_blocking_put_imp";
  }

  virtual ~uvm_blocking_put_imp(){};

 private:
  IMP* m_imp;
};


template <typename T = int, typename IMP = int>
class uvm_blocking_get_imp
: public uvm_export_base< tlm::tlm_blocking_get_if<T> >,
  public virtual tlm::tlm_blocking_get_if<T>
{
 public:
  uvm_blocking_get_imp( IMP* imp ) :
    uvm_export_base< tlm::tlm_blocking_get_if<T> >(), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_blocking_get_if<T>)
  }

  uvm_blocking_get_imp( const std::string& name, IMP* imp ) :
    uvm_export_base< tlm::tlm_blocking_get_if<T> >( name ), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_blocking_get_if<T>)
  }

  UVM_BLOCKING_GET_IMP_METHODS

  UVM_NO_BIND_AND_CONNECT_METHODS

  virtual const std::string get_type_name() const
  {
    return "uvm::uvm_blocking_get_imp";
  }

  virtual ~uvm_blocking_get_imp(){}

 private:
  IMP* m_imp;
};


template <typename T = int, typename IMP = int>
class uvm_blocking_peek_imp
: public uvm_export_base< tlm::tlm_blocking_peek_if<T> >,
  public virtual tlm::tlm_blocking_peek_if<T>
{
  using uvm::uvm_export_base< tlm::tlm_blocking_peek_if<T> >::connect;
  using sc_core::sc_export< tlm::tlm_blocking_peek_if<T> >::bind;

 public:
  uvm_blocking_peek_imp( IMP* imp ) :
    uvm_export_base< tlm::tlm_blocking_peek_if<T> >(), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_blocking_peek_if<T>)
  }

  uvm_blocking_peek_imp( const std::string& name, IMP* imp ) :
    uvm_export_base< tlm::tlm_blocking_peek_if<T> >( name ), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_blocking_peek_if<T>)
  }

  UVM_BLOCKING_PEEK_IMP_METHODS

  UVM_NO_BIND_AND_CONNECT_METHODS

  virtual const std::string get_type_name() const
  {
    return "uvm::uvm_blocking_peek_imp";
  }

  virtual ~uvm_blocking_peek_imp(){}

 private:
  IMP* m_imp;
};

template <typename T = int, typename IMP = int>
class uvm_blocking_get_peek_imp
: public uvm_export_base< tlm::tlm_blocking_get_peek_if<T> >,
  public virtual tlm::tlm_blocking_get_peek_if<T>
{
 public:

  uvm_export_base< tlm::tlm_blocking_get_peek_if<T> > inp;

  uvm_blocking_get_peek_imp( IMP* imp ) :
    uvm_export_base< tlm::tlm_blocking_get_peek_if<T> >(), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_blocking_get_peek_if<T>)
  }

  uvm_blocking_get_peek_imp( const std::string& name, IMP* imp ) :
    uvm_export_base< tlm::tlm_blocking_get_peek_if<T> >( name ), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_blocking_get_peek_if<T>)
  }

  virtual const std::string get_type_name() const
  {
    return "uvm::uvm_blocking_get_peek_imp";
  }

  UVM_BLOCKING_GET_IMP_METHODS
  UVM_BLOCKING_PEEK_IMP_METHODS

  UVM_NO_BIND_AND_CONNECT_METHODS

  virtual ~uvm_blocking_get_peek_imp(){}

 private:
  IMP* m_imp;
};

//---------------------------------------------------------------------------
// TLM1 non-blocking port imps
//---------------------------------------------------------------------------

template <typename T = int, typename IMP = int>
class uvm_nonblocking_put_imp
: public uvm_export_base< tlm::tlm_nonblocking_put_if<T> >,
  public virtual tlm::tlm_nonblocking_put_if<T>
{
 public:
  uvm_nonblocking_put_imp( IMP* imp ) :
    uvm_export_base< tlm::tlm_nonblocking_put_if<T> >(), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_nonblocking_put_if<T>)
  }

  uvm_nonblocking_put_imp( const std::string& name, IMP* imp ) :
    uvm_export_base< tlm::tlm_nonblocking_put_if<T> >( name ), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_nonblocking_put_if<T>)
  }

  UVM_NONBLOCKING_PUT_IMP_METHODS

  UVM_NO_BIND_AND_CONNECT_METHODS

  virtual const std::string get_type_name() const
  {
    return "uvm::uvm_nonblocking_put_imp";
  }

  virtual ~uvm_nonblocking_put_imp(){}

 private:
  IMP* m_imp;
  sc_core::sc_event e; /* non-functional in uvmsc */
};


template <typename T = int, typename IMP = int>
class uvm_nonblocking_get_imp
: public uvm_export_base< tlm::tlm_nonblocking_get_if<T> >,
  public virtual tlm::tlm_nonblocking_get_if<T>
{
 public:
  uvm_nonblocking_get_imp( IMP* imp ) :
    uvm_export_base< tlm::tlm_nonblocking_get_if<T> >(), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_nonblocking_get_if<T>)
  }

  uvm_nonblocking_get_imp( const std::string& name, IMP* imp ) :
    uvm_export_base< tlm::tlm_nonblocking_get_if<T> >( name ), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_nonblocking_get_if<T>)
  }

  UVM_NONBLOCKING_GET_IMP_METHODS

  UVM_NO_BIND_AND_CONNECT_METHODS

  virtual const std::string get_type_name() const
  {
    return "uvm::uvm_nonblocking_get_imp";
  }

  virtual ~uvm_nonblocking_get_imp(){}

 private:
  IMP* m_imp;
  sc_core::sc_event e; /* non-functional in uvmsc */
};

template <typename T = int, typename IMP = int>
class uvm_nonblocking_peek_imp
: public uvm_export_base< tlm::tlm_nonblocking_peek_if<T> >,
  public virtual tlm::tlm_nonblocking_peek_if<T>
{
 public:
  uvm_nonblocking_peek_imp( IMP* imp ) :
    uvm_export_base< tlm::tlm_nonblocking_peek_if<T> >(), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_nonblocking_peek_if<T>)
  }

  uvm_nonblocking_peek_imp( const std::string& name, IMP* imp ) :
    uvm_export_base< tlm::tlm_nonblocking_peek_if<T> >( name ), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_nonblocking_peek_if<T>)
  }

  UVM_NONBLOCKING_PEEK_IMP_METHODS

  UVM_NO_BIND_AND_CONNECT_METHODS

  virtual const std::string get_type_name() const
  {
    return "uvm::uvm_nonblocking_peek_imp";
  }

  virtual ~uvm_nonblocking_peek_imp(){}

 private:
  IMP* m_imp;
  sc_core::sc_event e; /* non-functional in uvmsc */
};

template <typename T = int, typename IMP = int>
class uvm_nonblocking_get_peek_imp
: public uvm_export_base< tlm::tlm_nonblocking_get_peek_if<T> >,
  public virtual tlm::tlm_nonblocking_get_peek_if<T>
{
 public:
  uvm_nonblocking_get_peek_imp( IMP* imp ) :
    uvm_export_base< tlm::tlm_nonblocking_get_peek_if<T> >(), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_nonblocking_get_peek_if<T>)
  }

  uvm_nonblocking_get_peek_imp( const std::string& name, IMP* imp ) :
    uvm_export_base< tlm::tlm_nonblocking_get_peek_if<T> >( name ), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_nonblocking_get_peek_if<T>)
  }

  UVM_NONBLOCKING_GET_IMP_METHODS
  UVM_NONBLOCKING_PEEK_IMP_METHODS

  UVM_NO_BIND_AND_CONNECT_METHODS

  virtual const std::string get_type_name() const
  {
    return "uvm::uvm_nonblocking_get_peek_imp";
  }

  virtual ~uvm_nonblocking_get_peek_imp(){}

 private:
  IMP* m_imp;
  sc_core::sc_event e; /* non-functional in uvmsc */
};

//---------------------------------------------------------------------------
// TLM1 port imps (with contain both blocking as non-blocking methods
//---------------------------------------------------------------------------

template <typename T = int, typename IMP = int>
class uvm_put_imp
: public uvm_export_base< tlm::tlm_put_if<T> >,
  public virtual tlm::tlm_blocking_put_if<T>,
  public virtual tlm::tlm_nonblocking_put_if<T>
{
 public:
  uvm_put_imp( IMP* imp ) :
    uvm_export_base< tlm::tlm_put_if<T> >(), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_put_if<T>)
  }

  uvm_put_imp( const std::string& name, IMP* imp ) :
    uvm_export_base< tlm::tlm_put_if<T> >( name ), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_put_if<T>)
  }

  UVM_NONBLOCKING_PUT_IMP_METHODS
  UVM_BLOCKING_PUT_IMP_METHODS

  UVM_NO_BIND_AND_CONNECT_METHODS

  virtual const std::string get_type_name() const
  {
    return "uvm::uvm_put_imp";
  }

  virtual ~uvm_put_imp(){}

 private:
  IMP* m_imp;
  sc_core::sc_event e; /* non-functional in uvmsc */
};

template <typename T = int, typename IMP = int>
class uvm_get_imp
: public uvm_export_base< tlm::tlm_get_if<T> >,
  public virtual tlm::tlm_blocking_get_if<T>,
  public virtual tlm::tlm_nonblocking_get_if<T>
{
 public:
  uvm_get_imp( IMP* imp ) :
    uvm_export_base< tlm::tlm_get_if<T> >(), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_get_if<T>)
  }

  uvm_get_imp( const std::string& name, IMP* imp ) :
    uvm_export_base< tlm::tlm_get_if<T> >( name ), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_get_if<T>)
  }

  UVM_NONBLOCKING_GET_IMP_METHODS
  UVM_BLOCKING_GET_IMP_METHODS

  UVM_NO_BIND_AND_CONNECT_METHODS

  virtual const std::string get_type_name() const
  {
    return "uvm::uvm_get_imp";
  }

  virtual ~uvm_get_imp(){}

 private:
  IMP* m_imp;
  sc_core::sc_event e; /* non-functional in uvmsc */
};

template <typename T = int, typename IMP = int>
class uvm_peek_imp
: public uvm_export_base< tlm::tlm_peek_if<T> >,
  public virtual tlm::tlm_blocking_peek_if<T>,
  public virtual tlm::tlm_nonblocking_peek_if<T>
{
 public:
  uvm_peek_imp( IMP* imp ) :
    uvm_export_base< tlm::tlm_peek_if<T> >(), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_peek_if<T>)
  }

  uvm_peek_imp( const std::string& name, IMP* imp ) :
    uvm_export_base< tlm::tlm_peek_if<T> >( name ), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_peek_if<T>)
  }

  UVM_NONBLOCKING_PEEK_IMP_METHODS
  UVM_BLOCKING_PEEK_IMP_METHODS

  UVM_NO_BIND_AND_CONNECT_METHODS

  virtual const std::string get_type_name() const
  {
    return "uvm::uvm_peek_imp";
  }

  virtual ~uvm_peek_imp(){}

 private:
  IMP* m_imp;
  sc_core::sc_event e; /* non-functional in uvmsc */
};

template <typename T = int, typename IMP = int>
class uvm_get_peek_imp
: public uvm_export_base< tlm::tlm_get_peek_if<T> >,
  public virtual tlm::tlm_blocking_get_peek_if<T>,
  public virtual tlm::tlm_nonblocking_get_peek_if<T>
{
 public:
  uvm_get_peek_imp( IMP* imp ) :
    uvm_export_base< tlm::tlm_get_peek_if<T> >(), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_get_peek_if<T>)
  }

  uvm_get_peek_imp( const std::string& name, IMP* imp ) :
    uvm_export_base< tlm::tlm_get_peek_if<T> >( name ), m_imp(imp)
  {
    UVM_BIND_TO_INTERFACE(tlm::tlm_get_peek_if<T>)
  }

  UVM_NONBLOCKING_GET_IMP_METHODS
  UVM_BLOCKING_GET_IMP_METHODS
  UVM_NONBLOCKING_PEEK_IMP_METHODS
  UVM_BLOCKING_PEEK_IMP_METHODS

  UVM_NO_BIND_AND_CONNECT_METHODS

  virtual const std::string get_type_name() const
  {
    return "uvm::uvm_get_peek_imp";
  }

  virtual ~uvm_get_peek_imp(){}

 private:
  IMP* m_imp;
  sc_core::sc_event e; /* non-functional in uvmsc */
};


} // namespace uvm

#endif /* UVM_IMPS_H_ */
