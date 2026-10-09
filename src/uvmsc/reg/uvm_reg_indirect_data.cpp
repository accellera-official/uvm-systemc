//-----------------------------------------------------------------------------
//   Copyright 2013-2016 NXP B.V.
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2010 Cadence Design Systems, Inc.
//   Copyright 2011 Mentor Graphics Corporation
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
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

#include "uvmsc/reg/uvm_reg_indirect_data.h"
#include "uvmsc/reg/uvm_reg_indirect_ftdr_seq.h"
#include "uvmsc/reg/uvm_reg_frontdoor.h"
#include "uvmsc/reg/uvm_reg.h"
#include "uvmsc/reg/uvm_reg_block.h"

namespace uvm {

//-----------------------------------------------------------------------------
// Constructor
//
//! Create an instance of this class
//!
//! Should not be called directly, other than via a derived class.
//! The value of \m n_bits must match the number of bits
//! in the indirect register array.
//-----------------------------------------------------------------------------

uvm_reg_indirect_data::uvm_reg_indirect_data( const std::string& name,
                                              unsigned int n_bits,
                                              int has_cover)
  : uvm_reg( ((name.empty()) ? sc_core::sc_gen_unique_name("uvm_reg_indirect") : name), n_bits, has_cover)
{
  m_idx = nullptr;
}

//-----------------------------------------------------------------------------
// member function: configure
//
//! Configure the indirect data register.
//!
//! The \p idx register specifies the index,
//! in the \p reg_a register array, of the register to access.
//! The \p idx must be written to first.
//! A read or write operation to this register will subsequently
//! read or write the indexed register in the register array.
//!
//! The number of bits in each register in the register array must be
//! equal to \p n_bits of this register.
//!
//! See uvm_reg::configure() for the remaining arguments.
//-----------------------------------------------------------------------------

void uvm_reg_indirect_data::configure( uvm_reg* idx,
                                       std::vector<uvm_reg*> reg_a,
                                       uvm_reg_block* blk_parent,
                                       uvm_reg_file* regfile_parent )
{
  uvm_reg::configure(blk_parent, regfile_parent, "");
  m_idx = idx;
  m_tbl = reg_a;

  // Not testable using pre-defined sequences

  // TODO support normal types in uvm_resource_db
  //uvm_resource_db<bool>::set("REG::" + get_full_name(),
  //    "NO_REG_TESTS", true);

  // Add a frontdoor to each indirectly-accessed register
  // for every address map this register is in.
  for( m_maps_citt it = m_maps.begin(); it != m_maps.end(); it++ )
    add_frontdoors((*it).first);
}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// member function: build (virtual)
//
// Implementation defined
// Will be overloaded in application
//-----------------------------------------------------------------------------

void uvm_reg_indirect_data::build()
{}

//-----------------------------------------------------------------------------
// member function: add_map (virtual)
//
// Implementation defined
//-----------------------------------------------------------------------------

void uvm_reg_indirect_data::add_map( uvm_reg_map* map )
{
  uvm_reg::add_map(map);
  add_frontdoors(map);
}


//-----------------------------------------------------------------------------
// member function: add_frontdoors
//
// Implementation defined
//-----------------------------------------------------------------------------

void uvm_reg_indirect_data::add_frontdoors( uvm_reg_map* map )
{
  for( unsigned int i = 0; i < m_tbl.size(); i++)
  {
    uvm_reg_indirect_ftdr_seq* fd;
    if (m_tbl[i] == nullptr)
    {
      std::ostringstream str;
      str << "Indirect register "<< i << " is nullptr";
      UVM_ERROR(get_full_name(), str.str() );
      continue;
    }
    fd = new uvm_reg_indirect_ftdr_seq(m_idx, i, this);

    uvm_reg_frontdoor* fdbase;
    fdbase = dynamic_cast<uvm_reg_frontdoor*>(fd);

    if (m_tbl[i]->is_in_map(map))

      m_tbl[i]->set_frontdoor(fdbase, map);
    else
      map->add_reg(m_tbl[i], -1, "RW", 1, fdbase);
  }
}


//-----------------------------------------------------------------------------
// member function: do_predict (virtual)
//
// Implementation defined
//-----------------------------------------------------------------------------

void uvm_reg_indirect_data::do_predict( uvm_handle<uvm_reg_item>  rw,
                                        uvm_predict_e kind,
                                        uvm_reg_byte_en_t be )
{
  if (m_idx->get() >= m_tbl.size())
  {
    std::ostringstream str;
    str << "Address register "
        << m_idx->get_full_name()
        << " has a value ("
        << m_idx->get()
        << ") greater than the maximum indirect register array size ("
        << m_tbl.size()
        << ").";
    UVM_ERROR(get_full_name(), str.str());
    rw->status = UVM_NOT_OK;
    return;
  }

  //NOTE limit to 2**32 registers
  {
    unsigned int idx = m_idx->get();
    m_tbl[idx]->do_predict(rw, kind, be);
  }
}

//-----------------------------------------------------------------------------
// member function: get_local_map (virtual)
//
// Implementation defined
//-----------------------------------------------------------------------------

uvm_reg_map* uvm_reg_indirect_data::get_local_map( const uvm_reg_map* map,
                                                   const std::string& caller ) const
{
  return m_idx->get_local_map(map,caller);
}

//-----------------------------------------------------------------------------
// member function: add_field (virtual)
//
// Implementation defined
//
// Just for good measure, to catch and short-circuit non-sensical uses
//-----------------------------------------------------------------------------

void uvm_reg_indirect_data::add_field( uvm_reg_field* field )
{
  UVM_ERROR(get_full_name(), "Cannot add field to an indirect data access register");
}

//-----------------------------------------------------------------------------
// member function: set (virtual)
//
// Implementation defined
//
// Just for good measure, to catch and short-circuit non-sensical uses
//-----------------------------------------------------------------------------

void uvm_reg_indirect_data::set( uvm_reg_data_t value,
                                 const std::string& fname,
                                 int lineno )
{
  UVM_ERROR(get_full_name(), "Cannot set() an indirect data access register");
}

//-----------------------------------------------------------------------------
// member function: get (virtual)
//
// Implementation defined
//-----------------------------------------------------------------------------

uvm_reg_data_t uvm_reg_indirect_data::get( const std::string& fname,
                                           int lineno ) const
{
  UVM_ERROR(get_full_name(), "Cannot get() an indirect data access register");
  return 0;
}

//-----------------------------------------------------------------------------
// member function: get_indirect_reg (virtual)
//
// Implementation defined
//-----------------------------------------------------------------------------

uvm_reg* uvm_reg_indirect_data::get_indirect_reg( const std::string& fname,
                                                  int lineno ) const
{
  unsigned int idx = m_idx->get_mirrored_value();
  return(m_tbl[idx]);
}

//-----------------------------------------------------------------------------
// member function: needs_update (virtual)
//
// Implementation defined
//-----------------------------------------------------------------------------

bool uvm_reg_indirect_data::needs_update() const
{
  return false;
}

//-----------------------------------------------------------------------------
// member function: write (virtual)
//
// Implementation defined
//-----------------------------------------------------------------------------

void uvm_reg_indirect_data::write( uvm_status_e& status,
                                   uvm_reg_data_t     value,
                                   uvm_path_e         path,
                                   uvm_reg_map*       map,
                                   uvm_sequence_base* parent,
                                   int                prior,
                                   uvm_object*        extension,
                                   const std::string& fname,
                                   int                lineno )
{
  if (path == UVM_DEFAULT_PATH)
  {
    uvm_reg_block* blk = get_parent();
    path = blk->get_default_path();
  }

  if (path == UVM_BACKDOOR)
  {
    UVM_WARNING(get_full_name(), "Cannot backdoor-write an indirect data access register. Switching to frontdoor.");
    path = UVM_FRONTDOOR;
  }

  // Can't simply call super.write() because it'll call set()
  {
    uvm_handle<uvm_reg_item>  rw;

    m_atomic_check_lock(1);

    rw = uvm_reg_item::type_id::create_handle("write_item", nullptr, get_full_name());
    rw->element      = this;
    rw->element_kind = UVM_REG;
    rw->access_kind  = UVM_WRITE;
    rw->value[0]     = value;
    rw->path         = path;
    rw->map          = map;
    rw->parent       = parent;
    rw->prior        = prior;
    rw->extension    = extension;
    rw->fname        = fname;
    rw->lineno       = lineno;

    do_write(rw);

    status = rw->status;

    m_atomic_check_lock(false);
  }
}

//-----------------------------------------------------------------------------
// member function: read (virtual)
//
// Implementation defined
//-----------------------------------------------------------------------------

void uvm_reg_indirect_data::read( uvm_status_e&      status,
                                  uvm_reg_data_t&    value,
                                  uvm_path_e         path,
                                  uvm_reg_map*       map,
                                  uvm_sequence_base* parent,
                                  int                prior,
                                  uvm_object*        extension,
                                  const std::string& fname,
                                  int                lineno )
{
  if (path == UVM_DEFAULT_PATH)
  {
    uvm_reg_block* blk = get_parent();
    path = blk->get_default_path();
  }

  if (path == UVM_BACKDOOR)
  {
    UVM_WARNING(get_full_name(), "Cannot backdoor-read an indirect data access register. Switching to frontdoor.");
    path = UVM_FRONTDOOR;
  }

  // call base class
  uvm_reg::read(status, value, path, map, parent, prior, extension, fname, lineno);
}

//-----------------------------------------------------------------------------
// member function: poke (virtual)
//
// Implementation defined
//-----------------------------------------------------------------------------

void uvm_reg_indirect_data::poke( uvm_status_e&      status,
                                  uvm_reg_data_t     value,
                                  const std::string& kind,
                                  uvm_sequence_base* parent,
                                  uvm_object*        extension,
                                  const std::string& fname,
                                  int                lineno )
{
  UVM_ERROR(get_full_name(), "Cannot poke() an indirect data access register");
  status = UVM_NOT_OK;
}

//-----------------------------------------------------------------------------
// member function: peek (virtual)
//
// Implementation defined
//-----------------------------------------------------------------------------

void uvm_reg_indirect_data::peek( uvm_status_e&      status,
                                  uvm_reg_data_t&    value,
                                  const std::string& kind,
                                  uvm_sequence_base* parent,
                                  uvm_object*        extension,
                                  const std::string& fname,
                                  int                lineno )
{
  UVM_ERROR(get_full_name(), "Cannot peek() an indirect data access register");
  status = UVM_NOT_OK;
}

//-----------------------------------------------------------------------------
// member function: update (virtual)
//
// Implementation defined
//-----------------------------------------------------------------------------

void uvm_reg_indirect_data::update( uvm_status_e&      status,
                                    uvm_path_e         path,
                                    uvm_reg_map*       map,
                                    uvm_sequence_base* parent,
                                    int                prior,
                                    uvm_object*        extension,
                                    const std::string& fname,
                                    int                lineno )
{
  status = UVM_IS_OK;
}

//-----------------------------------------------------------------------------
// member function: mirror (virtual)
//
// Implementation defined
//-----------------------------------------------------------------------------

void uvm_reg_indirect_data::mirror( uvm_status_e&      status,
                                    uvm_check_e        check,
                                    uvm_path_e         path,
                                    uvm_reg_map*       map,
                                    uvm_sequence_base* parent,
                                    int                prior,
                                    uvm_object*        extension,
                                    const std::string& fname,
                                    int                lineno )
{
  status = UVM_IS_OK;
}

} // namespace uvm
