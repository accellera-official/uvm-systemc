//-----------------------------------------------------------------------------
//   Copyright 2013-2016 NXP B.V.
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2010 Cadence Design Systems, Inc.
//   Copyright 2011 Mentor Graphics Corporation
//   Copyright 2018 Intel Corp.
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

#ifndef UVM_REG_INDIRECT_DATA_H_
#define UVM_REG_INDIRECT_DATA_H_

#include <systemc>

#include "uvmsc/reg/uvm_reg.h"
#include "uvmsc/reg/uvm_reg_model.h"

namespace uvm {

// forward class declaration
class uvm_reg_indirect_ftdr_seq;
class uvm_reg_map;
class uvm_reg_field;

//----------------------------------------------------------------------
// CLASS: uvm_reg_indirect_data
//
//! Indirect data access abstraction class
//!
//! Models the behavior of a register used to indirectly access
//! a register array, indexed by a second \p address register.
//!
//! This class should not be instantiated directly.
//! A type-specific class extension should be used to
//! provide a factory-enabled constructor and specify the
//! \p n_bits and coverage models.
//----------------------------------------------------------------------

class uvm_reg_indirect_data : public uvm_reg
{
public:
  template <typename BUSTYPE> friend class uvm_reg_predictor;

  //--------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------

  uvm_reg_indirect_data( const std::string& name,
                         unsigned int n_bits,
                         int has_cover );

  void configure( uvm_reg* idx,
                  std::vector<uvm_reg*> reg_a,
                  uvm_reg_block* blk_parent,
                  uvm_reg_file* regfile_parent = nullptr );


   /////////////////////////////////////////////////////
   // Implementation-defined member functions below,
   // not part of UVM Class reference / LRM
   /////////////////////////////////////////////////////

 private:

  virtual void build();

  void add_map( uvm_reg_map* map ) override;

  void add_frontdoors( uvm_reg_map* map );

  void do_predict( uvm_reg_item*     rw,
                           uvm_predict_e     kind = UVM_PREDICT_DIRECT,
                           uvm_reg_byte_en_t be = -1 ) override;

#if defined(ALLOW_NON_IEEE_1800_2_2020)
 protected:
#endif
  uvm_reg_map* get_local_map( const uvm_reg_map* map, const std::string& caller = "" ) const override;
#if defined(ALLOW_NON_IEEE_1800_2_2020)
 private:
#endif

  void add_field( uvm_reg_field* field ) override;

  void set( uvm_reg_data_t value,
                    const std::string& fname = "",
                    int lineno = 0 ) override;

   uvm_reg_data_t get( const std::string& fname = "",
                               int    lineno = 0 ) const override;
   
   virtual uvm_reg* get_indirect_reg( const std::string& fname = "",
                                      int    lineno = 0 ) const;

   bool needs_update() const override;

#if defined(ALLOW_NON_IEEE_1800_2_2020)
 protected:
#endif
   void write( uvm_status_e&      status,
                       uvm_reg_data_t     value,
                       uvm_path_e         path = UVM_DEFAULT_PATH,
                       uvm_reg_map*       map = nullptr,
                       uvm_sequence_base* parent = nullptr,
                       int                prior = -1,
                       uvm_object*        extension = nullptr,
                       const std::string& fname = "",
                       int                lineno = 0) override;

   void read( uvm_status_e&      status,
                      uvm_reg_data_t&    value,
                      uvm_path_e         path = UVM_DEFAULT_PATH,
                      uvm_reg_map*       map = nullptr,
                      uvm_sequence_base* parent = nullptr,
                      int                prior = -1,
                      uvm_object*        extension = nullptr,
                      const std::string& fname = "",
                      int                lineno = 0 ) override;
#if defined(ALLOW_NON_IEEE_1800_2_2020)
 private:
#endif

   void poke( uvm_status_e&      status,
                      uvm_reg_data_t     value,
                      const std::string& kind = "",
                      uvm_sequence_base* parent = nullptr,
                      uvm_object*        extension = nullptr,
                      const std::string& fname = "",
                      int                lineno = 0 ) override;

   void peek( uvm_status_e&      status,
                      uvm_reg_data_t&    value,
                      const std::string& kind = "",
                      uvm_sequence_base* parent = nullptr,
                      uvm_object*        extension = nullptr,
                      const std::string& fname = "",
                      int                lineno = 0 ) override;

   void update( uvm_status_e&      status,
                        uvm_path_e         path = UVM_DEFAULT_PATH,
                        uvm_reg_map*       map = nullptr,
                        uvm_sequence_base* parent = nullptr,
                        int                prior = -1,
                        uvm_object*        extension = nullptr,
                        const std::string& fname = "",
                        int                lineno = 0) override;
   
   void mirror( uvm_status_e&      status,
                        uvm_check_e        check  = UVM_NO_CHECK,
                        uvm_path_e         path = UVM_DEFAULT_PATH,
                        uvm_reg_map*       map = nullptr,
                        uvm_sequence_base* parent = nullptr,
                        int                prior = -1,
                        uvm_object*        extension = nullptr,
                        const std::string& fname = "",
                        int                lineno = 0) override;

protected:
   uvm_reg* m_idx;
   std::vector<uvm_reg*> m_tbl;


}; // class uvm_reg_indirect_data

} // namespace uvm

#endif // uvm_reg_indirect_data
