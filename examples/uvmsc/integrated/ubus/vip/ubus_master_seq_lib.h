//----------------------------------------------------------------------
//   Copyright 2016 NXP B.V.
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

#ifndef UBUS_MASTER_SEQ_LIB_H_
#define UBUS_MASTER_SEQ_LIB_H_

#include <systemc>
#include <uvm>

// forward class declaration
class ubus_transfer;

//------------------------------------------------------------------------------
//
// SEQUENCE: ubus_base_sequence
//
//------------------------------------------------------------------------------

// This sequence raises/drops objections in the pre/post_body so that root
// sequences raise objections but subsequences do not.

class ubus_base_sequence : public uvm::uvm_sequence<ubus_transfer>
{
public:

  ubus_base_sequence( const std::string& name = "ubus_base_seq")
  : uvm::uvm_sequence<ubus_transfer>(name)
  {}

  UVM_OBJECT_UTILS(ubus_base_sequence);

  // Raise in pre_body so the objection is only raised for root sequences.
  // There is no need to raise for sub-sequences since the root sequence
  // will encapsulate the sub-sequence. 

  virtual void pre_body()
  {
    if (starting_phase != NULL)
    {
      UVM_INFO(get_type_name(), get_sequence_path() +
        " pre_body() raising " + starting_phase->get_name() +
        " objection.", uvm::UVM_MEDIUM);

      starting_phase->raise_objection(this);
    }
  }

  // Drop the objection in the post_body so the objection is removed when
  // the root sequence is complete. 

  virtual void post_body()
  {
    if (starting_phase != NULL)
    {
      UVM_INFO(get_type_name(), get_sequence_path() +
        " post_body() dropping " + starting_phase->get_name() +
        " objection.", uvm::UVM_MEDIUM);

    starting_phase->drop_objection(this);
    }
  }
  
}; // class ubus_base_sequence

//------------------------------------------------------------------------------
//
// SEQUENCE: read_byte
//
//------------------------------------------------------------------------------

class read_byte_seq : public ubus_base_sequence
{
public:

  /* rand */ unsigned int start_addr;
  /* rand */ unsigned int transmit_del;

  // TODO constraints
  //constraint transmit_del_ct { (transmit_del <= 10); }

  read_byte_seq( const std::string& name = "read_byte_seq")
  : ubus_base_sequence(name), start_addr(0), transmit_del(0)
  {}

  UVM_OBJECT_UTILS(read_byte_seq);

  virtual void body()
  {
    /* TODO constraints using macro
    UVM_DO_WITH(req,
      { req.addr == start_addr;
        req.read_write == READ;
        req.size == 1;
        req.error_pos == 1000;
        req.transmit_delay == transmit_del; } )
    get_response(rsp);
    */

    ubus_transfer* req = new ubus_transfer();
    ubus_transfer* rsp = new ubus_transfer();

    // TODO no constraints yet, so we assign the values directly
    req->addr = start_addr;
    req->read_write = READ;
    req->size = 1;
    req->error_pos = 1000;
    req->transmit_delay = transmit_del;

    start_item(req);
    finish_item(req);
    get_response(rsp);

    std::ostringstream msg;
    msg << get_sequence_path()
        << " read : addr = 0x"
        << std::hex << rsp->addr.to_uint()
        << ", data[0] = 0x"
        << std::hex << rsp->data[0];
    UVM_INFO(get_type_name(), msg.str(), uvm::UVM_HIGH);
  }
  
}; // class read_byte_seq


//------------------------------------------------------------------------------
//
// SEQUENCE: read_half_word_seq
//
//------------------------------------------------------------------------------

class read_half_word_seq : public ubus_base_sequence
{
public:

  // TODO randomization
  /* rand */ unsigned int start_addr;
  /* rand */ unsigned int transmit_del;

  // TODO constraints
  //constraint transmit_del_ct { (transmit_del <= 10); }

  read_half_word_seq( const std::string& name = "read_half_word_seq")
  : ubus_base_sequence(name), start_addr(0), transmit_del(0)
  {}
  
  UVM_OBJECT_UTILS(read_half_word_seq);

  virtual void body()
  {
    /* TODO constraints using macro
    UVM_DO_WITH(req,
      { req.addr == start_addr;
        req.read_write == READ;
        req.size == 2;
        req.error_pos == 1000;
        req.transmit_delay == transmit_del; } )
    get_response(rsp);
    */

    ubus_transfer* req = new ubus_transfer();
    ubus_transfer* rsp = new ubus_transfer();

    // TODO no constraints yet, so we assign the values directly
    req->addr = start_addr;
    req->read_write = READ;
    req->size = 2;
    req->error_pos = 1000;
    req->transmit_delay = transmit_del;

    start_item(req);
    finish_item(req);
    get_response(rsp);

    std::cout << "************************************bla" << std::endl;
    std::ostringstream msg;
    msg << get_sequence_path()
        << " read : addr = 0x"
        << std::hex << rsp->addr.to_uint()
        << ", data[0] = 0x"
        << std::hex << rsp->data[0]
        << ", data[1] = 0x"
        << std::hex << rsp->data[1];
    UVM_INFO(get_type_name(), msg.str(), uvm::UVM_HIGH);
  }

}; // class read_half_word_seq


//------------------------------------------------------------------------------
//
// SEQUENCE: read_word_seq
//
//------------------------------------------------------------------------------

class read_word_seq : public ubus_base_sequence
{
public:

  // TODO randomization
  /* rand */ unsigned int start_addr;
  /* rand */ unsigned int transmit_del;

  // TODO constraints
  //constraint transmit_del_ct { (transmit_del <= 10); }

  read_word_seq( const std::string& name = "read_word_seq")
  : ubus_base_sequence(name), start_addr(0), transmit_del(0)
  {}
  
  UVM_OBJECT_UTILS(read_word_seq);

  virtual void body()
  {
    /* TODO constraints using macro
    UVM_DO_WITH(req,
      { req.addr == start_addr;
        req.read_write == READ;
        req.size == 4;
        req.error_pos == 1000;
        req.transmit_delay == transmit_del; } )
    get_response(rsp);
    */

    ubus_transfer* req = new ubus_transfer();
    ubus_transfer* rsp = new ubus_transfer();

    // TODO no constraints yet, so we assign the values directly
    req->addr = start_addr;
    req->read_write = READ;
    req->size = 4;
    req->error_pos = 1000;
    req->transmit_delay = transmit_del;

    start_item(req);
    finish_item(req);
    get_response(rsp);

    std::ostringstream msg;
    msg << get_sequence_path()
        << " read : addr = 0x"
        << std::hex << rsp->addr.to_uint()
        << ", data[0] = 0x"
        << std::hex << rsp->data[0]
        << ", data[1] = 0x"
        << std::hex << rsp->data[1]
        << ", data[2] = 0x"
        << std::hex << rsp->data[2]
        << ", data[3] = 0x"
        << std::hex << rsp->data[3];

    UVM_INFO(get_type_name(), msg.str(), uvm::UVM_HIGH);
  }
  
}; // class read_word_seq


//------------------------------------------------------------------------------
//
// SEQUENCE: read_double_word_seq
//
//------------------------------------------------------------------------------

class read_double_word_seq : public ubus_base_sequence
{
public:

  // TODO randomization
  /* rand */ unsigned int start_addr;
  /* rand */ unsigned int transmit_del;

  // TODO constraints
  //constraint transmit_del_ct { (transmit_del <= 10); }

  read_double_word_seq( const std::string& name = "read_double_word_seq")
  : ubus_base_sequence(name), start_addr(0), transmit_del(0)
  {}
  
  UVM_OBJECT_UTILS(read_double_word_seq);

  virtual void body()
  {
    /* TODO constraints using macro
    UVM_DO_WITH(req,
      { req.addr == start_addr;
        req.read_write == READ;
        req.size == 8;
        req.error_pos == 1000;
        req.transmit_delay == transmit_del; } )
    get_response(rsp);
    */
    ubus_transfer* req = new ubus_transfer();
    ubus_transfer* rsp = new ubus_transfer();

    // TODO no constraints yet, so we assign the values directly
    req->addr = start_addr;
    req->read_write = READ;
    req->size = 8;
    req->error_pos = 1000;
    req->transmit_delay = transmit_del;

    start_item(req);
    finish_item(req);
    get_response(rsp);

    std::ostringstream msg;
    msg << get_sequence_path()
        << " read : addr = 0x"
        << std::hex << rsp->addr.to_uint()
        << ", data[0] = 0x"
        << std::hex << rsp->data[0]
        << ", data[1] = 0x"
        << std::hex << rsp->data[1]
        << ", data[2] = 0x"
        << std::hex << rsp->data[2]
        << ", data[3] = 0x"
        << std::hex << rsp->data[3]
        << ", data[4] = 0x"
        << std::hex << rsp->data[4]
        << ", data[5] = 0x"
        << std::hex << rsp->data[5]
        << ", data[6] = 0x"
        << std::hex << rsp->data[6]
        << ", data[7] = 0x"
        << std::hex << rsp->data[7];

      UVM_INFO(get_type_name(), msg.str(), uvm::UVM_HIGH);
    }
  
}; // class read_double_word_seq


//------------------------------------------------------------------------------
//
// SEQUENCE: write_byte_seq
//
//------------------------------------------------------------------------------

class write_byte_seq : public ubus_base_sequence
{
public:

  // TODO randomization
  /* rand */ unsigned int start_addr;
  /* rand */ unsigned int data0;
  /* rand */ unsigned int transmit_del;

  // TODO constraints
  //constraint transmit_del_ct { (transmit_del <= 10); }

  write_byte_seq( const std::string& name = "write_byte_seq")
  : ubus_base_sequence(name), start_addr(0), data0(0), transmit_del(0)
  {}

  UVM_OBJECT_UTILS(write_byte_seq);

  virtual void body()
  {
    /*
    uvm_do_with(req,
      { req.addr == start_addr;
        req.read_write == WRITE;
        req.size == 1;
        req.data[0] == data0;
        req.error_pos == 1000;
        req.transmit_delay == transmit_del; } )
    */

    ubus_transfer* req = new ubus_transfer();

    // TODO no constraints yet, so we assign the values directly
    req->addr = start_addr;
    req->read_write = WRITE;
    req->size = 1;
    req->data.push_back(data0);
    req->error_pos = 1000;
    req->transmit_delay = transmit_del;

    std::ostringstream msg;
    msg << get_sequence_path()
        << " wrote : addr = 0x"
        << std::hex << req->addr.to_uint()
        << ", data[0] = 0x"
        << std::hex << req->data[0];
    UVM_INFO(get_type_name(), msg.str(), uvm::UVM_HIGH);
  }

}; // class write_byte_seq


//------------------------------------------------------------------------------
//
// SEQUENCE: write_half_word_seq
//
//------------------------------------------------------------------------------

class write_half_word_seq : public ubus_base_sequence
{
public:

  // TODO randomization
  /* rand */ unsigned int start_addr;
  /* rand */ unsigned int data0;
  /* rand */ unsigned int data1;
  /* rand */ unsigned int transmit_del;

  // TODO constraints
  // constraint transmit_del_ct { transmit_del <= 10; }

  write_half_word_seq( const std::string& name = "write_half_word_seq")
  : ubus_base_sequence(name), start_addr(0), data0(0), data1(0), transmit_del(0)
  {}

  UVM_OBJECT_UTILS(write_half_word_seq);

  virtual void body()
  {
    /*
    uvm_do_with(req,
      { req.addr == start_addr;
        req.read_write == WRITE;
        req.size == 2;
        req.data[0] == data0; req.data[1] == data1;
        req.error_pos == 1000;
        req.transmit_delay == transmit_del; } )
    */

    ubus_transfer* req = new ubus_transfer();

    // TODO no constraints yet, so we assign the values directly
    req->addr = start_addr;
    req->read_write = WRITE;
    req->size = 2;
    req->data[0] = data0;
    req->data[1] = data1;
    req->error_pos = 1000;
    req->transmit_delay = transmit_del;

    std::ostringstream msg;
    msg << get_sequence_path()
        << " wrote : addr = 0x"
        << std::hex << req->addr.to_uint()
        << ", data[0] = 0x"
        << std::hex << req->data[0]
        << ", data[1] = 0x"
        << std::hex << req->data[1];
    UVM_INFO(get_type_name(), msg.str(), uvm::UVM_HIGH);
  }

}; // class write_half_word_seq


//------------------------------------------------------------------------------
//
// SEQUENCE: write_word_seq
//
//------------------------------------------------------------------------------

class write_word_seq : public ubus_base_sequence
{
public:

  // TODO randomization
  /* rand */ unsigned int start_addr;
  /* rand */ unsigned int data0;
  /* rand */ unsigned int data1;
  /* rand */ unsigned int data2;
  /* rand */ unsigned int data3;
  /* rand */ unsigned int transmit_del;

  // TODO constraints
  // constraint transmit_del_ct { (transmit_del <= 10); }

  write_word_seq( const std::string& name = "write_word_seq")
  : ubus_base_sequence(name), start_addr(0), data0(0),
    data1(0), data2(0), data3(0), transmit_del(0)
  {}

  UVM_OBJECT_UTILS(write_word_seq);


  virtual void body()
  {
    /*
    UVM_DO_WITH(req,
      { req.addr == start_addr;
        req.read_write == WRITE;
        req.size == 4;
        req.data[0] == data0; req.data[1] == data1;
        req.data[2] == data2; req.data[3] == data3;
        req.error_pos == 1000;
        req.transmit_delay == transmit_del; } )
    */

    ubus_transfer* req = new ubus_transfer();

    // TODO no constraints yet, so we assign the values directly
    req->addr = start_addr;
    req->read_write = WRITE;
    req->size = 4;
    req->data[0] = data0;
    req->data[1] = data1;
    req->data[2] = data2;
    req->data[3] = data3;
    req->error_pos = 1000;
    req->transmit_delay = transmit_del;

    std::ostringstream msg;
    msg << get_sequence_path()
        << " wrote : addr = 0x"
        << std::hex << req->addr.to_uint()
        << ", data[0] = 0x"
        << std::hex << req->data[0]
        << ", data[1] = 0x"
        << std::hex << req->data[1]
        << ", data[2] = 0x"
        << std::hex << req->data[2]
        << ", data[3] = 0x"
        << std::hex << req->data[3];
    UVM_INFO(get_type_name(), msg.str(), uvm::UVM_HIGH);
  }

}; // class write_word_seq


//------------------------------------------------------------------------------
//
// SEQUENCE: write_double_word_seq
//
//------------------------------------------------------------------------------

class write_double_word_seq : public ubus_base_sequence
{
public:

  // TODO randomization
  /* rand */ unsigned int start_addr;
  /* rand */ unsigned int data0;
  /* rand */ unsigned int data1;
  /* rand */ unsigned int data2;
  /* rand */ unsigned int data3;
  /* rand */ unsigned int data4;
  /* rand */ unsigned int data5;
  /* rand */ unsigned int data6;
  /* rand */ unsigned int data7;
  /* rand */ unsigned int transmit_del;

  // TODO constraints
  //constraint transmit_del_ct { (transmit_del <= 10); }

  write_double_word_seq( const std::string& name = "write_word_seq")
  : ubus_base_sequence(name), start_addr(0), data0(0),
    data1(0), data2(0), data3(0), data4(0),
    data5(0), data6(0), data7(0), transmit_del(0)
  {}

  UVM_OBJECT_UTILS(write_double_word_seq);

  virtual void body()
  {
    /*
    uvm_do_with(req,
      { req.addr == start_addr;
        req.read_write == WRITE;
        req.size == 8;
        req.data[0] == data0; req.data[1] == data1;
        req.data[2] == data2; req.data[3] == data3;
        req.data[4] == data4; req.data[5] == data5;
        req.data[6] == data6; req.data[7] == data7;
        req.error_pos == 1000;
        req.transmit_delay == transmit_del; } )
    */

    ubus_transfer* req = new ubus_transfer();

    // TODO no constraints yet, so we assign the values directly
    req->addr = start_addr;
    req->read_write = WRITE;
    req->size = 8;
    req->data[0] = data0;
    req->data[1] = data1;
    req->data[2] = data2;
    req->data[3] = data3;
    req->data[4] = data4;
    req->data[5] = data5;
    req->data[6] = data6;
    req->data[7] = data7;
    req->error_pos = 1000;
    req->transmit_delay = transmit_del;

    std::ostringstream msg;
    msg << get_sequence_path()
        << " wrote : addr = 0x"
        << std::hex << req->addr.to_uint()
        << ", data[0] = 0x"
        << std::hex << req->data[0]
        << ", data[1] = 0x"
        << std::hex << req->data[1]
        << ", data[2] = 0x"
        << std::hex << req->data[2]
        << ", data[3] = 0x"
        << std::hex << req->data[3]
        << ", data[4] = 0x"
        << std::hex << req->data[4]
        << ", data[5] = 0x"
        << std::hex << req->data[5]
        << ", data[6] = 0x"
        << std::hex << req->data[6]
        << ", data[7] = 0x"
        << std::hex << req->data[7];
    UVM_INFO(get_type_name(), msg.str(), uvm::UVM_HIGH);
  }

}; // class write_double_word_seq


#endif /* UBUS_MASTER_SEQ_LIB_H_ */

