//-----------------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2010 Cadence Design Systems, Inc.
//   Copyright 2011 Mentor Graphics Corporation
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

#ifndef UVM_REG_INDIRECT_FTDR_SEQ_H_
#define UVM_REG_INDIRECT_FTDR_SEQ_H_

#include "uvmsc/reg/uvm_reg_frontdoor.h"

namespace uvm {

// forward class declarations
class uvm_reg;


class uvm_reg_indirect_ftdr_seq : public uvm_reg_frontdoor
{
 public:

  uvm_reg_indirect_ftdr_seq( uvm_reg* addr_reg,
                             int idx,
                             uvm_reg* data_reg);

  void body() override;

 private:
  // local data members

  uvm_reg* m_addr_reg;
  uvm_reg* m_data_reg;
  int      m_idx;

}; // class

} // namespace uvm

#endif // UVM_REG_INDIRECT_FTDR_SEQ_H_
