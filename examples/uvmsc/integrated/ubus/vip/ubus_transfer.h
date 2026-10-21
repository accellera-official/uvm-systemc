//----------------------------------------------------------------------
//   Copyright 2016-2019 NXP B.V.
//   Copyright 2007-2010 Mentor Graphics Corporation
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
//----------------------------------------------------------------------

#ifndef UBUS_TRANSFER_H_
#define UBUS_TRANSFER_H_

#include <systemc>
#include <uvm>

#include "ubus_defines.h"

//----------------------------------------------------------------------
// ubus transfer enums, parameters, and events
//----------------------------------------------------------------------

typedef enum {
  NOP,
  READ,
  WRITE
} ubus_read_write_enum;

extern const char *ubus_read_write_name[];

//----------------------------------------------------------------------
// Class: ubus_transfer
//----------------------------------------------------------------------

class ubus_transfer : public uvm::uvm_sequence_item
{
  using uvm_sequence_item::uvm_report;
  using uvm_sequence_item::uvm_report_info;
  using uvm_sequence_item::uvm_report_warning;
  using uvm_sequence_item::uvm_report_error;
  using uvm_sequence_item::uvm_report_fatal;

public:
  // TODO Randomization
  /* rand */ sc_dt::sc_uint<16> addr;
  /* rand */ ubus_read_write_enum read_write;
  /* rand */ unsigned int size;
  /* rand */ sc_dt::sc_uint<8> data[MAXSIZE];
  /* rand */ sc_dt::sc_uint<4> wait_state[MAXSIZE];
  /* rand */ unsigned int error_pos;
  /* rand */ unsigned int transmit_delay;

  std::string master;
  std::string slave;

  /* TODO constraints
  constraint c_read_write {
    read_write inside { READ, WRITE };
  }
  constraint c_size {
    size inside {1,2,4,8};
  }
  constraint c_data_wait_size {
    data.size() == size;
    wait_state.size() == size;
  }
  constraint c_transmit_delay { 
    transmit_delay <= 10 ; 
  }
  */

  UVM_OBJECT_UTILS(ubus_transfer);

  /* TODO no field macros; add specific pack/unpack methods
    `uvm_field_int      (addr,           UVM_DEFAULT)
    `uvm_field_enum     (ubus_read_write_enum, read_write, UVM_DEFAULT)
    `uvm_field_int      (size,           UVM_DEFAULT)
    `uvm_field_array_int(data,           UVM_DEFAULT)
    `uvm_field_array_int(wait_state,     UVM_DEFAULT)
    `uvm_field_int      (error_pos,      UVM_DEFAULT)
    `uvm_field_int      (transmit_delay, UVM_DEFAULT)
    `uvm_field_string   (master,         UVM_DEFAULT|UVM_NOCOMPARE)
    `uvm_field_string   (slave,          UVM_DEFAULT|UVM_NOCOMPARE)
  */

  ubus_transfer(const std::string& name = "ubus_transfer_inst");
  void do_print(const uvm::uvm_printer& printer) const override;
  void do_pack(uvm::uvm_packer& p) const override;
  void do_unpack(uvm::uvm_packer& p) override;
  void do_copy(const uvm::uvm_object& rhs) override;
  bool do_compare(const uvm::uvm_object& rhs, const uvm::uvm_comparer* comparer = nullptr ) const override;
  std::string convert2string() const override;

};

#endif /* UBUS_TRANSFER_H_ */
