//-----------------------------------------------------------------------------
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2013 NXP B.V.
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

#ifndef UVM_RECORDER_H_
#define UVM_RECORDER_H_

#include <systemc>

#include "uvmsc/macros/uvm_object_defines.h"
#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/base/uvm_object.h"
#include "uvmsc/factory/uvm_object_registry.h"

namespace uvm {


// forward class declarations
class uvm_scope_stack;

//------------------------------------------------------------------------------
// CLASS: uvm_recorder
//
//! The class #uvm_recorder shall provide a policy object for recording objects of
//! type #uvm_object.
//! The policies determine how recording should be done.
//!
//! A default recorder instance, #uvm_default_recorder, is used when the
//! uvm_object::record is called without specifying a recorder.
//----------------------------------------------------------------------------


class uvm_recorder : public uvm_object
{
 public:

  UVM_OBJECT_UTILS(uvm_recorder);

  uvm_recorder( const std::string& name = "uvm_recorder" );

  ~uvm_recorder() override;

  virtual void record_field( const std::string& name,
                             uvm_bitstream_t value,
                             int size,
                             uvm_radix_enum radix = UVM_NORADIX);

  virtual void record_field_real( const std::string& name,
                                  double value );

  virtual void record_object( const std::string& name,
                              uvm_object* value );

  virtual void record_string( const std::string& name, const std::string& value );

  virtual void record_time( const std::string& name, const sc_core::sc_time& value );

  virtual void record_generic( const std::string& name, const std::string& value );


  //--------------------------------------------------------------------------
  // data member: tr_handle
  //
  //! This is an integral handle to a transaction object. Its use is vendor
  //! specific.
  //! A handle of 0 indicates there is no active transaction object.
  //--------------------------------------------------------------------------

  int tr_handle; // default: 0

  //--------------------------------------------------------------------------
  // data member: default_radix
  //
  //! This is the default radix setting if #record_field is called without
  //! a radix.
  //--------------------------------------------------------------------------

  uvm_radix_enum default_radix; // default = UVM_HEX;

  //--------------------------------------------------------------------------
  // data member: physical
  //
  //! This boolean variable provides a filtering mechanism for fields.
  //! The #abstract and physical settings allow an object to distinguish between
  //! two different classes of fields.
  //! It is up to you, in the uvm_object::do_record method, to test the
  //! setting of this field if you want to use the physical trait as a filter.
  //--------------------------------------------------------------------------

  bool physical; // default = true;

  //--------------------------------------------------------------------------
  // data member: abstract
  //
  //! This boolean variable provides a filtering mechanism for fields.
  //!
  //! The abstract and physical settings allow an object to distinguish between
  //! two different classes of fields.
  //!
  //! It is up to you, in the uvm_object::do_record method, to test the
  //! setting of this field if you want to use the abstract trait as a filter.
  //--------------------------------------------------------------------------

  bool abstract; // default = true;

  //--------------------------------------------------------------------------
  // data member: identifier
  //
  //! This data member is used to specify whether or not an object's reference should be
  //! recorded when the object is recorded.
  //--------------------------------------------------------------------------

  bool identifier; // default = true;

  //--------------------------------------------------------------------------
  // data member: recursion_policy
  //
  //! Sets the recursion policy for recording objects.
  //!
  //! The default policy is deep (which means to recurse an object).
  //--------------------------------------------------------------------------

  uvm_recursion_policy_enum policy; // default = UVM_DEFAULT_POLICY;

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

  int recording_depth; // TODO move to private?

 private:


  std::ofstream file;
  std::ostream* outstr;
  std::string filename;

  uvm_scope_stack* scope;

  static int handle;

  static std::map<int, bool>& m_handles();

  virtual bool open_file();

  virtual int create_stream( const std::string& name,
                             const std::string& t,
                             const std::string& scope );

  virtual void m_set_attribute( int txh,
                                const std::string& nm,
                                const std::string& value );

  virtual void set_attribute( int txh,
                              const std::string& nm,
                              const std::string& value, // TODO was logic [1023:0]
                              uvm_radix_enum radix,
                              int numbits = 1024);

  virtual bool check_handle_kind( const std::string& htype, int handle );

  virtual int begin_tr( const std::string& txtype,
                        int stream,
                        const std::string& nm,
                        const std::string& label = "",
                        const std::string& desc = "",
                        const sc_core::sc_time begin_time = sc_core::SC_ZERO_TIME );

  virtual void end_tr( int handle, const sc_core::sc_time end_time = sc_core::SC_ZERO_TIME );

  virtual void link_tr( int h1,
                        int h2,
                        const std::string& relation = "" );

  virtual void free_tr( int handle );

}; // class uvm_recorder

} // namespace uvm
  
  
#endif /* UVM_RECORDER_H_ */
