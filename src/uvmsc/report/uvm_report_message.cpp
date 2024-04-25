//----------------------------------------------------------------------
//   Copyright 2007-2010 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc. 
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2016 NXP B.V.
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
#include <string>

#include "uvmsc/report/uvm_report_message.h"
#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/print/uvm_printer.h"
#include "uvmsc/policy/uvm_recorder.h"

namespace uvm {

//----------------------------------------------------------------------
// Class: uvm_report_message_element_base
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: get_name (virtual)
//
// Get the name of the element
//----------------------------------------------------------------------

const std::string uvm_report_message_element_base::get_name() const
{
  return m_name;
}

//----------------------------------------------------------------------
// Member function: set_name (virtual)
//
// Set the name of the element
//----------------------------------------------------------------------

void uvm_report_message_element_base::set_name(const std::string& name)
{
  m_name = name;
}
     
//----------------------------------------------------------------------
// Member function: get_action (virtual)
//
// Get the authorized action for the element
//----------------------------------------------------------------------

uvm_action uvm_report_message_element_base::get_action() const
{
  return m_action;
}

//----------------------------------------------------------------------
// Member function: set_action (virtual)
//
// Set the authorized action for the element
//----------------------------------------------------------------------

void uvm_report_message_element_base::set_action(uvm_action action)
{
  m_action = action;
}

//----------------------------------------------------------------------
// Member function: print
//----------------------------------------------------------------------

void uvm_report_message_element_base::print(uvm_printer* printer) const
{
  if (m_action & (UVM_LOG | UVM_DISPLAY))
    do_print(*printer);
}

//----------------------------------------------------------------------
// Member function: record
//----------------------------------------------------------------------

void uvm_report_message_element_base::record(uvm_recorder* recorder)
{
  if (m_action & UVM_RM_RECORD)
    do_record(*recorder);
}

//----------------------------------------------------------------------
// Member function: copy
//----------------------------------------------------------------------

void uvm_report_message_element_base::copy( const uvm_report_message_element_base& rhs)
{
  do_copy(rhs);
}

//----------------------------------------------------------------------
// Member function: clone
//----------------------------------------------------------------------

uvm_report_message_element_base* uvm_report_message_element_base::clone()
{
  return do_clone();
}

//----------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------

uvm_report_message_element_base::~uvm_report_message_element_base()
{}


//----------------------------------------------------------------------
// Class: uvm_report_message_int_element
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: get_value (virtual)
//
// Get the value (integral type) of the element, with size and radix
//----------------------------------------------------------------------

uvm_bitstream_t uvm_report_message_int_element::get_value(
  int& size,
  uvm_radix_enum& radix ) const
{
  size = m_size;
  radix = m_radix;
  return m_val;
}

//----------------------------------------------------------------------
// Member function: set_value (virtual)
//
// Set the value (integral type) of the element, with size and radix
//----------------------------------------------------------------------

void uvm_report_message_int_element::set_value(
  uvm_bitstream_t value,
  int size,
  uvm_radix_enum radix )
{
  m_size = size;
  m_radix = radix;
  m_val = value;
}

//----------------------------------------------------------------------
// Member function: do_print (virtual)
//----------------------------------------------------------------------

void uvm_report_message_int_element::do_print(const uvm_printer& printer) const
{
  printer.print_field(m_name, m_val, m_size, m_radix);
}

//----------------------------------------------------------------------
// Member function: do_record (virtual)
//----------------------------------------------------------------------

void uvm_report_message_int_element::do_record(const uvm_recorder& recorder)
{
  // FIXME fix const
  uvm_recorder* rec = const_cast<uvm_recorder*>(&recorder);
  rec->record_field(m_name, m_val, m_size, m_radix);
}

//----------------------------------------------------------------------
// Member function: do_copy (virtual)
//----------------------------------------------------------------------

void uvm_report_message_int_element::do_copy( const uvm_report_message_element_base& rhs )
{
  const this_type* _rhs = dynamic_cast<const this_type*>(&rhs);
  m_name = _rhs->m_name;
  m_val = _rhs->m_val;
  m_size = _rhs->m_size;
  m_radix = _rhs->m_radix;
  m_action = rhs.m_action;
}

//----------------------------------------------------------------------
// Member function: do_copy (virtual)
//----------------------------------------------------------------------

uvm_report_message_element_base* uvm_report_message_int_element::do_clone()
{
  this_type* tmp = new uvm_report_message_int_element();
  tmp->copy(*this);
  return tmp;
}


//----------------------------------------------------------------------
// Class: uvm_report_message_string_element
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: get_value (virtual)
//
// Get the value (string type) of the element
//----------------------------------------------------------------------

const std::string uvm_report_message_string_element::get_value() const
{
  return m_val;
}

//----------------------------------------------------------------------
// Member function: set_value (virtual)
//
// Set the value (string type) of the element
//----------------------------------------------------------------------

void uvm_report_message_string_element::set_value(const std::string& value)
{
  m_val = value;
}

//----------------------------------------------------------------------
// Member function: do_print (virtual)
//----------------------------------------------------------------------

void uvm_report_message_string_element::do_print(const uvm_printer& printer) const
{
  printer.print_string(m_name, m_val);
}

//----------------------------------------------------------------------
// Member function: do_record (virtual)
//----------------------------------------------------------------------

void uvm_report_message_string_element::do_record(const uvm_recorder& recorder)
{
  // FIXME const cast
  uvm_recorder* lrec = const_cast<uvm_recorder*>(&recorder);
  lrec->record_string(m_name, m_val);
}

//----------------------------------------------------------------------
// Member function: do_copy (virtual)
//----------------------------------------------------------------------

void uvm_report_message_string_element::do_copy( const uvm_report_message_element_base& rhs )
{
  const this_type* _rhs = dynamic_cast<const this_type*>(&rhs);
  m_name = _rhs->m_name;
  m_val = _rhs->m_val;
  m_action = rhs.m_action;
}

//----------------------------------------------------------------------
// Member function: do_clone (virtual)
//----------------------------------------------------------------------

uvm_report_message_element_base* uvm_report_message_string_element::do_clone()
{
  this_type* tmp = new uvm_report_message_string_element();
  tmp->copy(*this);
  return tmp;
}

//----------------------------------------------------------------------
// Class: uvm_report_message_object_element
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Member function: get_value (virtual)
//
// Get the value (object reference) of the element
//----------------------------------------------------------------------

uvm_object* uvm_report_message_object_element::get_value() const
{
  return m_obj;
}

//----------------------------------------------------------------------
// Member function: set_value (virtual)
//
// Set the value (object reference) of the element
//----------------------------------------------------------------------

void uvm_report_message_object_element::set_value(uvm_object* obj)
{
  m_obj = obj;
}

//----------------------------------------------------------------------
// Member function: do_print (virtual)
//----------------------------------------------------------------------

void uvm_report_message_object_element::do_print(const uvm_printer& printer) const
{
  printer.print_object(m_name, *m_obj);
}

//----------------------------------------------------------------------
// Member function: do_record (virtual)
//----------------------------------------------------------------------

void uvm_report_message_object_element::do_record( const uvm_recorder& recorder )
{
  // FIXME const cast
  uvm_recorder* rec = const_cast<uvm_recorder*>(&recorder);
  rec->record_object(m_name, m_obj);
}

//----------------------------------------------------------------------
// Member function: do_copy (virtual)
//----------------------------------------------------------------------

void uvm_report_message_object_element::do_copy( const uvm_report_message_element_base& rhs )
{
  const this_type* _rhs = dynamic_cast<const this_type*>(&rhs);
  m_name = _rhs->m_name;
  m_obj = _rhs->m_obj;
  m_action = rhs.m_action;
}

//----------------------------------------------------------------------
// Member function: do_clone (virtual)
//----------------------------------------------------------------------

uvm_report_message_element_base* uvm_report_message_object_element::do_clone()
{
  this_type* tmp = new uvm_report_message_object_element();
  tmp->copy(*this);
  return tmp;
}

//----------------------------------------------------------------------
// Class: uvm_report_message_element_container
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// constructor
//
// Create a new uvm_report_message_element_container object
//----------------------------------------------------------------------

uvm_report_message_element_container::uvm_report_message_element_container(const std::string& name)
: uvm::uvm_object(name)
{}

//----------------------------------------------------------------------
// Member function: size (virtual)
//
// Returns the size of the container, i.e. the number of elements
//----------------------------------------------------------------------

unsigned int uvm_report_message_element_container::size() const
{
  return (unsigned int)m_elements.size();
}

//----------------------------------------------------------------------
// Member function: do_delete (virtual)
//
// Delete the ~index~-th element in the container
//----------------------------------------------------------------------

void uvm_report_message_element_container::do_delete(int index)
{
  m_elements.erase(m_elements.begin()+index);
}

//----------------------------------------------------------------------
// Member function: delete_elements (virtual)
//
// Delete all the elements in the container
//----------------------------------------------------------------------

void uvm_report_message_element_container::delete_elements()
{
  m_elements.clear();
}

//----------------------------------------------------------------------
// Member function: get_elements (virtual)
//
// Get all the elements from the container and put them in a queue
//----------------------------------------------------------------------

uvm_report_message_element_container::queue_of_element
  uvm_report_message_element_container::get_elements() const
{
  return m_elements;
}

//----------------------------------------------------------------------
// Member function: add_int (virtual)
//
// This member function adds an integral type of the name ~name~ and value ~value~ to
// the container.  The required ~size~ field indicates the size of ~value~.
// The required ~radix~ field determines how to display and
// record the field. The optional print/record bit is to specify whether
// the element will be printed/recorded.
//----------------------------------------------------------------------

void uvm_report_message_element_container::add_int(
  const std::string& name,
  uvm_bitstream_t value,
  int size,
  uvm_radix_enum radix,
  uvm_action action )
{
  // TODO
  //process p;
  //string rand_state;

  uvm_report_message_int_element* urme;

  //p = process::self();
  //if (p != null)
    //rand_state = p.get_randstate();

  urme = new uvm_report_message_int_element();

  //if (p != null)
    //p.set_randstate(rand_state);

  urme->set_name(name);
  urme->set_value(value, size, radix);
  urme->set_action(action);
  m_elements.push_back(urme);
}

//----------------------------------------------------------------------
// Member function: add_string (virtual)
//
// This member function adds a string of the name ~name~ and value ~value~ to the
// message. The optional print/record bit is to specify whether
// the element will be printed/recorded.
//----------------------------------------------------------------------

void uvm_report_message_element_container::add_string(
  const std::string& name,
  const std::string& value,
  uvm_action action )
{
  //TODO
  //process p;
  //string rand_state;
  uvm_report_message_string_element* urme;

  //p = process::self();
  //if (p != null)
    //rand_state = p.get_randstate();

  urme = new uvm_report_message_string_element();

  //if (p != null)
  //  p.set_randstate(rand_state);

  urme->set_name(name);
  urme->set_value(value);
  urme->set_action(action);
  m_elements.push_back(urme);
}

//----------------------------------------------------------------------
// Member function: add_object (virtual)
//
// This member function adds a uvm_object of the name ~name~ and reference ~obj~ to
// the message. The optional print/record bit is to specify whether
// the element will be printed/recorded.
//----------------------------------------------------------------------

void uvm_report_message_element_container::add_object(
  const std::string& name,
  uvm_object* obj,
  uvm_action action )
{
  //process p;
  //string rand_state;
  uvm_report_message_object_element* urme;

  //p = process::self();
  //if (p != null)
  //  rand_state = p.get_randstate();

  urme = new uvm_report_message_object_element();

  //if (p != null)
  //  p.set_randstate(rand_state);

  urme->set_name(name);
  urme->set_value(obj);
  urme->set_action(action);
  m_elements.push_back(urme);
}

//----------------------------------------------------------------------
// Member function: do_print (virtual)
//----------------------------------------------------------------------

void uvm_report_message_element_container::do_print( const uvm_printer& printer ) const
{
  uvm_printer lprinter = printer;

  uvm_object::do_print(lprinter);
  for(unsigned int i = 0; i < m_elements.size(); i++)
    m_elements[i]->print(&lprinter);
}

//----------------------------------------------------------------------
// Member function: do_record (virtual)
//----------------------------------------------------------------------

void uvm_report_message_element_container::do_record( const uvm_recorder& recorder )
{
  uvm_object::do_record(recorder);

  // TODO fix const
  uvm_recorder* l_recorder = const_cast<uvm_recorder*>(&recorder);

  for( unsigned int i = 0; i < m_elements.size(); i++)
    m_elements[i]->record(l_recorder);
}

//----------------------------------------------------------------------
// Member function: do_copy (virtual)
//----------------------------------------------------------------------

void uvm_report_message_element_container::do_copy( const uvm_object& rhs )
{
  const uvm_report_message_element_container* urme_container;

  uvm_object::do_copy(rhs);

  urme_container = dynamic_cast<const uvm_report_message_element_container*>(&rhs);

  if( urme_container==NULL )
    return;

  delete_elements();
  for( unsigned int i = 0; i < urme_container->size(); i++)
    m_elements.push_back(urme_container->m_elements[i]->clone()); // TODO check
}

//------------------------------------------------------------------------------
// Class: uvm_report_message
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Constructor
//
// Creates a new uvm_report_message object.
//------------------------------------------------------------------------------

uvm_report_message::uvm_report_message( const std::string& name )
: uvm_object(name)
{
  m_report_message_element_container = new uvm_report_message_element_container();
}

//------------------------------------------------------------------------------
// Member function: new_report_message (static)
//
// Creates a new uvm_report_message object.
// This function is the same as new(), but keeps the random stability.
//------------------------------------------------------------------------------

uvm_report_message* uvm_report_message::new_report_message( const std::string& name )
{
  //process p;
  //string rand_state;

  //p = process::self();

  //if (p != null)
  //  rand_state = p.get_randstate();
  static uvm_report_message* msg = new uvm_report_message(name);
  return msg;
  //if (p != null)
  //  p.set_randstate(rand_state);
}


//------------------------------------------------------------------------------
// Member function: print (virtual)
//
// The uvm_report_message implements <uvm_object::do_print()> such that
// ~print~ method provides UVM printer formatted output
// of the message.
//------------------------------------------------------------------------------

void uvm_report_message::do_print( const uvm_printer& printer ) const
{
  uvm_object::do_print(printer);

  printer.print_generic("severity", "uvm_severity", m_severity, uvm_severity_name[m_severity]);
  printer.print_string("id", m_id);
  printer.print_string("message", m_message);

  if ( (m_verbosity == UVM_NONE) ||
       (m_verbosity == UVM_LOW) ||
       (m_verbosity == UVM_MEDIUM) ||
       (m_verbosity == UVM_HIGH) ||
       (m_verbosity == UVM_FULL) ||
       (m_verbosity == UVM_DEBUG) )
    printer.print_generic("verbosity", "uvm_verbosity", m_verbosity, uvm_verbosity_name[m_verbosity]);
  else
  {
    std::ostringstream str;
    str << m_verbosity;
    printer.print_generic("verbosity", "uvm_verbosity", m_verbosity, str.str());
  }

  printer.print_string("filename", m_filename);
  printer.print_field_int("line", m_line, m_line, UVM_UNSIGNED);
  printer.print_string("context_name", m_context_name);

  // TODO fix const
  uvm_printer* l_printer = const_cast<uvm_printer*>(&printer);

  if (m_report_message_element_container->size() != 0)
    m_report_message_element_container->print(l_printer);
}

//------------------------------------------------------------------------------
// Member function: do_copy (virtual)
//
//------------------------------------------------------------------------------

void uvm_report_message::do_copy( const uvm_object& rhs )
{
  const uvm_report_message* report_message;

  uvm_object::do_copy(rhs);

  report_message =  dynamic_cast<const uvm_report_message*>(&rhs);

  if (report_message == NULL) return;

  m_report_object = report_message->get_report_object();
  m_report_handler = report_message->get_report_handler();
  m_report_server = report_message->get_report_server();
  m_context_name = report_message->get_context();
  m_file = report_message->get_file();
  m_filename = report_message->get_filename();
  m_line = report_message->get_line();
  m_action = report_message->get_action();
  m_severity = report_message->get_severity();
  m_id = report_message->get_id();
  m_message = report_message->get_message();
  m_verbosity = report_message->get_verbosity();

  m_report_message_element_container->copy(*report_message->m_report_message_element_container); // TODO check
}


//------------------------------------------------------------------------------
// Member function: get_report_object (virtual)
//
// Get the uvm_report_object that originated the message.
//------------------------------------------------------------------------------

uvm_report_object* uvm_report_message::get_report_object() const
{
  return m_report_object;
}

//------------------------------------------------------------------------------
// Member function: set_report_object (virtual)
//
// Set the uvm_report_object that originated the message.
//------------------------------------------------------------------------------

void uvm_report_message::set_report_object( uvm_report_object* ro )
{
  m_report_object = ro;
}

//------------------------------------------------------------------------------
// Member function: get_report_handler (virtual)
//
// Get the uvm_report_handler that is responsible for checking
//------------------------------------------------------------------------------

uvm_report_handler* uvm_report_message::get_report_handler() const
{
  return m_report_handler;
}

//------------------------------------------------------------------------------
// Member function: set_report_handler (virtual)
//
// Set the uvm_report_handler that is responsible for checking
// whether the message is enabled, should be upgraded/downgraded, etc.
//------------------------------------------------------------------------------

void uvm_report_message::set_report_handler( uvm_report_handler* rh )
{
  m_report_handler = rh;
}
  
//------------------------------------------------------------------------------
// Member function: get_report_server (virtual)
//
// Get the uvm_report_server that is responsible for servicing
// the message's actions.
//------------------------------------------------------------------------------

uvm_report_server* uvm_report_message::get_report_server() const
{
  return m_report_server;
}

//------------------------------------------------------------------------------
// Member function: set_report_server (virtual)
//
// Set the uvm_report_server that is responsible for servicing
// the message's actions.
//------------------------------------------------------------------------------

void uvm_report_message::set_report_server( uvm_report_server* rs )
{
  m_report_server = rs;
}

//------------------------------------------------------------------------------
// Member function: get_severity (virtual)
//
// Get the severity
//------------------------------------------------------------------------------

uvm_severity uvm_report_message::get_severity() const
{
  return m_severity;
}

//------------------------------------------------------------------------------
// Member function: set_severity (virtual)
//
// Set the severity (UVM_INFO, UVM_WARNING, UVM_ERROR or
// UVM_FATAL) of the message.  The value of this field is determined via
// the API used (UVM_INFO(), UVM_WARNING(), etc.) and populated for the user.
//------------------------------------------------------------------------------

void uvm_report_message::set_severity(uvm_severity sev)
{
  m_severity = sev;
}

//------------------------------------------------------------------------------
// Member function: get_id (virtual)
//
// Set the id of the message
//------------------------------------------------------------------------------

const std::string uvm_report_message::get_id() const
{
  return m_id;
}

//------------------------------------------------------------------------------
// Member function: set_id (virtual)
//
// Set the id of the message.  The value of this field is
// completely under user discretion.  Users are recommended to follow a
// consistent convention.  Settings in the uvm_report_handler allow various
// messaging controls based on this field.  See <uvm_report_handler>.
//------------------------------------------------------------------------------

void uvm_report_message::set_id( const std::string& id )
{
  m_id = id;
}

//------------------------------------------------------------------------------
// Member function: get_message (virtual)
//
// Get the user message content string.
//------------------------------------------------------------------------------

const std::string uvm_report_message::get_message() const
{
  return m_message;
}

//------------------------------------------------------------------------------
// Member function: set_message (virtual)
//
// Set the user message content string.
//------------------------------------------------------------------------------

void uvm_report_message::set_message( const std::string& msg )
{
  m_message = msg;
}

//------------------------------------------------------------------------------
// Member function: get_verbosity (virtual)
//
// Get the message verbosity threshold value.  This value is compared
// against settings in the <uvm_report_handler> to determine whether this
// message should be executed.
//------------------------------------------------------------------------------

int uvm_report_message::get_verbosity() const
{
  return m_verbosity;
}

//------------------------------------------------------------------------------
// Member function: set_verbosity (virtual)
//
// Set the message verbosity threshold value.  This value is compared
// against settings in the <uvm_report_handler> to determine whether this
// message should be executed.
//------------------------------------------------------------------------------


void uvm_report_message::set_verbosity(int ver)
{
  m_verbosity = ver;
}

//------------------------------------------------------------------------------
// Member function: get_filename (virtual)
//
// Get the file from which the message originates. This value
// is automatically populated by the messaging macros.
//------------------------------------------------------------------------------

const std::string uvm_report_message::get_filename() const
{
  return m_filename;
}

//------------------------------------------------------------------------------
// Member function: set_filename (virtual)
//
// Set the file from which the message originates. This value
// is automatically populated by the messaging macros.
//------------------------------------------------------------------------------

void uvm_report_message::set_filename( const std::string& fname)
{
  m_filename = fname;
}

//------------------------------------------------------------------------------
// Member function: get_line (virtual)
//
// Get the line in the ~file~ from which the message originates.
// This value is automatically populate by the messaging macros.
//------------------------------------------------------------------------------

int uvm_report_message::get_line() const
{
  return m_line;
}

//------------------------------------------------------------------------------
// Member function: set_line (virtual)
//
// Set the line in the ~file~ from which the message originates.
// This value is automatically populate by the messaging macros.
//------------------------------------------------------------------------------

void uvm_report_message::set_line(int ln)
{
  m_line = ln;
}

//------------------------------------------------------------------------------
// Member function: get_context (virtual)
//
// Get the optional user-supplied string that is meant to convey
// the context of the message.  It can be useful in scopes that are not
// inherently UVM like modules, interfaces, etc.
//------------------------------------------------------------------------------

const std::string uvm_report_message::get_context() const
{
  return m_context_name;
}

//------------------------------------------------------------------------------
// Member function: get_context (virtual)
//
// Set the optional user-supplied string that is meant to convey
// the context of the message.  It can be useful in scopes that are not
// inherently UVM like modules, interfaces, etc.
//------------------------------------------------------------------------------

void uvm_report_message::set_context( const std::string& cn )
{
  m_context_name = cn;
}

//------------------------------------------------------------------------------
// Member function: get_action (virtual)
//
// Get the action(s) that the uvm_report_server should perform
// for this message. This field is populated by the uvm_report_handler during
// message execution flow.
//------------------------------------------------------------------------------

uvm_action uvm_report_message::get_action() const
{
  return m_action;
}

//------------------------------------------------------------------------------
// Member function: set_action (virtual)
//
// Set the action(s) that the uvm_report_server should perform
// for this message. This field is populated by the uvm_report_handler during
// message execution flow.
//------------------------------------------------------------------------------

void uvm_report_message::set_action(uvm_action act)
{
  m_action = act;
}


//------------------------------------------------------------------------------
// Member function: get_file (virtual)
//
// Get the file that the message is to be written to when the
// message's action is UVM_LOG.  This field is populated by the
// uvm_report_handler during message execution flow.
//------------------------------------------------------------------------------


UVM_FILE uvm_report_message::get_file() const
{
  return m_file;
}

//------------------------------------------------------------------------------
// Member function: set_file (virtual)
//
// Set the file that the message is to be written to when the
// message's action is UVM_LOG.  This field is populated by the
// uvm_report_handler during message execution flow.
//------------------------------------------------------------------------------

void uvm_report_message::set_file(UVM_FILE fl)
{
  m_file = fl;
}


//------------------------------------------------------------------------------
// Member function: get_element_container (virtual)
//
// Get the element_container of the message
//------------------------------------------------------------------------------

uvm_report_message_element_container* uvm_report_message::get_element_container() const
{
  return m_report_message_element_container;
}

//------------------------------------------------------------------------------
// Member function: set_report_message (virtual)
//
// Set all the common fields of the report message in one shot.
//------------------------------------------------------------------------------

void uvm_report_message::set_report_message( uvm_severity severity,
    					     const std::string& id,
					     const std::string& message,
					     int verbosity,
					     const std::string& filename,
					     int line,
					     const std::string& context_name )
{
  m_context_name = context_name;
  m_filename = filename;
  m_line = line;
  m_severity = severity;
  m_id = id;
  m_message = message;
  m_verbosity = verbosity;
}

//------------------------------------------------------------------------------
// Member function: m_record_message (virtual)
//
// Implementation defined
//------------------------------------------------------------------------------

void uvm_report_message::m_record_message( const uvm_recorder& recorder )
{
  // FIXME fix const
  uvm_recorder* lrecorder = const_cast<uvm_recorder*>(&recorder);
  lrecorder->record_string("message", m_message);
}

//------------------------------------------------------------------------------
// Member function: m_record_core_properties (virtual)
//
// Implementation defined
//------------------------------------------------------------------------------

void uvm_report_message::m_record_core_properties( const uvm_recorder& recorder )
{
  // FIXME fix const
  uvm_recorder* lrecorder = const_cast<uvm_recorder*>(&recorder);

  if (!m_context_name.empty())
    lrecorder->record_string("context_name", m_context_name);

  lrecorder->record_string("filename", m_filename);
  lrecorder->record_field("line", m_line, m_line, UVM_UNSIGNED);
  lrecorder->record_string("severity", uvm_severity_name[m_severity]);

  if( (m_verbosity == UVM_NONE) ||
      (m_verbosity == UVM_LOW) ||
      (m_verbosity == UVM_MEDIUM) ||
      (m_verbosity == UVM_HIGH) ||
      (m_verbosity == UVM_FULL) ||
      (m_verbosity == UVM_DEBUG) )
    lrecorder->record_string("verbosity", uvm_verbosity_name[m_verbosity]);
  else
  {
    std::ostringstream str;
    str << m_verbosity;
    lrecorder->record_string("verbosity", str.str());
  }

  lrecorder->record_string("id", m_id);
  m_record_message(*lrecorder); // TODO check
}

//------------------------------------------------------------------------------
// Member function: do_record (virtual)
//
// Implementation defined
//------------------------------------------------------------------------------

void uvm_report_message::do_record( const uvm_recorder& recorder )
{
  uvm_object::do_record(recorder);

  m_record_core_properties(recorder);

  // TODO fix const
  uvm_recorder* l_recorder = const_cast<uvm_recorder*>(&recorder);

  m_report_message_element_container->record(l_recorder);
}

//------------------------------------------------------------------------------
// Member function: add_int (virtual)
//
// This method adds an integral type of the name ~name~ and value ~value~ to
// the message.  The required ~size~ field indicates the size of ~value~.
// The required ~radix~ field determines how to display and
// record the field. The optional print/record bit is to specify whether
// the element will be printed/recorded.
//------------------------------------------------------------------------------

void uvm_report_message::add_int( const std::string& name,
                                  uvm_bitstream_t value,
                                  int size,
                                  uvm_radix_enum radix,
                                  uvm_action action )
{
  m_report_message_element_container->add_int(name, value, size, radix, action);
}

//------------------------------------------------------------------------------
// Member function: add_string (virtual)
//
// This method adds a string of the name ~name~ and value ~value~ to the
// message. The optional print/record bit is to specify whether
// the element will be printed/recorded.
//------------------------------------------------------------------------------

void uvm_report_message::add_string( const std::string& name,
                                     const std::string& value,
                                     uvm_action action )
{
  m_report_message_element_container->add_string(name, value, action);
}

//------------------------------------------------------------------------------
// Member function: add_object (virtual)
//
// This method adds a uvm_object of the name ~name~ and reference ~obj~ to
// the message. The optional print/record bit is to specify whether
// the element will be printed/recorded.
//------------------------------------------------------------------------------

void uvm_report_message::add_object( const std::string& name,
                                     uvm_object* obj,
                                     uvm_action action )
{
  m_report_message_element_container->add_object(name, obj, action);
}

} // namespace uvm
