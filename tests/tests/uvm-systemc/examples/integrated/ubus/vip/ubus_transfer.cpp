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

#include <systemc>
#include <uvm>

#include <algorithm>

#include "ubus_transfer.h"

const char* ubus_read_write_name[] = {
    "NOP",
    "READ",
    "WRITE"
};

ubus_transfer::ubus_transfer( const std::string& name )
: uvm::uvm_sequence_item(name)
{
  addr = 0;
  read_write = NOP;
  size = 0;

  std::fill(data,data+MAXSIZE,0);
  std::fill(wait_state,wait_state+MAXSIZE,0);

  error_pos = 0;
  transmit_delay = 0;
  master.clear();
  slave.clear();
}

void ubus_transfer::do_print(const uvm::uvm_printer& printer) const
{
  printer.print_field_int("addr", addr.to_uint());
  printer.print_string("read_write", ubus_read_write_name[read_write]);
  printer.print_field_int("size", size);

  for(unsigned int i = 0; i < size; i++)
  {
    char data_idx[10];
    std::sprintf(data_idx, "data[%d]", i);
    printer.print_field_int(data_idx, data[i].to_uint());
  }

  for(unsigned int i = 0; i < size; i++)
  {
    char wait_state_idx[20];
    std::sprintf(wait_state_idx, "wait_state[%d]", i);
    printer.print_field_int(wait_state_idx, wait_state[i].to_uint());
  }
  printer.print_field_int("error_pos", error_pos);
  printer.print_field_int("transmit_delay", transmit_delay);
  printer.print_string("master", master);
  printer.print_string("slave", slave);
}

void ubus_transfer::do_pack(uvm::uvm_packer& p) const
{
  p << addr;
  p << (int)read_write;
  p << size;

  for(unsigned int i = 0; i < size; i++)
    p << data[i];

  for(unsigned int i = 0; i < size; i++)
    p << wait_state[i];

  p << error_pos;
  p << transmit_delay;
  p << master;
  p << slave;
}

void ubus_transfer::do_unpack(uvm::uvm_packer& p)
{
  int rw;

  p >> addr;
  p >> rw;  read_write = (ubus_read_write_enum)rw;
  p >> size;

  for(unsigned int i = 0; i < size; i++)
    p >> data[i];

  for(unsigned int i = 0; i < size; i++)
    p >> wait_state[i];

  p >> error_pos;
  p >> transmit_delay;
  p >> master;
  p >> slave;
}

void ubus_transfer::do_copy(const uvm::uvm_object& rhs)
{
  const ubus_transfer* drhs = dynamic_cast<const ubus_transfer*>(&rhs);

  if (!drhs)
    UVM_FATAL("DO_COPY", "Object not of type ubus_transfer");

  addr = drhs->addr;
  read_write = drhs->read_write;
  size = drhs->size;
  for(unsigned int i = 0; i < size; i++) data[i] = drhs->data[i];
  for(unsigned int i = 0; i < size; i++) wait_state[i] = drhs->wait_state[i];
  error_pos = drhs->error_pos;
  transmit_delay = drhs->transmit_delay;
  master = drhs->master;
  slave = drhs->slave;
}

bool ubus_transfer::do_compare(const uvm::uvm_object& rhs, const uvm::uvm_comparer* comparer) const
{
  const ubus_transfer* drhs = dynamic_cast<const ubus_transfer*>(&rhs);

  if (!drhs)
    UVM_FATAL("DO_COPY", "Object not of type ubus_transfer");

  bool data_equal = true;
  bool wait_state_equal = true;

  for(unsigned int i = 0; i < size; i++)
  {
    if (data[i] != drhs->data[i]) data_equal = false;
    if (wait_state[i] != drhs->wait_state[i]) wait_state_equal = false;
  }

  return ((addr == drhs->addr) && (read_write == drhs->read_write) &&
    (size == drhs->size) && (data_equal) &&
    (wait_state_equal) && (error_pos == drhs->error_pos) &&
    (transmit_delay == drhs->transmit_delay));

  // Note: master and slave string deliberately not in compare
}

std::string ubus_transfer::convert2string() const
{
  std::ostringstream str;
  str << " addr: " << addr;
  str << " read_write: " << ubus_read_write_name[read_write];
  str << " size: " << size;

  for(unsigned int i = 0; i < size; i++)
    str << " data[" << i << "]: " << data[i];

  for(unsigned int i = 0; i < size; i++)
    str << " wait_state[" << i << "]: " << wait_state[i];

  str << " error_pos: " << error_pos;
  str << " transmit_delay: " << transmit_delay;
  str << " master: " << master;
  str << " slave: " << slave;

  return str.str();
}
