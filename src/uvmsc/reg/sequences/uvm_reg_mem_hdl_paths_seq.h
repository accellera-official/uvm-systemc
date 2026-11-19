// -------------------------------------------------------------
//    Copyright 2015 NXP B.V.
//    Copyright 2010 Cadence.
//    Copyright 2011 Mentor Graphics Corporation
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
// -------------------------------------------------------------

#ifndef UVM_REG_MEM_HDL_PATHS_SEQ_H_
#define UVM_REG_MEM_HDL_PATHS_SEQ_H_

#include <systemc>
#include <string>
#include <iostream>
#include <vector>

#include "uvmsc/reg/uvm_reg_block.h"
#include "uvmsc/reg/uvm_reg_sequence.h"
#include "uvmsc/dpi/uvm_hdl.h"

// -------------------------------------------------------------
// HDL Paths Checking Test Sequence
//
// class: uvm_reg_mem_hdl_paths_seq
//
// Verify the correctness of HDL paths specified for registers and memories.
//
// This sequence is be used to check that the specified backdoor paths
// are indeed accessible by the simulator.
// By default, the check is performed for the default design abstraction.
// If the simulation contains multiple s of the DUT,
// HDL paths for multiple design abstractions can be checked.
// 
// If a path is not accessible by the simulator, it cannot be used for 
// read/write backdoor accesses. In that case a warning is produced. 
// A simulator may have finer-grained access permissions such as separate 
// read or write permissions.
// These extra access permissions are NOT checked.
//
// The test is performed in zero time and
// does not require any reads/writes to/from the DUT.
// -------------------------------------------------------------

namespace uvm
{

class uvm_reg_mem_hdl_paths_seq
: public uvm_reg_sequence<uvm_sequence<uvm_reg_item> >
{
 public:

    // Variable: abstractions
    // If set, check the HDL paths for the specified design abstractions.
    // If empty, check the HDL path for the default design abstraction,
    // as specified with <uvm_reg_block::set_default_hdl_path()>
    std::vector<std::string> abstractions;

  UVM_OBJECT_UTILS(uvm_reg_mem_hdl_paths_seq);

  uvm_reg_mem_hdl_paths_seq( std::string name="uvm_reg_mem_hdl_paths_seq")
  {}

  void body() override
  {

    if (model == nullptr)
    {
      uvm_report_error("uvm_reg_mem_hdl_paths_seq", "Register model handle is nullptr");
      return;
    }

    UVM_INFO("uvm_reg_mem_hdl_paths_seq",
        "checking HDL paths for all registers/memories in "+
        model->get_full_name(), UVM_LOW);

    if (abstractions.size() == 0)
      do_block(model, "");
    else
    {
      for(unsigned int i = 0; i < abstractions.size(); i++)
        do_block(model, abstractions[i]);
    }

    UVM_INFO("uvm_reg_mem_hdl_paths_seq", "HDL path validation completed ", UVM_LOW);

  }


  // Any additional steps required to reset the block
  // and make it accessible
  virtual void reset_blk(uvm_reg_block blk) {}

 protected:
  virtual void do_block(uvm_reg_block* blk, std::string kind)
  {
    std::vector<uvm_reg*> regs;
    std::vector<uvm_mem*> mems;

    UVM_INFO("uvm_reg_mem_hdl_paths_seq",
        "Validating HDL paths in " + blk->get_full_name() +
        " for " + ((kind == "") ? "default" : kind) +
        " design abstraction", UVM_MEDIUM);

    // Iterate over all registers, checking accesses
    blk->get_registers(regs, UVM_NO_HIER);
    for ( unsigned int i = 0; i < regs.size(); i++)
      check_reg(regs[i], kind);

    blk->get_memories(mems, UVM_NO_HIER);

    for ( unsigned int i = 0; i < mems.size(); i++)
      check_mem(mems[i], kind);

    {
      std::vector<uvm_reg_block*> blks;

      blk->get_blocks(blks);

      for ( unsigned int i = 0; i < blks.size(); i++)
        do_block(blks[i], kind);
    }
  }


  virtual void check_reg(uvm_reg* r, std::string kind)
  {
    std::vector<uvm_hdl_path_concat> paths;

    // avoid calling get_full_hdl_path when the register has not path for this abstraction kind
    if(!r->has_hdl_path(kind))
      return;

    r->get_full_hdl_path(paths, kind);
    if (paths.size() == 0) return;

    for ( unsigned int p = 0; p < paths.size(); p++)
    {
      uvm_hdl_path_concat path=paths[p];
      for ( unsigned int j = 0; j < path.slices.size(); j++)
      {
        std::string p_ = path.slices[j].path;
        // uvm_reg_data_t d; // TODO type mismatch?
        uvm_hdl_data_t d;

        if (!uvm_hdl_read(p_, d))
        {
          std::ostringstream str;
          str << "HDL path '"
              << p_
              << "' for register '"
              << r->get_full_name()
              << "' is not readable.";
          UVM_ERROR("uvm_reg_mem_hdl_paths_seq", str.str());
        }
        if (!uvm_hdl_check_path(p_))
        {
          std::ostringstream str;
          str << "HDL path '"
              << p_
              << "' for register '"
              << r->get_full_name()
              << "' is not accessible.";
          UVM_ERROR("uvm_reg_mem_hdl_paths_seq", str.str());
        }
      }
    }
  }

  virtual void check_mem(uvm_mem* m, std::string kind)
  {
    std::vector<uvm_hdl_path_concat> paths;

    // avoid calling get_full_hdl_path when the register has not path for this abstraction kind
    if(!m->has_hdl_path(kind))
      return;

    m->get_full_hdl_path(paths, kind);
    if (paths.size() == 0)
      return;

    for( unsigned int p = 0; p < paths.size(); p++)
    {
      uvm_hdl_path_concat path = paths[p];

      for( unsigned int j = 0; j < path.slices.size(); j++)
      {
        std::string p_ = path.slices[j].path;
        if(!uvm_hdl_check_path(p_))
        {
          std::ostringstream str;
          str << "HDL path '"
              << p_
              << "' for memory '"
              << m->get_full_name()
              << "' is not accessible.";
          UVM_ERROR("uvm_reg_mem_hdl_paths_seq", str.str());
        }
      }
    }
  }
}; // class uvm_reg_mem_hdl_paths_seq

} // namespace

#endif // UVM_REG_MEM_HDL_PATHS_SEQ_H_

