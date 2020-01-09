//------------------------------------------------------------------------------
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2019 COSEDA Technologies GmbH
//   Copyright 2020 Bosch Sensortec GmbH
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
//------------------------------------------------------------------------------

#ifndef SEQ_ARB_ITEM_H_
#define SEQ_ARB_ITEM_H_

#include <uvm>
#include <systemc>

// This item carries the number of the sequence from
// which it was sent

class seq_arb_item : public uvm::uvm_sequence_item
{
 public:

  int seq_no;

  UVM_OBJECT_UTILS(seq_arb_item);

  seq_arb_item( const std::string name = "seq_arb_item" )
  : uvm::uvm_sequence_item(name), seq_no(0)
  {}

}; // class seq_arb_item

#endif
