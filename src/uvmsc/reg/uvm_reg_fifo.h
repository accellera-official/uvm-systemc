//----------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2010-2011 Mentor Graphics Corporation
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

#ifndef UVM_REG_FIFO_H_
#define UVM_REG_FIFO_H_

#include "uvmsc/reg/uvm_reg.h"

//////////////

namespace uvm {


//----------------------------------------------------------------------
// Class: uvm_reg_fifo
//
//! This special register models a DUT FIFO accessed via write/read,
//! where writes push to the FIFO and reads pop from it.
//!
//! Backdoor access is not enabled, as it is not yet possible to force
//! complete FIFO state, i.e. the write and read indexes used to access
//! the FIFO data.
//----------------------------------------------------------------------

class uvm_reg_fifo : public uvm_reg
{
 public:

  //--------------------------------------------------------------------
  // Group: Initialization
  //--------------------------------------------------------------------

  uvm_reg_fifo( const std::string& name,
                unsigned int size,
                unsigned int n_bits,
                int has_cover);

  void set_compare( uvm_check_e check = UVM_CHECK );

  //--------------------------------------------------------------------
  // Group: Introspection
  //--------------------------------------------------------------------

  unsigned int size();

  unsigned int capacity();

  //--------------------------------------------------------------------
  // Group: Access
  //--------------------------------------------------------------------

  //  virtual void write(...) - inherited from base class

  //  virtual void read(...) - inherited from base class

  void set( uvm_reg_data_t  value,
                    const std::string& fname = "",
                    int lineno = 0 ) override;

  void update( uvm_status_e&      status,
                       uvm_path_e         path = UVM_DEFAULT_PATH,
                       uvm_reg_map*       map = nullptr,
                       uvm_sequence_base* parent = nullptr,
                       int                prior = -1,
                       uvm_object*        extension = nullptr,
                       const std::string&             fname = "",
                       int                lineno = 0 ) override;

  //  virtual void mirror(...) - inherited from base class

  uvm_reg_data_t get( const std::string& fname = "",
                              int lineno = 0 ) const override;

  void do_predict( uvm_reg_item*     rw,
                           uvm_predict_e     kind = UVM_PREDICT_DIRECT,
                           uvm_reg_byte_en_t be = -1 ) override; // TODO is -1 allowed ?

  //--------------------------------------------------------------------
  // Group: Special Overrides
  //--------------------------------------------------------------------

  void pre_write( uvm_reg_item* rw ) override;

  void pre_read( uvm_reg_item* rw ) override;


  // data members

  /*rand*/ std::vector<uvm_reg_data_t> fifo; // TODO randomize


  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

  virtual void build();

  void post_randomize();

  /* TODO constraints
  constraint valid_fifo_size
  {
    fifo.size() <= m_size;
  }
  */

  // local data members
private:
  uvm_reg_field* m_value;
  int m_set_cnt;
  unsigned int m_size;

}; // class uvm_reg_fifo

} // namespace uvm

#endif // UVM_REG_FIFO_H_
