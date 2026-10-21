//----------------------------------------------------------------------
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
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

#ifndef UVM_TABLE_PRINTER_H_
#define UVM_TABLE_PRINTER_H_

#include <memory>
#include <systemc>

#include "uvmsc/print/uvm_printer.h"


//////////////

namespace uvm {


//------------------------------------------------------------------------------
// Class: uvm_table_printer
//
//! The table printer prints output in a tabular format.
//------------------------------------------------------------------------------

class uvm_table_printer : public uvm_printer
{
 public:

  uvm_table_printer();

  std::string emit() override;

  static std::shared_ptr<uvm_table_printer> get_default(); 
  
  static void set_default(std::shared_ptr<uvm_table_printer>);
  
  // Variables: m_max_*
  //
  // holds max size of each column, so table columns can be resized dynamically

 protected:
  unsigned int m_max_name;
  unsigned int m_max_type;
  unsigned int m_max_size;
  unsigned int m_max_value;

  void calculate_max_widths();

};

//////////////

} // namespace uvm

#endif /* UVM_TABLE_PRINTER_H_ */
