//----------------------------------------------------------------------
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc.
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2012-2019 NXP B.V.
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

#include <systemc>

#include "uvmsc/seq/uvm_sequence_item.h"
#include "uvmsc/seq/uvm_sequence_base.h"
#include "uvmsc/seq/uvm_sequencer_base.h"
#include "uvmsc/report/uvm_report_object.h"
#include "uvmsc/report/uvm_report_handler.h"
#include "uvmsc/report/uvm_report_message.h"
#include "uvmsc/print/uvm_printer.h"

namespace uvm {

//----------------------------------------------------------------------
// constructors
//
//! In case the application does not give an explicit name to the sequence item
//! the transaction will become nameless. It is passed as an empty string
//! argument to avoid multiple constructors for the associated classes
//----------------------------------------------------------------------

uvm_sequence_item::uvm_sequence_item()
  : uvm_transaction( sc_core::sc_gen_unique_name("sequence_item") )
{
  m_sequencer = nullptr;
  print_sequence_info = false;
  m_parent_sequence = nullptr;
  m_use_sequence_info = true;
  m_sequence_id = -1;
  m_depth = -1;
}

uvm_sequence_item::uvm_sequence_item( uvm_object_name name_ )
  : uvm_transaction( name_ )
{
  m_sequencer = nullptr;
  print_sequence_info = false;
  m_parent_sequence = nullptr;
  m_use_sequence_info = true;
  m_sequence_id = -1;
  m_depth = -1;
}

//----------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------

uvm_sequence_item::~uvm_sequence_item()
{
  // TODO - cleanup
}

//----------------------------------------------------------------------
// member function: set_item_context
//
//! Set the sequence and sequencer execution context for a sequence item
//----------------------------------------------------------------------

void uvm_sequence_item::set_item_context( uvm_sequence_base* parent_seq,
                                          uvm_sequencer_base* sequencer)
{
  set_use_sequence_info(true);

  if (parent_seq)
    set_parent_sequence(parent_seq);

  if ((sequencer == nullptr) && (m_parent_sequence))
    sequencer = m_parent_sequence->get_sequencer();

  set_sequencer(sequencer);

  if (m_parent_sequence)
    set_depth(m_parent_sequence->get_depth() + 1);

  //reseed(); // TODO reseed?
}

//----------------------------------------------------------------------
// member function: set_use_sequence_info
//
//! The member function #set_use_sequence_info shall set the status of
//! the boolean variable #use_sequence_info.
//! Use_sequence_info controls whether the sequence information
//! (sequencer, parent_sequence, sequence_id, etc.) is printed, copied, or
//! recorded. When #use_sequence_info is the default value of false, then the
//! sequence information is not used. When use_sequence_info is set to true,
//! the sequence information will be used in printing and copying.
//----------------------------------------------------------------------

void uvm_sequence_item::set_use_sequence_info( bool value )
{
  m_use_sequence_info = value;
}

//----------------------------------------------------------------------
// member function: get_use_sequence_info
//
//! The member function #get_use_sequence_info shall get the status of
//! the boolean variable #use_sequence_info.
//! Use_sequence_info controls whether the sequence information
//! (sequencer, parent_sequence, sequence_id, etc.) is printed, copied, or
//! recorded. When #use_sequence_info is the default value of false, then the
//! sequence information is not used. When use_sequence_info is set to true,
//! the sequence information will be used in printing and copying.
//----------------------------------------------------------------------

bool uvm_sequence_item::get_use_sequence_info() const
{
  return m_use_sequence_info;
}

//----------------------------------------------------------------------
// member function: set_id_info
//
//! The member function #set_id_info shall copy the #sequence_id and
//! #transaction_id from the referenced item into
//! the calling item.  This function should always be used by drivers to
//! initialize responses for future compatibility.
//----------------------------------------------------------------------


void uvm_sequence_item::set_id_info(uvm_handle<uvm_sequence_item> item){
    this->set_transaction_id(item->get_transaction_id());
    this->set_sequence_id(item->get_sequence_id());
};
//----------------------------------------------------------------------
// member function: set_sequencer
//
//! The member function #set_sequencer shall set the default sequencer
//! for the sequence to sequencer.  It will take
//! effect immediately, so it should not be called while the sequence is
//! actively communicating with the sequencer.
//----------------------------------------------------------------------

void uvm_sequence_item::set_sequencer(uvm_sequencer_base* sequencer)
{
  m_sequencer = sequencer;
  m_set_p_sequencer();
}

//----------------------------------------------------------------------
// member function: get_sequencer
//
//! The member function #get_sequencer shall return a reference to the
//! default sequencer used by this sequence.
//----------------------------------------------------------------------

uvm_sequencer_base* uvm_sequence_item::get_sequencer() const
{
  return m_sequencer;
}

//----------------------------------------------------------------------
// member function: set_parent_sequence
//
//! The member function #set_parent_sequence shall set the parent sequence
//! of this #sequence_item.  This is used to identify the source sequence
//! of a sequence_item.
//----------------------------------------------------------------------

void uvm_sequence_item::set_parent_sequence( uvm_sequence_base* parent )
{
  m_parent_sequence = parent;
}

//----------------------------------------------------------------------
// member function: get_parent_sequence
//
//! Returns a reference to the parent sequence of any sequence on which this
//! method was called. If this is a parent sequence, the method returns nullptr.
//----------------------------------------------------------------------

uvm_sequence_base* uvm_sequence_item::get_parent_sequence() const
{
  return m_parent_sequence;
}

//----------------------------------------------------------------------
// member function: set_depth
//
//! The depth of any sequence is calculated automatically. However, the
//! application may use #set_depth to specify the depth of a particular
//! sequence. This member function will override the automatically calculated
//! depth, even if it is incorrect.
//----------------------------------------------------------------------

void uvm_sequence_item::set_depth( int value )
{
  m_depth = value;
}

//----------------------------------------------------------------------
// member function: get_depth
//
//! Returns the depth of a sequence from it's parent. A parent sequence will
//! have a depth of 1, it's child will have a depth  of 2, and it's grandchild
//! will have a depth of 3.
//----------------------------------------------------------------------

int uvm_sequence_item::get_depth() const
{
  // If depth has been set or calculated, then use that
  if (m_depth != -1)
    return (m_depth);

  // Calculate the depth, store it, and return the value
  if (m_parent_sequence == nullptr)
    m_depth = 1;
  else
    m_depth = m_parent_sequence->get_depth() + 1;

  return (m_depth);
}

//----------------------------------------------------------------------
// member function: is_item
//
//! This member function may be called on any #sequence_item or #sequence.
//! It will return true for items and false for sequences (which derive
//! from this class).
//----------------------------------------------------------------------

bool uvm_sequence_item::is_item() const
{
  return true;
}

//----------------------------------------------------------------------
// member function: get_root_sequence_name
//
//! Provides the name of the root sequence (the top-most parent sequence).
//----------------------------------------------------------------------

const std::string uvm_sequence_item::get_root_sequence_name() const
{
  const uvm_sequence_base* root_seq;
  root_seq = get_root_sequence();
  if (root_seq == nullptr)
    return "";
  else
    return root_seq->get_name();
}

//----------------------------------------------------------------------
// member function: get_root_sequence
//
//! Provides a reference to the root sequence (the top-most parent sequence).
//----------------------------------------------------------------------

const uvm_sequence_base* uvm_sequence_item::get_root_sequence() const
{
  const uvm_sequence_item* root_seq_base;
  const uvm_sequence_base* root_seq = nullptr;
  root_seq_base = this;

  while(true)
  {
    if( root_seq_base->get_parent_sequence() != nullptr )
    {
      root_seq_base = root_seq_base->get_parent_sequence();
      root_seq = dynamic_cast< const uvm_sequence_base*>(root_seq_base);
    }
    else
      return root_seq;
  }

  // TODO recursive loop - check if we always return a value
  return nullptr;
}

//----------------------------------------------------------------------
// member function: get_sequence_path
//
//! Provides a string of names of each sequence in the full hierarchical
//! path. The dot character "." is used as the separator between each sequence.

//----------------------------------------------------------------------

const std::string uvm_sequence_item::get_sequence_path() const
{
  const uvm_sequence_item* this_item;
  std::string seq_path;
  this_item = this;
  seq_path = this->get_name();

  while(true)
  {
    if(this_item->get_parent_sequence()!=nullptr)
    {
      this_item = this_item->get_parent_sequence();
      seq_path = this_item->get_name() + "." + seq_path;
    }
    else
      return seq_path;
  }

  // TODO recursive loop - check if we always return a value
  return "";
}

//----------------------------------------------------------------------
// Reporting interface
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: uvm_report (virtual)
//----------------------------------------------------------------------

void uvm_sequence_item::uvm_report( uvm_severity severity,
                                    const std::string& id,
                                    const std::string& message,
                                    int verbosity,
                                    const std::string& filename,
                                    int line,
                                    const std::string& context_name,
                                    bool report_enabled_checked ) const
{
  uvm_report_message* l_report_message;

  if (verbosity == -1)
    verbosity = (severity == UVM_ERROR) ? UVM_LOW :
                (severity == UVM_FATAL) ? UVM_NONE : UVM_MEDIUM;

  if (report_enabled_checked == 0)
  {
    if (!uvm_report_enabled(verbosity, severity, id))
      return;
  }

  l_report_message = uvm_report_message::new_report_message();
  l_report_message->set_report_message(severity, id, message,
                                      verbosity, filename, line, context_name);

  uvm_process_report_message(l_report_message);
}

//----------------------------------------------------------------------
// member function: uvm_report_info (virtual)
//----------------------------------------------------------------------

void uvm_sequence_item::uvm_report_info( const std::string& id,
                              const std::string& message,
                              int verbosity,
                              const std::string& filename,
                              int line,
                              const std::string& context_name,
                              bool report_enabled_checked ) const
{
  this->uvm_report(UVM_INFO, id, message, verbosity, filename, line,
                   context_name, report_enabled_checked);
}

//----------------------------------------------------------------------
// member function: uvm_report_warning (virtual)
//----------------------------------------------------------------------

void uvm_sequence_item::uvm_report_warning( const std::string& id,
                                            const std::string& message,
                                             int verbosity,
                                             const std::string& filename,
                                             int line,
                                             const std::string& context_name,
                                             bool report_enabled_checked ) const
{
  this->uvm_report(UVM_WARNING, id, message, verbosity, filename, line,
                   context_name, report_enabled_checked);
}

//----------------------------------------------------------------------
// member function: uvm_report_error (virtual)
//----------------------------------------------------------------------

void uvm_sequence_item::uvm_report_error( const std::string& id,
                                          const std::string& message,
                                          int verbosity,
                                          const std::string& filename,
                                          int line,
                                          const std::string& context_name,
                                          bool report_enabled_checked ) const
{
  this->uvm_report(UVM_ERROR, id, message, verbosity, filename, line,
                   context_name, report_enabled_checked);
}


//----------------------------------------------------------------------
// member function: uvm_report_fatal (virtual)
//----------------------------------------------------------------------

void uvm_sequence_item::uvm_report_fatal( const std::string& id,
                                          const std::string& message,
                                          int verbosity,
                                          const std::string& filename,
                                          int line,
                                          const std::string& context_name,
                                          bool report_enabled_checked ) const
{
  this->uvm_report(UVM_FATAL, id, message, verbosity, filename, line,
                   context_name, report_enabled_checked);
}

//----------------------------------------------------------------------
// member function: uvm_process_report_message (virtual)
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_sequence_item::uvm_process_report_message( uvm_report_message* report_message ) const
{
  //TODO avoid const_cast!
  uvm_report_object* l_report_object = const_cast<uvm_report_object*>(m_get_report_object());
  report_message->set_report_object(l_report_object);

  if (report_message->get_context() == "")
    report_message->set_context(get_sequence_path());

  l_report_object->m_rh->process_report_message(report_message);
}

//----------------------------------------------------------------------
// member function: m_get_report_object (virtual)
//
// Implementation defined
//----------------------------------------------------------------------

const uvm_report_object* uvm_sequence_item::m_get_report_object() const
{
  if(m_sequencer == nullptr)
  {
    uvm_coreservice_t* cs = uvm_coreservice_t::get();
    return cs->get_root();
  }
  else
    return m_sequencer;

}


//----------------------------------------------------------------------
// member function: uvm_report_enabled
//
// Implementation defined
//----------------------------------------------------------------------

bool uvm_sequence_item::uvm_report_enabled( int verbosity,
                                            uvm_severity severity,
                                            std::string id ) const
{
  const uvm_report_object* l_report_object = m_get_report_object();

  if (l_report_object->get_report_verbosity_level(severity, id) < verbosity)
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------
// member function: get_full_name
//
// Implementation-defined member function
// overrides must follow same naming convention
//----------------------------------------------------------------------

const std::string uvm_sequence_item::get_full_name() const
{
  std::string s;

  if(m_parent_sequence != nullptr)
    s = m_parent_sequence->get_full_name()+ ".";
  else
    if(m_sequencer!=nullptr)
      s = m_sequencer->get_full_name()+ ".";

  if(!get_name().empty())
    s += get_name();
  else
    s += "_item";

  return s;
}


//----------------------------------------------------------------------
// member function: get_sequence_id
//
// Implementation-defined member function
//
// Get_sequence_id is an internal method that is not intended for user code.
// The sequence_id is not a simple integer.  The get_transaction_id is meant
// for users to identify specific transactions.
//
// These methods allow access to the sequence_item sequence and transaction
// IDs. get_transaction_id and set_transaction_id are methods on the
// uvm_transaction base_class. These IDs are used to identify sequences to
// the sequencer, to route responses back to the sequence that issued a
// request, and to uniquely identify transactions.
//
// The sequence_id is assigned automatically by a sequencer when a sequence
// initiates communication through any sequencer calls (i.e. `uvm_do_xxx,
// wait_for_grant).  A sequence_id will remain unique for this sequence
// until it ends or it is killed.  However, a single sequence may have
// multiple valid sequence ids at any point in time.  Should a sequence
// start again after it has ended, it will be given a new unique sequence_id.
//
// The transaction_id is assigned automatically by the sequence each time a
// transaction is sent to the sequencer with the transaction_id in its
// default (-1) value.  If the user sets the transaction_id to any non-default
// value, that value will be maintained.
//
// Responses are routed back to this sequences based on sequence_id. The
// sequence may use the transaction_id to correlate responses with their
// requests.
//----------------------------------------------------------------------

int uvm_sequence_item::get_sequence_id() const
{
  return m_sequence_id;
}

//----------------------------------------------------------------------
// member function: set_sequence_id
//
// Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequence_item::set_sequence_id( int id )
{
  m_sequence_id = id;
}

//----------------------------------------------------------------------
// member function: m_set_p_sequencer
//
// Implementation-defined member function
//
// This member function shall be overloaded in the uvm_sequence_defines
// as part of the macro UVM_DECLARE_P_SEQUENCER(SEQUENCER)
//----------------------------------------------------------------------

void uvm_sequence_item::m_set_p_sequencer()
{
  return;
}

//----------------------------------------------------------------------
// member function: do_print
//
// Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequence_item::do_print( const uvm_printer& printer ) const
{
  std::string temp_str0, temp_str1;

  int depth = get_depth();
  uvm_transaction::do_print(printer);

  if( print_sequence_info || m_use_sequence_info )
  {
    printer.print_field_int("depth", depth, sizeof(depth)*CHAR_BIT, UVM_DEC, ".", "int");
    if(m_parent_sequence != nullptr)
    {
      temp_str0 = m_parent_sequence->get_name();
      temp_str1 = m_parent_sequence->get_full_name();
      printer.print_string("parent sequence (name)", temp_str0);
      printer.print_string("parent sequence (full name)", temp_str1);
    }
    temp_str1 = "";
    if(m_sequencer != nullptr)
    {
      temp_str1 = m_sequencer->get_full_name();
      printer.print_string("sequencer", temp_str1);
    }
  }
}

} /* namespace uvm */
