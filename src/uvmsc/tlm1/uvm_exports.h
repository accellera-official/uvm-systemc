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

#ifndef UVM_EXPORTS_H_
#define UVM_EXPORTS_H_

#include <systemc>
#include <tlm>
#include "uvmsc/base/uvm_export_base.h"

namespace uvm {

//---------------------------------------------------------------------------
// TLM1 blocking exports
//---------------------------------------------------------------------------

template <typename T>
class uvm_blocking_put_export
: public uvm_export_base< tlm::tlm_blocking_put_if<T> >
{
 public:
  uvm_blocking_put_export() :
    uvm_export_base< tlm::tlm_blocking_put_if<T> >() {}

  uvm_blocking_put_export( const std::string& name ) :
    uvm_export_base< tlm::tlm_blocking_put_if<T> >( name ) {}

  // exports have no implementation

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_blocking_put_export";
  }

  virtual ~uvm_blocking_put_export(){};

};


template < typename T >
class uvm_blocking_get_export
: public uvm_export_base< tlm::tlm_blocking_get_if<T> >
{
 public:
  uvm_blocking_get_export() :
    uvm_export_base< tlm::tlm_blocking_get_if<T> >() {}

  uvm_blocking_get_export( const std::string& name ) :
    uvm_export_base< tlm::tlm_blocking_get_if<T> >( name ) {}

  // exports have no implementation

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_blocking_get_export";
  }

  virtual ~uvm_blocking_get_export(){}
};


template < typename T >
class uvm_blocking_peek_export
: public uvm_export_base< tlm::tlm_blocking_peek_if<T> >
{
 public:
  uvm_blocking_peek_export() :
    uvm_export_base< tlm::tlm_blocking_peek_if<T> >() {}

  uvm_blocking_peek_export( const std::string& name ) :
    uvm_export_base< tlm::tlm_blocking_peek_if<T> >( name ) {}

  // exports have no implementation

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_blocking_peek_export";
  }

  virtual ~uvm_blocking_peek_export(){}
};

template < typename T >
class uvm_blocking_get_peek_export
: public uvm_export_base< tlm::tlm_blocking_get_peek_if<T> >
{
 public:
  uvm_blocking_get_peek_export() :
    uvm_export_base< tlm::tlm_blocking_get_peek_if<T> >() {}

  uvm_blocking_get_peek_export( const std::string& name ) :
    uvm_export_base< tlm::tlm_blocking_get_peek_if<T> >( name ) {}

  // exports have no implementation

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_blocking_get_peek_export";
  }

  virtual ~uvm_blocking_get_peek_export(){}
};

//---------------------------------------------------------------------------
// TLM1 nonblocking exports
//---------------------------------------------------------------------------

template <typename T>
class uvm_nonblocking_put_export
: public uvm_export_base< tlm::tlm_nonblocking_put_if<T> >
{
 public:
  uvm_nonblocking_put_export() :
    uvm_export_base< tlm::tlm_nonblocking_put_if<T> >() {}

  uvm_nonblocking_put_export( const std::string& name ) :
    uvm_export_base< tlm::tlm_nonblocking_put_if<T> >( name ) {}

  // exports have no implementation

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_nonblocking_put_export";
  }

  virtual ~uvm_nonblocking_put_export(){};

};


template < typename T >
class uvm_nonblocking_get_export
: public uvm_export_base< tlm::tlm_nonblocking_get_if<T> >
{
 public:
  uvm_nonblocking_get_export() :
    uvm_export_base< tlm::tlm_nonblocking_get_if<T> >() {}

  uvm_nonblocking_get_export( const std::string& name ) :
    uvm_export_base< tlm::tlm_nonblocking_get_if<T> >( name ) {}

  // exports have no implementation

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_nonblocking_get_export";
  }

  virtual ~uvm_nonblocking_get_export(){}
};


template < typename T >
class uvm_nonblocking_peek_export
: public uvm_export_base< tlm::tlm_nonblocking_peek_if<T> >
{
 public:
  uvm_nonblocking_peek_export() :
    uvm_export_base< tlm::tlm_nonblocking_peek_if<T> >() {}

  uvm_nonblocking_peek_export( const std::string& name ) :
    uvm_export_base< tlm::tlm_nonblocking_peek_if<T> >( name ) {}

  // exports have no implementation

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_nonblocking_peek_export";
  }

  virtual ~uvm_nonblocking_peek_export(){}
};

template < typename T >
class uvm_nonblocking_get_peek_export
: public uvm_export_base< tlm::tlm_nonblocking_get_peek_if<T> >
{
 public:
  uvm_nonblocking_get_peek_export() :
    uvm_export_base< tlm::tlm_nonblocking_get_peek_if<T> >() {}

  uvm_nonblocking_get_peek_export( const std::string& name ) :
    uvm_export_base< tlm::tlm_nonblocking_get_peek_if<T> >( name ) {}

  // exports have no implementation

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_nonblocking_get_peek_export";
  }

  virtual ~uvm_nonblocking_get_peek_export(){}
};

//---------------------------------------------------------------------------
// TLM1 exports (with contain both blocking as non-blocking methods)
//---------------------------------------------------------------------------

template <typename T>
class uvm_put_export
: public uvm_export_base< tlm::tlm_put_if<T> >
{
 public:
  uvm_put_export() :
    uvm_export_base< tlm::tlm_put_if<T> >() {}

  uvm_put_export( const std::string& name ) :
    uvm_export_base< tlm::tlm_put_if<T> >( name ) {}

  // exports have no implementation

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_put_export";
  }

  virtual ~uvm_put_export(){};

};

template < typename T >
class uvm_get_export
: public uvm_export_base< tlm::tlm_get_if<T> >
{
 public:
  uvm_get_export() :
    uvm_export_base< tlm::tlm_get_if<T> >() {}

  uvm_get_export( const std::string& name ) :
    uvm_export_base< tlm::tlm_get_if<T> >( name ) {}

  // exports have no implementation

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_get_export";
  }

  virtual ~uvm_get_export(){}
};


template < typename T >
class uvm_peek_export
: public uvm_export_base< tlm::tlm_peek_if<T> >
{
 public:
  uvm_peek_export() :
    uvm_export_base< tlm::tlm_peek_if<T> >() {}

  uvm_peek_export( const std::string& name ) :
    uvm_export_base< tlm::tlm_peek_if<T> >( name ) {}

  // exports have no implementation

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_peek_export";
  }

  virtual ~uvm_peek_export(){}
};

template < typename T >
class uvm_get_peek_export
: public uvm_export_base< tlm::tlm_get_peek_if<T> >
{
 public:
  uvm_get_peek_export() :
    uvm_export_base< tlm::tlm_get_peek_if<T> >() {}

  uvm_get_peek_export( const std::string& name ) :
    uvm_export_base< tlm::tlm_get_peek_if<T> >( name ) {}

  // exports have no implementation

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_get_peek_export";
  }

  virtual ~uvm_get_peek_export(){}
};

} // namespace uvm

#endif /* UVM_EXPORTS_H_ */
