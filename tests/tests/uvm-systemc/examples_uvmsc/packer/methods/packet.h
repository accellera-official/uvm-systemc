//----------------------------------------------------------------------
//   Copyright 2009 Cadence Design Systems, Inc.
//   Copyright 2012-2014 NXP B.V.
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

#ifndef PACKET_H
#define PACKET_H

#include <systemc>
#include <uvm>

/////////////////

class myobj : public uvm::uvm_object
{
 public:
  int a;
  int b;

  UVM_OBJECT_UTILS(myobj);

  myobj( const std::string& name = "myobj", int a_ = 1, int b_ = 1)
  : uvm::uvm_object(name), a(a_), b(b_)
  {}

  void do_print(const uvm::uvm_printer& printer) const override
  {
    printer.print_field_int("a", a);
    printer.print_field_int("b", b);
  }

  void do_pack(uvm::uvm_packer& p) const override
  {
    p.pack_field_int(a, 64);
    p.pack_field_int(b, 64);
  }

  void do_unpack(uvm::uvm_packer& p) override
  {
    a = p.unpack_field_int(64);
    b = p.unpack_field_int(64);
  }

  virtual bool do_compare( const uvm::uvm_object& rhs,
                           const uvm::uvm_comparer* comparer = nullptr ) const override
  {
    const myobj* drhs = dynamic_cast<const myobj*>(&rhs);
    if (!drhs)
    {
      UVM_ERROR("COMPERR", "Cannot compare different types.");
      return false;
    }
    return (
      (a == drhs->a)
      && (b == drhs->b)
    );
  }

};

class packet : public uvm::uvm_object
{
 public:
  int data;
  std::string s;
  int addr;
  float r;
  sc_core::sc_time t;
  myobj o;

  UVM_OBJECT_UTILS(packet);

  packet( const std::string& name = "packet",
          int _data = 0x0, std::string _s = "", int _addr = 0x0, float _r = 0.0, sc_core::sc_time _t = sc_core::SC_ZERO_TIME, int _o1 = 0x0, int _o2 = 0x0 )
  : uvm::uvm_object(name),
    data(_data),
    s(_s),
    addr(_addr),
    r(_r),
    t(_t),
    o("o", _o1, _o2)
  {}

  ~packet() override {}

  void do_print(const uvm::uvm_printer& printer) const override
  {
    printer.print_field_int("data", data);
    printer.print_string("s", s);
    printer.print_field_int("addr", addr);
    printer.print_real("real", r);
    printer.print_time("time", t);
    printer.print_object("o", o);
  }

  void do_pack(uvm::uvm_packer& p) const override
  {
    p.pack_field_int(data, 64);
    p.pack_string(s);
    p.pack_field_int(addr, 64);
    p.pack_real(r);
    p.pack_time(t);
    p.pack_object(o);
  }

  void do_unpack(uvm::uvm_packer& p) override
  {
    data = p.unpack_field_int(64);
    s = p.unpack_string(3);
    addr = p.unpack_field_int(64);
    r = p.unpack_real();
    t = p.unpack_time();
    p.unpack_object(o);
  }

  void do_copy( const uvm::uvm_object& rhs ) override
  {
    const packet* drhs = dynamic_cast<const packet*>(&rhs);
    if (!drhs)
    {
      UVM_ERROR("COPYERR", "Cannot copy different types.");
      return;
    }
    data = drhs->data;
  }

  virtual bool do_compare( const uvm::uvm_object& rhs,
                           const uvm::uvm_comparer* comparer = nullptr ) const override
  {
    const packet* drhs = dynamic_cast<const packet*>(&rhs);
    if (!drhs)
    {
      UVM_ERROR("COMPERR", "Cannot compare different types.");
      return false;
    }
    return (
      (data == drhs->data)
      && (s == drhs->s)
      && (addr == drhs->addr)
      && (r == drhs->r)
      && (t == drhs->t)
      && (o.do_compare(drhs->o))
    );
  }
};

/////////////////

#endif /* PACKET_H_ */
