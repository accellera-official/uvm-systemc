//-----------------------------------------------------------------------------
//   Copyright 2013 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc.
//   Copyright 2010 Synopsys, Inc.
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

//-----------------------------------------------------------------------------
// Title: Sequence Item Pull Ports
//
// This section defines the port, export, and imp port classes for
// communicating sequence items between uvm_sequencer<REQ,RSP> and
// uvm_driver<REQ,RSP>.
//-----------------------------------------------------------------------------

#ifndef UVM_SQR_CONNECTIONS_H_
#define UVM_SQR_CONNECTIONS_H_

#include "uvmsc/base/uvm_port_base.h"
#include "uvmsc/base/uvm_export_base.h"
#include "uvmsc/seq/uvm_sequencer_ifs.h"

namespace uvm {

#define UVM_SEQ_ITEM_PULL_IMP(imp, REQ, RSP, req_arg, rsp_arg) \
  void disable_auto_item_recording() { imp->disable_auto_item_recording(); } \
  bool is_auto_item_recording_enabled() { return imp->is_auto_item_recording_enabled(); } \
  void get_next_item( REQ& req_arg ) { imp->get_next_item(req_arg); } \
  bool try_next_item( REQ& req_arg ) { return imp->try_next_item(req_arg); } \
  void item_done() { imp->item_done(); } \
  void item_done( const RSP& rsp_arg ) { imp->item_done(rsp_arg); } \
  void wait_for_sequences() { imp->wait_for_sequences(); } \
  bool has_do_available() { return imp->has_do_available(); } \
  void put_response( const RSP& rsp_arg ) { imp->put_response(rsp_arg); } \
  void get( REQ& req_arg ) { imp->get(req_arg); } \
  REQ get() { return imp->get(); } \
  void peek( REQ& req_arg ) { imp->peek(req_arg); } \
  REQ peek() { return imp->peek(); } \
  void put( const RSP& rsp_arg ) { imp->put(rsp_arg); }

//-----------------------------------------------------------------------------
// Class: uvm_seq_item_pull_port<REQ,RSP>
//
//! UVM provides a port, export, and imp connector for use in sequencer-driver
//! communication. All have standard port connector constructors, except that
//! #uvm_seq_item_pull_port's default min_size argument is 0; it can be left
//! unconnected.
//-----------------------------------------------------------------------------


template <typename REQ = int, typename RSP = REQ>
class uvm_seq_item_pull_port : public uvm_port_base< uvm_sqr_if_base<REQ, RSP> >
{
 public:
  typedef uvm_port_base<uvm_sqr_if_base<REQ, RSP> > this_type;

  uvm_seq_item_pull_port() : uvm_port_base< uvm_sqr_if_base<REQ, RSP> >() {}
  uvm_seq_item_pull_port( const char *nm ) : uvm_port_base< uvm_sqr_if_base<REQ, RSP> >(nm) {}
  virtual ~uvm_seq_item_pull_port(){}

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_seq_item_pull_port";
  }

  UVM_SEQ_ITEM_PULL_IMP(this->get_interface(0), REQ, RSP, req, rsp)
};

//-----------------------------------------------------------------------------
// Class: uvm_seq_item_pull_export<REQ,RSP>
//
// This export type is used in sequencer-driver communication. It has the
// standard constructor for exports.
//-----------------------------------------------------------------------------

template <typename REQ = int, typename RSP = REQ>
class uvm_seq_item_pull_export : public uvm_export_base<uvm_sqr_if_base<REQ, RSP> >
{
  uvm_seq_item_pull_export() : uvm_export_base< uvm_sqr_if_base<REQ, RSP> >() {}
  uvm_seq_item_pull_export( const char *nm ) : uvm_export_base< uvm_sqr_if_base<REQ, RSP> >(nm) {}
  virtual ~uvm_seq_item_pull_export(){}

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_seq_item_pull_export";
  }

  // TODO - check: an export shall not redirect interface methods, this is only for ports and imps
  //UVM_SEQ_ITEM_PULL_IMP(this->get_interface(), REQ, RSP, req, rsp)
};

//-----------------------------------------------------------------------------
// Class: uvm_seq_item_pull_imp<REQ,RSP,IMP>
//
// This imp type is used in sequencer-driver communication. It has the
// standard constructor for imp-type ports.
//-----------------------------------------------------------------------------

template <typename REQ = int, typename RSP = REQ, typename IMP = int>
class uvm_seq_item_pull_imp : public uvm_export_base<uvm_sqr_if_base<REQ, RSP> >
{
 public:
  uvm_seq_item_pull_imp() : uvm_export_base< uvm_sqr_if_base<REQ, RSP> >() {}
  uvm_seq_item_pull_imp( const char *nm ) : uvm_export_base< uvm_sqr_if_base<REQ, RSP> >(nm) {}
  virtual ~uvm_seq_item_pull_imp(){}

  virtual std::string get_type_name() const
  {
    return "uvm::uvm_seq_item_pull_imp";
  }

  UVM_SEQ_ITEM_PULL_IMP(IMP::get_interface(), REQ, RSP, req, rsp)

};

} // namespace uvm

#endif // UVM_SQR_CONNECTIONS_H_
