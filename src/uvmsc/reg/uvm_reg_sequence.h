//----------------------------------------------------------------------
//   Copyright 2013-2015 NXP B.V.
//   Copyright 2004-2009 Synopsys, Inc.
//   Copyright 2010-2011 Mentor Graphics Corporation
//   Copyright 2010-2011 Cadence Design Systems, Inc.
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2014 Université Pierre et Marie Curie, Paris
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

#ifndef UVM_REG_SEQUENCE_H_
#define UVM_REG_SEQUENCE_H_

#include <systemc>

#include "uvmsc/macros/uvm_object_defines.h"
#include "uvmsc/seq/uvm_sequencer.h"
#include "uvmsc/seq/uvm_sequence.h"
#include "uvmsc/factory/uvm_object_registry.h"
#include "uvmsc/reg/uvm_reg_item.h"
#include "uvmsc/reg/uvm_reg_map.h"
#include "uvmsc/reg/uvm_mem.h"
#include "uvmsc/base/uvm_port_base.h"

namespace uvm {


//forward class declarations
class uvm_reg_adapter;
class uvm_reg_block;

//----------------------------------------------------------------------
// CLASS: uvm_reg_sequence
//
//! This class provides base functionality for both user-defined RegModel test
//! sequences and "register translation sequences".
//!
//! - When used as a base for user-defined RegModel test sequences, this class
//!   provides convenience methods for reading and writing registers and
//!   memories. Users implement the body() method to interact directly with
//!   the RegModel model (held in the <model> property) or indirectly via the
//!   delegation methods in this class.
//!
//! - When used as a translation sequence, objects of this class are
//!   executed directly on a bus sequencer which are used in support of a layered sequencer
//!   use model, a pre-defined convert-and-execute algorithm is provided.
//!
//! Register operations do not require extending this class if none of the above
//! services are needed. Register test sequences can be extend from the base
//! <uvm_sequence<REQ, RSP> base class or even from outside a sequence.
//!
//! Note- The convenience API not yet implemented.
//----------------------------------------------------------------------

template <typename BASE = uvm_sequence<uvm_reg_item> >
class uvm_reg_sequence : public BASE
{
 public:

  UVM_OBJECT_PARAM_UTILS(uvm_reg_sequence<BASE>)

  explicit uvm_reg_sequence(const std::string& name = "uvm_reg_sequence_inst" );

  void body() override;

  virtual void do_reg_item( uvm_reg_item* rw );

  //----------------------------------
  // Group: Convenience Write/Read API
  //----------------------------------

  virtual void write_reg( uvm_reg*            rg,
                          uvm_status_e&       status, // output
                          uvm_reg_data_t      value,
                          uvm_path_e          path = UVM_DEFAULT_PATH,
                          uvm_reg_map*        map = nullptr,
                          int                 prior = -1,
                          uvm_object*         extension = nullptr,
                          const std::string&  fname = "",
                          int                 lineno = 0 );


   virtual void read_reg( uvm_reg*            rg,
                          uvm_status_e&       status, // output
                          uvm_reg_data_t&     value, // output
                          uvm_path_e          path = UVM_DEFAULT_PATH,
                          uvm_reg_map*        map = nullptr,
                          int                 prior = -1,
                          uvm_object*         extension = nullptr,
                          const std::string&  fname = "",
                          int                 lineno = 0 );

   virtual void poke_reg( uvm_reg*           rg,
                          uvm_status_e&      status, // output
                          uvm_reg_data_t     value,
                          const std::string& kind = "",
                          uvm_object*        extension = nullptr,
                          const std::string& fname = "",
                          int                lineno = 0 );

   virtual void peek_reg( uvm_reg*            rg,
                          uvm_status_e&       status, // output
                          uvm_reg_data_t&     value, // output
                          const std::string&  kind = "",
                          uvm_object*         extension = nullptr,
                          const std::string&  fname = "",
                          int                 lineno = 0 );
   
   virtual void update_reg( uvm_reg*           rg,
                            uvm_status_e&      status,
                            uvm_path_e         path = UVM_DEFAULT_PATH,
                            uvm_reg_map*       map = nullptr,
                            int                prior = -1,
                            uvm_object*        extension = nullptr,
                            const std::string& fname = "",
                            int                lineno = 0 );

   virtual void mirror_reg( uvm_reg*           rg,
                            uvm_status_e&      status, // output
                            uvm_check_e        check  = UVM_NO_CHECK,
                            uvm_path_e         path = UVM_DEFAULT_PATH,
                            uvm_reg_map*       map = nullptr,
                            int                prior = -1,
                            uvm_object*        extension = nullptr,
                            const std::string& fname = "",
                            int                lineno = 0 );

   virtual void write_mem( uvm_mem*           mem,
                           uvm_status_e&      status, // output
                           uvm_reg_addr_t     offset,
                           uvm_reg_data_t     value,
                           uvm_path_e         path = UVM_DEFAULT_PATH,
                           uvm_reg_map*       map = nullptr,
                           int                prior = -1,
                           uvm_object*        extension = nullptr,
                           const std::string& fname = "",
                           int                lineno = 0 );

   virtual void read_mem( uvm_mem*            mem,
                          uvm_status_e&       status, // output
                          uvm_reg_addr_t      offset,
                          uvm_reg_data_t&     value, // output
                          uvm_path_e          path = UVM_DEFAULT_PATH,
                          uvm_reg_map*        map = nullptr,
                          int                 prior = -1,
                          uvm_object*         extension = nullptr,
                          const std::string&  fname = "",
                          int                 lineno = 0 );

   virtual void poke_mem( uvm_mem*       mem,
                          uvm_status_e&  status, // output
                          uvm_reg_addr_t offset,
                          uvm_reg_data_t value,
                          const std::string&         kind = "",
                          uvm_object*    extension = nullptr,
                          const std::string&         fname = "",
                          int            lineno = 0 );

   virtual void peek_mem( uvm_mem*        mem,
                          uvm_status_e&   status, // output
                          uvm_reg_addr_t  offset,
                          uvm_reg_data_t& value, // output
                          const std::string&          kind = "",
                          uvm_object*     extension = nullptr,
                          const std::string&          fname = "",
                          int             lineno = 0 );

  //----------------------------------------------------------------------
  // Variable: model
  //
  //! Block abstraction this sequence executes on, defined only when this
  //! sequence is a user-defined test sequence.
  //----------------------------------------------------------------------
  uvm_reg_block* model;


  //----------------------------------------------------------------------
  // Variable: adapter
  //
  //! Adapter to use for translating between abstract register transactions
  //! and physical bus transactions, defined only when this sequence is a
  //! translation sequence.
  //----------------------------------------------------------------------
  uvm_reg_adapter* adapter;


  //----------------------------------------------------------------------
  // Variable: reg_seqr
  //
  //! Layered upstream "register" sequencer.
  //!
  //! Specifies the upstream sequencer between abstract register transactions
  //! and physical bus transactions. Defined only when this sequence is a
  //! translation sequence, and we want to "pull" from an upstream sequencer.
  //----------------------------------------------------------------------
  uvm_sequencer<uvm_reg_item>* reg_seqr;

  typedef enum { LOCAL, UPSTREAM } seq_parent_e;

 private:

  void put_response( const uvm_sequence_item& response_item ) override;

  // other local data members

  seq_parent_e parent_select; // default set to LOCAL in constructor

  uvm_sequence_base* upstream_parent;

}; // class uvm_reg_sequence


//----------------------------------------------------------------------
// Constructor
//
//! Create a new instance, giving it the optional ~name~.
//----------------------------------------------------------------------

template <typename BASE>
uvm_reg_sequence<BASE>::uvm_reg_sequence( const std::string& name ) : BASE(name)
{
  model = nullptr;
  adapter = nullptr;
  reg_seqr = nullptr;
  upstream_parent = nullptr;

  parent_select = LOCAL;
}


//----------------------------------------------------------------------
// Member function: body
//
//! Continually gets a register transaction from the configured upstream
//! sequencer, reg_seqr, and executes the corresponding bus transaction
//! via #do_rw_access.
//!
//! User-defined RegModel test sequences must override body() and not call
//! the member function body() from the base class,
//! else a warning will be issued and the calling process not return.
//----------------------------------------------------------------------

template <typename BASE>
void uvm_reg_sequence<BASE>::body()
{
  if (this->m_sequencer == nullptr)
  {
    UVM_FATAL("NO_SEQR", std::string("Sequence executing as translation sequence, ") +
    	std::string("but is not associated with a sequencer (m_sequencer == )") );
  }
  if (reg_seqr == nullptr)
  {
    UVM_WARNING("REG_XLATE_NO_SEQR",
      "Executing RegModel translation sequence on sequencer " +
     this->m_sequencer->get_full_name() +
      "' does not have an upstream sequencer defined. " +
      "Execution of register items available only via direct calls to 'do_rw_access'");

    sc_core::sc_event _forever; // shall never be notified!
    sc_core::wait(_forever);
  }

  UVM_INFO("REG_XLATE_SEQ_START",
    "Starting RegModel translation sequence on sequencer " +
     this->m_sequencer->get_full_name() + "'" , UVM_LOW);

  while (true) // forever
  {
    // TODO check correctness
    uvm_reg_item reg_item;
    reg_seqr->peek(reg_item);
    do_reg_item(&reg_item);
    reg_seqr->get(reg_item);
    sc_core::wait(sc_core::SC_ZERO_TIME);
  }
}


//----------------------------------------------------------------------
// Member function: do_reg_item
//
//! Executes the given register transaction, \p rw, via the sequencer on
//! which this sequence was started (i.e. m_sequencer). Uses the configured
//! #adapter to convert the register transaction into the type expected by
//! this sequencer.
//----------------------------------------------------------------------

template <typename BASE>
void uvm_reg_sequence<BASE>::do_reg_item( uvm_reg_item* rw )
{
  if (this->m_sequencer == nullptr)
    UVM_FATAL("REG/DO_ITEM/NULL", "do_reg_item: m_sequencer is nullptr");
  if (adapter == nullptr)
    UVM_FATAL("REG/DO_ITEM/NULL", "do_reg_item: adapter handle is nullptr");

  UVM_INFO("DO_RW_ACCESS", std::string("Doing transaction: ") + rw->convert2string(), UVM_HIGH);

  if (parent_select == LOCAL)
  {
    upstream_parent = rw->parent;
    rw->parent = this;
  }

  if (rw->access_kind == UVM_WRITE)
    rw->local_map->do_bus_write(rw, this->m_sequencer, adapter);
  else
    rw->local_map->do_bus_read(rw, this->m_sequencer, adapter);

  if (parent_select == LOCAL)
     rw->parent = upstream_parent;
}


//----------------------------------------------------------------------
// Group: Convenience Write/Read API
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: write_reg
//
//! Writes the given register \p rg using uvm_reg::write, supplying 'this' as
//! the parent argument.
//----------------------------------------------------------------------

template <typename BASE>
void uvm_reg_sequence<BASE>::write_reg( uvm_reg* rg,
                                        uvm_status_e& status,
                                        uvm_reg_data_t value,
                                        uvm_path_e path,
                                        uvm_reg_map* map,
                                        int prior,
                                        uvm_object* extension,
                                        const std::string& fname,
                                        int  lineno )
{
  if (rg == nullptr)
  {
    UVM_ERROR("NO_REG", "Register argument is nullptr");
  }
  else
    rg->write(status, value, path, map, this, prior, extension, fname, lineno);
}

//----------------------------------------------------------------------
// Member function: read_reg
//
//! Reads the given register \p rg using uvm_reg::read, supplying 'this' as
//! the parent argument.
//----------------------------------------------------------------------

template <typename BASE>
void uvm_reg_sequence<BASE>::read_reg( uvm_reg* rg,
                                       uvm_status_e& status,
                                       uvm_reg_data_t& value,
                                       uvm_path_e path,
                                       uvm_reg_map* map,
                                       int prior,
                                       uvm_object* extension,
                                       const std::string& fname,
                                       int lineno )
{
  if (rg == nullptr)
  {
    UVM_ERROR("NO_REG", "Register argument is nullptr");
  }
  else
    rg->read(status, value, path, map, this, prior, extension, fname, lineno);
}


//----------------------------------------------------------------------
// Member function: poke_reg
//
//! Pokes the given register \p rg using uvm_reg::poke, supplying 'this' as
//! the parent argument.
//----------------------------------------------------------------------

template <typename BASE>
void uvm_reg_sequence<BASE>::poke_reg( uvm_reg* rg,
                                       uvm_status_e& status,
                                       uvm_reg_data_t value,
                                       const std::string& kind,
                                       uvm_object* extension,
                                       const std::string& fname,
                                       int lineno )
{
  if (rg == nullptr)
  {
    UVM_ERROR("NO_REG", "Register argument is nullptr");
  }
  else
    rg->poke(status, value, kind, this, extension, fname, lineno);
}



//----------------------------------------------------------------------
// Member function: peek_reg
//
//! Peeks the given register \p rg using uvm_reg::peek, supplying 'this' as
//! the parent argument.
//----------------------------------------------------------------------

template <typename BASE>
void uvm_reg_sequence<BASE>::peek_reg( uvm_reg* rg,
                                       uvm_status_e& status,
                                       uvm_reg_data_t& value,
                                       const std::string& kind,
                                       uvm_object* extension,
                                       const std::string& fname,
                                       int lineno )
{
  if (rg == nullptr)
  {
    UVM_ERROR("NO_REG", "Register argument is nullptr");
  }
  else
   rg->peek(status, value, kind, this, extension, fname, lineno);
}


//----------------------------------------------------------------------
// Member function: update_reg
//
//! Updates the given register \p rg using uvm_reg::update, supplying 'this' as
//! the parent argument.
//----------------------------------------------------------------------

template <typename BASE>
void uvm_reg_sequence<BASE>::update_reg( uvm_reg* rg,
                                         uvm_status_e& status,
                                         uvm_path_e path,
                                         uvm_reg_map* map,
                                         int prior,
                                         uvm_object* extension,
                                         const std::string& fname,
                                         int lineno )
{
  if (rg == nullptr)
  {
     UVM_ERROR("NO_REG", "Register argument is nullptr");
  }
  else
    rg->update(status, path, map, this, prior, extension, fname, lineno);
}


//----------------------------------------------------------------------
// Member function: mirror_reg
//
//! Mirrors the given register \p rg using uvm_reg::mirror, supplying 'this' as
//! the parent argument.
//----------------------------------------------------------------------

template <typename BASE>
void uvm_reg_sequence<BASE>::mirror_reg( uvm_reg* rg,
                                         uvm_status_e& status,
                                         uvm_check_e check,
                                         uvm_path_e path,
                                         uvm_reg_map* map,
                                         int prior,
                                         uvm_object* extension,
                                         const std::string& fname,
                                         int lineno )
{
  if (rg == nullptr)
  {
    UVM_ERROR("NO_REG", "Register argument is nullptr");
  }
  else
    rg->mirror(status, check, path, map, this, prior, extension, fname, lineno);
}



//----------------------------------------------------------------------
// Member function: write_mem
//
//! Writes the given memory \p mem using uvm_mem::write, supplying 'this' as
//! the parent argument.
//----------------------------------------------------------------------

template <typename BASE>
void uvm_reg_sequence<BASE>::write_mem( uvm_mem* mem,
                                        uvm_status_e& status,
                                        uvm_reg_addr_t offset,
                                        uvm_reg_data_t value,
                                        uvm_path_e path,
                                        uvm_reg_map* map,
                                        int prior,
                                        uvm_object* extension,
                                        const std::string& fname,
                                        int lineno )
{
  if (mem == nullptr)
  {
    UVM_ERROR("NO_MEM", "Memory argument is nullptr");
  }
  else
    mem->write(status, offset, value, path, map, this, prior, extension, fname, lineno);
}


//----------------------------------------------------------------------
// Member function: read_mem
//
//! Reads the given memory \p mem using uvm_mem::read, supplying 'this' as
//! the parent argument.
//----------------------------------------------------------------------

template <typename BASE>
void uvm_reg_sequence<BASE>::read_mem( uvm_mem* mem,
                                       uvm_status_e& status,
                                       uvm_reg_addr_t offset,
                                       uvm_reg_data_t& value,
                                       uvm_path_e path,
                                       uvm_reg_map* map,
                                       int prior,
                                       uvm_object* extension,
                                       const std::string& fname,
                                       int lineno )
{
  if (mem == nullptr)
  {
    UVM_ERROR("NO_MEM", "Memory argument is nullptr");
  }
  else
    mem->read(status, offset, value, path, map, this, prior, extension, fname, lineno);
}


//----------------------------------------------------------------------
// Member function: poke_mem
//
//! Pokes the given memory \p mem using uvm_mem::poke, supplying 'this' as
//! the parent argument.
//----------------------------------------------------------------------

template <typename BASE>
void uvm_reg_sequence<BASE>::poke_mem( uvm_mem* mem,
                                       uvm_status_e& status,
                                       uvm_reg_addr_t offset,
                                       uvm_reg_data_t value,
                                       const std::string& kind,
                                       uvm_object* extension,
                                       const std::string& fname,
                                       int lineno )
{
   if (mem == nullptr)
   {
     UVM_ERROR("NO_MEM", "Memory argument is nullptr");
   }
   else
     mem->poke(status, offset, value, kind, this, extension, fname, lineno);
}


//----------------------------------------------------------------------
// Member function: peek_mem
//
//! Peeks the given memory \p mem using uvm_mem::peek, supplying 'this' as
//! the parent argument.
//----------------------------------------------------------------------

template <typename BASE>
void uvm_reg_sequence<BASE>::peek_mem( uvm_mem* mem,
                                       uvm_status_e& status,
                                       uvm_reg_addr_t offset,
                                       uvm_reg_data_t& value,
                                       const std::string& kind,
                                       uvm_object* extension,
                                       const std::string& fname,
                                       int lineno )
{
  if (mem == nullptr)
  {
    UVM_ERROR("NO_MEM", "Memory argument is nullptr");
  }
  else
    mem->peek(status, offset, value, kind, this, extension, fname, lineno);
}


//----------------------------------------------------------------------
// Member function: put_response
//
//! This member function is not user visible. Needed to populate this sequence's response
//! queue with any bus item type.
//----------------------------------------------------------------------

template <typename BASE>
void uvm_reg_sequence<BASE>::put_response( const uvm_sequence_item& response_item )
{
  this->put_base_response(response_item);
}


} // namespace uvm

#endif // UVM_REG_SEQUENCE_H_
