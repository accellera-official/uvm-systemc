//----------------------------------------------------------------------
//    Copyright 2014 NXP B.V.
//    Copyright 2004-2008 Synopsys, Inc.
//    Copyright 2010 Mentor Graphics Corporation
//    All Rights Reserved Worldwide
// 
//    Licensed under the Apache License, Version 2.0 (the
//    "License"); you may not use this file except in
//    compliance with the License.  You may obtain a copy of
//    the License at
// 
//        http://www.apache.org/licenses/LICENSE-2.0
// 
//    Unless required by applicable law or agreed to in
//    writing, software distributed under the License is
//    distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//    CONDITIONS OF ANY KIND, either express or implied.  See
//    the License for the specific language governing
//    permissions and limitations under the License.
//----------------------------------------------------------------------

#ifndef UVM_REG_BIT_BASH_SEQ_H_
#define UVM_REG_BIT_BASH_SEQ_H_

#include <systemc>
#include <string>
#include <iostream>
#include <vector>

#include "uvmsc/reg/uvm_reg_sequence.h"

namespace uvm
{



//----------------------------------------------------------------------
// Title: Bit Bashing Test Sequences
//----------------------------------------------------------------------
// This section defines classes that test individual bits of the registers
// defined in a register model.
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class: uvm_reg_single_bit_bash_seq
//
//! Verify the implementation of a single register
//! by attempting to write 1's and 0's to every bit in it,
//! via every address map in which the register is mapped,
//! making sure that the resulting value matches the mirrored value.
//!
//! If bit-type resource named
//! "NO_REG_TESTS" or "NO_REG_BIT_BASH_TEST"
//! in the "REG::" namespace
//! matches the full name of the register,
//! the register is not tested.
//!
//!| uvm_resource_db<bool>::set("REG::" + regmodel->blk->r0->get_full_name(),
//!|                            "NO_REG_TESTS", true, this);
//!
//! Registers that contain fields with unknown access policies
//! cannot be tested.
//!
//! The DUT should be idle and not modify any register during this test.
//----------------------------------------------------------------------

class uvm_reg_single_bit_bash_seq
: public uvm_reg_sequence<uvm_sequence<uvm_reg_item> > //
{
 public:

  // Data member: rg
  // The register to be tested
  uvm_reg* rg;

  UVM_OBJECT_UTILS(uvm_reg_single_bit_bash_seq);

  uvm_reg_single_bit_bash_seq( const std::string& name = "uvm_reg_single_bit_bash_seq")
  : uvm_reg_sequence<uvm_sequence<uvm_reg_item> >(name)
  {}

  virtual void body()
  {
    std::vector<uvm_reg_field*> fields;
    std::string mode[UVM_REG_DATA_WIDTH];
    std::vector<uvm_reg_map*> maps;
    uvm_reg_data_t  dc_mask;
    uvm_reg_data_t  reset_val;
    int n_bits;

    if (rg == NULL)
    {
      UVM_ERROR("uvm_reg_bit_bash_seq", "No register specified to run sequence on");
      return;
    }

    // Registers with some attributes are not to be tested
    if ( uvm_resource_db<bool>::get_by_name("REG::" + rg->get_full_name(),
           "NO_REG_TESTS", false) != NULL ||
        uvm_resource_db<bool>::get_by_name("REG::" + rg->get_full_name(),
           "NO_REG_BIT_BASH_TEST", false) != NULL )
      return;

    n_bits = rg->get_n_bytes() * 8;

    // Let's see what kind of bits we have...
    rg->get_fields(fields);

    // Registers may be accessible from multiple physical interfaces (maps)
    rg->get_maps(maps);

    // Bash the bits in the register via each map
    for( unsigned int j = 0; j < maps.size(); j++)
    {
      //uvm_status_e status; //TODO not used?
      //uvm_reg_data_t  val, exp, v;
      int next_lsb;

      next_lsb = 0;
      dc_mask  = 0;

      for( unsigned int k = 0; k < fields.size(); k++)
      {
        int lsb, w, dc;

        dc = (fields[k]->get_compare() == UVM_NO_CHECK) ? 1 : 0;
        lsb = fields[k]->get_lsb_pos();
        w   = fields[k]->get_n_bits();

        // Ignore Write-only fields because
        // you are not supposed to read them

        std::string access = fields[k]->get_access(maps[j]);

        if ( access == "WO"  || access == "WOC" ||
            access ==  "WOS" || access == "WO1" )
          dc = 1;

        // Any unused bits on the right side of the LSB?
        while (next_lsb < lsb)
          mode[next_lsb++] = "RO";

        for ( int r = 0; r < w; r++) // repeat(w)
        {
          mode[next_lsb] = fields[k]->get_access(maps[j]);
          dc_mask[next_lsb] = dc;
          next_lsb++;
        }

      } // for-k

      // Any unused bits on the left side of the MSB?
      while (next_lsb < UVM_REG_DATA_WIDTH)
        mode[next_lsb++] = "RO";

      UVM_INFO("uvm_reg_bit_bash_seq", "Verifying bits in register " +
               rg->get_full_name() + "  in map '" +
               maps[j]->get_full_name() + "'...", UVM_LOW);

      // Bash the kth bit
      for (int k = 0; k < n_bits; k++)
      {
        // Cannot test unpredictable bit behavior
        if (dc_mask[k]) continue;

        bash_kth_bit(rg, k, mode[k], maps[j], dc_mask);
      }

    }
  } // body


  void bash_kth_bit( uvm_reg*             rg,
                     int                  k,
                     const std::string&   mode,
                     uvm_reg_map*         map,
                     uvm_reg_data_t       dc_mask )
  {
    uvm_status_e   status;
    uvm_reg_data_t val, exp, v;
    bool bit_val;

    std::ostringstream str;
    str << "...Bashing "
        << mode
        << " bit #"
        << k;

    UVM_INFO("uvm_reg_bit_bash_seq", str.str(), UVM_HIGH);

    for (int i = 0; i < 2; i++) // repeat(2)
    {
      val = rg->get();
      v   = val;
      exp = val;
      val[k] = ~val[k];
      bit_val = val[k];

      rg->write(status, val, UVM_FRONTDOOR, map, this);

      if (status != UVM_IS_OK)
      {
        UVM_ERROR("uvm_reg_bit_bash_seq",
                  "Status was " + std::string(uvm_status_name[status]) +
                  " when writing to register '" + rg->get_full_name() +
                  "' through map '" + map->get_full_name() + "'.");
      }

      exp = rg->get() & ~dc_mask;
      rg->read(status, val, UVM_FRONTDOOR, map, this);

      if (status != UVM_IS_OK)
      {
        UVM_ERROR("uvm_reg_bit_bash_seq",
                  "Status was " + std::string(uvm_status_name[status]) +
                  " when reading register '" + rg->get_full_name() +
                  " through map '" + map->get_full_name() + "'.");
      }

      val &= ~dc_mask;
      if (val != exp) // TODO check, was !==
      {
        std::ostringstream str;
        str << "Writing a "
            << ((bit_val) ? "1" : "0")
            << " in bit #"
            << std::dec << k
            << " of register '"
            << rg->get_full_name()
            << "' with initial value 0x"
            << std::hex << v.to_uint64()
            << " yielded 0x"
            << std::hex << val.to_uint64()
            << " instead of 0x"
            << std::hex << exp.to_uint64()
            << ".";
        UVM_ERROR("uvm_reg_bit_bash_seq", str.str() );
      }
    }
  }

}; // class uvm_reg_single_bit_bash_seq


//------------------------------------------------------------------------------
// Class: uvm_reg_bit_bash_seq
//
//! Verify the implementation of all registers in a block
//! by executing the <uvm_reg_single_bit_bash_seq> sequence on it.
//!
//! If bit-type resource named
//! "NO_REG_TESTS" or "NO_REG_BIT_BASH_TEST"
//! in the "REG::" namespace
//! matches the full name of the block,
//! the block is not tested.
//------------------------------------------------------------------------------

class uvm_reg_bit_bash_seq
: public uvm_reg_sequence<uvm_sequence<uvm_reg_item> >
{
 public:
  UVM_OBJECT_UTILS(uvm_reg_bit_bash_seq);

  uvm_reg_bit_bash_seq(const std::string& name="uvm_reg_bit_bash_seq")
  : uvm_reg_sequence<uvm_sequence<uvm_reg_item> >(name)
  {}

  // Member function: body
  //
  // Executes the Register Bit Bash sequence.
  // Do not call directly. Use seq.start() instead.
  //
  virtual void body()
  {
    if (model == NULL)
    {
      UVM_ERROR("uvm_reg_bit_bash_seq", "No register model specified to run sequence on");
      return;
    }

    uvm_report_info("STARTING_SEQ","\n\nStarting " + get_name() + " sequence...\n", UVM_LOW);

    reg_seq = uvm_reg_single_bit_bash_seq::type_id::create("reg_single_bit_bash_seq");

    this->reset_blk(model);
    model->reset();

    do_block(model);
  }

 protected:
  // Member function: do_block
  //
  // Test all of the registers in a a given ~block~
  //
  virtual void do_block( uvm_reg_block* blk )
  {
    std::vector<uvm_reg*> regs;

    if (uvm_resource_db<bool>::get_by_name("REG::" + blk->get_full_name(),
            "NO_REG_TESTS", false) != NULL ||
        uvm_resource_db<bool>::get_by_name("REG::" + blk->get_full_name(),
            "NO_REG_BIT_BASH_TEST", false) != NULL )
      return;

    // Iterate over all registers, checking accesses
    blk->get_registers(regs, UVM_NO_HIER);

    for( unsigned int i = 0; i < regs.size(); i++ )
    {
      // Registers with some attributes are not to be tested
      if (uvm_resource_db<bool>::get_by_name("REG::" + regs[i]->get_full_name(),
          "NO_REG_TESTS", false) != NULL ||
          uvm_resource_db<bool>::get_by_name("REG::" + regs[i]->get_full_name(),
              "NO_REG_BIT_BASH_TEST", false) != NULL )
        continue;

      reg_seq->rg = regs[i];
      reg_seq->start(NULL, this);
    }

    std::vector<uvm_reg_block*> blks;
    blk->get_blocks(blks);

    for( unsigned int i = 0; i < blks.size(); i++)
      do_block(blks[i]);
  }

 public:

  // Member function: reset_blk
  //
  // Reset the DUT that corresponds to the specified block abstraction class.
  //
  // Currently empty.
  // Will rollback the environment's phase to the ~reset~
  // phase once the new phasing is available.
  //
  // In the meantime, the DUT should be reset before executing this
  // test sequence or this method should be implemented
  // in an extension to reset the DUT.
  //
  virtual void reset_blk( uvm_reg_block* blk ) {}

 public:

  // Variable: model
  //
  // The block to be tested. Declared in the base class.
  //
  //| uvm_reg_block model;


  // Variable: reg_seq
  //
  // The sequence used to test one register
  //
 protected:
  uvm_reg_single_bit_bash_seq* reg_seq;

}; // class uvm_reg_bit_bash_seq

} // namespace uvm

#endif // UVM_REG_BIT_BASH_SEQ_H_

