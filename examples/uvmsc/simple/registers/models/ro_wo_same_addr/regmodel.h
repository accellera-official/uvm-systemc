//----------------------------------------------------------------------
//   Copyright 2010-2011 Synopsys, Inc.
//   Copyright 2013-2014 NXP B.V.
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
// This example demonstrates how to model a register containing
// read-only fields and a register containing write-only fields
// at the same physical address.
//----------------------------------------------------------------------

class reg_RO : public uvm::uvm_reg
{
public:
  uvm::uvm_reg_field* F1;
  uvm::uvm_reg_field* F2;

  // Register reg_RO:
  // msb                                                                                           lsb
  // -------------------------------------------------------------------------------------------------
  // |31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00|
  // |///////reserved////////|        F2 (RC)        |///////reserved////////|        F1 (RO)        |
  // -------------------------------------------------------------------------------------------------
  // Reset value:
  //   x  x  x  x  x  x  x  x  1  1  1  1  1  1  1  1  x  x  x  x  x  x  x  x  0  0  0  0  0  0  0  0
  //

  reg_RO( std::string name = "RO" ) : uvm::uvm_reg(name, 32, uvm::UVM_NO_COVERAGE)
  {}

  virtual void build()
  {
    F1 = uvm::uvm_reg_field::type_id::create("F1");
    F1->configure(this, 8, 0, "RO", false, 0x00, true, false, true);
    F2 = uvm::uvm_reg_field::type_id::create("F2");
    F2->configure(this, 8, 16, "RC", false, 0xFF, true, false, true);
  }

  UVM_OBJECT_UTILS(reg_RO);

}; // class reg_RO


class reg_WO : public uvm::uvm_reg
{
 public:
  /*rand*/ uvm::uvm_reg_field* F1; // TODO randomization
  /*rand*/ uvm::uvm_reg_field* F2;

  // Register reg_WO:
  // msb                                                                                           lsb
  // -------------------------------------------------------------------------------------------------
  // |31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00|
  // |///////reserved////////|             F2 (WO)               |        F1 (WO)        |/reserved//|
  // -------------------------------------------------------------------------------------------------
  // Reset value:
  //   x  x  x  x  x  x  x  x  1  1  0  0  1  1  0  0  1  1  0  0  1  0  1  0  1  0  1  0  x  x  x  x
  //

  reg_WO( std::string name = "WO") : uvm::uvm_reg(name, 32, uvm::UVM_NO_COVERAGE)
  {}

  virtual void build()
  {
    F1 = uvm::uvm_reg_field::type_id::create("F1");
    F1->configure(this, 8, 4, "WO", false, 0xAA, true, true, true);
    F2 = uvm::uvm_reg_field::type_id::create("F2");
    F2->configure(this, 12, 12, "WO", false, 0xCCC, true, true, true);
  }

  UVM_OBJECT_UTILS(reg_WO);

}; // class reg_WO


class block_B : public uvm::uvm_reg_block
{
 public:
  /*rand*/ reg_RO* R; // TODO randomization
  /*rand*/ reg_WO* W;

  // Register block block_B (part of map 'default_map').
  // Addr   msb                                                                                           lsb
  //        -------------------------------------------------------------------------------------------------
  //        |31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00|
  // 0x0100 |///////reserved////////|        F2 (RC)        |///////reserved////////|        F1 (RO)        | reg_RO (RO)
  //        -------------------------------------------------------------------------------------------------
  // 0x0100 |///////reserved////////|             F2 (WO)               |        F1 (WO)        |/reserved//| reg_WO (WO)
  //        -------------------------------------------------------------------------------------------------
  // 0x0000 (base)

  block_B( std::string name = "B" ) : uvm::uvm_reg_block(name, uvm::UVM_NO_COVERAGE)
  {}

  virtual void build()
  {
    uvm::uvm_reg_addr_t base_addr = 0x0000;
    unsigned int n_bytes = 4;

    default_map = create_map("default_map", base_addr, n_bytes, uvm::UVM_BIG_ENDIAN);

    R = reg_RO::type_id::create("RO");
    R->configure(this, NULL, "RO_reg");
    R->build();

    W = reg_WO::type_id::create("WO");
    W->configure(this, NULL, "WO_reg");
    W->build();

    // on purpose, we add the R and W register using the same offset (0x100)
    default_map->add_reg(R, 0x100, "RO");
    default_map->add_reg(W, 0x100, "WO");
  }

  UVM_OBJECT_UTILS(block_B);

}; // class block_B

#endif // REGMODEL_H_
