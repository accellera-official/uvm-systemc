//----------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2010-2011 Synopsys, Inc.
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

#ifndef REGMODEL_H_
#define REGMODEL_H_

#include <systemc>
#include <uvm>

//----------------------------------------------------------------------
// This example demonstrates how to model aliased registers
// i.e. registers that are present at two physical addresses,
// possibily with different access policies.
//
// In this case, we have a register "R" which is known under two names
// "Ra" and "Rb". When accessed as "Ra", field F2 is RO.
//----------------------------------------------------------------------

class reg_Ra : public uvm::uvm_reg
{
 public:
  /*rand*/ uvm::uvm_reg_field* F1;  // TODO randomization
  /*rand*/ uvm::uvm_reg_field* F2;

  reg_Ra(std::string name = "Ra") : uvm::uvm_reg(name, 32, uvm::UVM_NO_COVERAGE)
  {}

  virtual void build()
  {
    F1 = uvm::uvm_reg_field::type_id::create("F1");
    F1->configure(this, 8, 0, "RW", false, 0x0, true, false, true);
    F2 = uvm::uvm_reg_field::type_id::create("F2");
    F2->configure(this, 8, 16, "RO", false, 0x0, true, false, true);
  }

  UVM_OBJECT_UTILS(reg_Ra);

}; // class reg_Ra


class reg_Rb : public uvm::uvm_reg
{
 public:
  /*rand*/ uvm::uvm_reg_field* F1;
  /*rand*/ uvm::uvm_reg_field* F2;

  reg_Rb(std::string name = "Rb") : uvm::uvm_reg(name, 32, uvm::UVM_NO_COVERAGE)
  {}

  virtual void build()
  {
    F1 = uvm::uvm_reg_field::type_id::create("F1");
    F1->configure(this, 8, 0, "RW", false, 0x0, true, false, true);
    F2 = uvm::uvm_reg_field::type_id::create("F2");
    F2->configure(this, 8, 16, "RW", false, 0x0, true, false, true);
  }

  UVM_OBJECT_UTILS(reg_Rb);

}; // class reg_Rb


class write_also_to_F : public uvm::uvm_reg_cbs
{
 public:

  write_also_to_F( uvm::uvm_reg_field* toF )
  {
    m_toF = toF;
  }

  virtual void post_predict( uvm::uvm_reg_field*  fld,
                             uvm::uvm_reg_data_t  previous,
                             uvm::uvm_reg_data_t  value,
                             uvm::uvm_predict_e   kind,
                             uvm::uvm_path_e      path,
                             uvm::uvm_reg_map*    map )
  {
    if (kind != uvm::UVM_PREDICT_WRITE)
      return;

    m_toF->predict(value, -1, uvm::UVM_PREDICT_DIRECT, path, map);
  }

 private:
  uvm::uvm_reg_field* m_toF;

};


class alias_RaRb : public uvm::uvm_object
{
 public:

  UVM_OBJECT_UTILS(alias_RaRb);
   
   alias_RaRb(std::string name = "alias_RaRb") : uvm::uvm_object(name)
   {}

   void configure(reg_Ra* Ra, reg_Rb* Rb)
   {
      m_Ra = Ra;
      m_Rb = Rb;
      
      F2F1 = new write_also_to_F(Ra->F1);
      uvm::uvm_reg_field_cb::add(Rb->F1, F2F1);
      F2F2 = new write_also_to_F(Ra->F2);
      uvm::uvm_reg_field_cb::add(Rb->F2, F2F2);
      F2F3 = new write_also_to_F(Rb->F1);
      uvm::uvm_reg_field_cb::add(Ra->F1, F2F3);
   }

   virtual ~alias_RaRb()
   {
      delete F2F1;
      delete F2F2;
      delete F2F3;
   }

 protected:
   reg_Ra* m_Ra;
   reg_Rb* m_Rb;
   write_also_to_F* F2F1;
   write_also_to_F* F2F2;
   write_also_to_F* F2F3;

}; // class alias_RaRb


class block_B : public uvm::uvm_reg_block
{
 public:
  /*rand*/ reg_Ra* Ra; // TODO randomization
  /*rand*/ reg_Rb* Rb;

  block_B(std::string name = "B") : uvm::uvm_reg_block( name, uvm::UVM_NO_COVERAGE )
  {}

  virtual void build()
  {
    uvm::uvm_reg_addr_t base_addr = 0x0000;
    unsigned int n_bytes = 4;

    default_map = create_map("default_map", base_addr, n_bytes, uvm::UVM_BIG_ENDIAN);

    Ra = reg_Ra::type_id::create("Ra");
    Ra->configure(this, NULL);
    Ra->build();

    Rb = reg_Rb::type_id::create("Rb");
    Rb->configure(this, NULL);
    Rb->build();

    default_map->add_reg(Ra, 0x0,   "RW");
    default_map->add_reg(Rb, 0x100, "RW");

    alias_RaRb* RaRb;

    RaRb = alias_RaRb::type_id::create("RaRb", NULL, get_full_name());
    RaRb->configure(Ra, Rb);
  }

  UVM_OBJECT_UTILS(block_B);

}; // class block_B

#endif // REGMODEL_H_
