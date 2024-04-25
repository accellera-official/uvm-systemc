//----------------------------------------------------------------------------
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2007-2010 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc. 
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2014 NVIDIA Corporation
//   Copyright 2016 NXP B.V.
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
//----------------------------------------------------------------------------

#include <systemc>

#include <ostream>
#include <iostream>
#include <iomanip>  // setw

#include "uvmsc/report/uvm_default_report_server.h"
#include "uvmsc/macros/uvm_string_defines.h"
#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/report/uvm_report_server.h"
#include "uvmsc/report/uvm_report_message.h"
#include "uvmsc/report/uvm_report_catcher.h"
#include "uvmsc/report/uvm_report_handler.h"
#include "uvmsc/print/uvm_printer.h"
#include "uvmsc/base/uvm_globals.h"
#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"
#include "uvmsc/print/uvm_printer_globals.h"

namespace uvm {

//----------------------------------------------------------------------------
// Constructor
//
//! Creates an instance of the class.
//----------------------------------------------------------------------------

uvm_default_report_server::uvm_default_report_server( const std::string& name )
: uvm_report_server(name)
{
  max_quit_overridable = true;

  enable_report_id_count_summary = true;
  record_all_messages = false;
  show_verbosity = false;
  show_terminator = false;

  set_max_quit_count(0);
  reset_quit_count();
  reset_severity_counts();
}

//----------------------------------------------------------------------------
// member function: do_print (virtual)
//
// The uvm_default_report_server implements the <uvm_object::do_print()> such that
// ~print~ method provides UVM printer formatted output
// of the current configuration.
//----------------------------------------------------------------------------

void uvm_default_report_server::do_print( const uvm_printer& printer ) const
{
  uvm_severity l_severity_count_index;
  std::string l_id_count_index;

  printer.print_field_int("quit_count", m_quit_count, m_quit_count, UVM_DEC, ".", "int");
  printer.print_field_int("max_quit_count", m_max_quit_count, m_max_quit_count, UVM_DEC, ".", "int");
  printer.print_field_int("max_quit_overridable", max_quit_overridable,
      max_quit_overridable, UVM_BIN, ".", "bit");

  if (m_severity_count.size() != 0)
  {
    printer.print_array_header("severity_count", (int)m_severity_count.size(), "severity counts");

    for( severity_count_citt it = m_severity_count.begin();
         it != m_severity_count.end(); ++it)
    {
      l_severity_count_index = it->first;
      std::ostringstream str;
      str << "[" << uvm_severity_name[l_severity_count_index] << "]";
      printer.print_field_int(str.str(),
          m_severity_count[l_severity_count_index], 32, UVM_DEC);
    }
    printer.print_array_footer();
  }

  if( m_id_count.size() != 0 )
  {
    printer.print_array_header("id_count", (int)m_id_count.size(), "id counts");

    for( id_count_itt it = m_id_count.begin();
         it != m_id_count.end(); ++it)
    {
      l_id_count_index = it->first;
      std::ostringstream str;
      str << "[" << l_id_count_index << "]";
      printer.print_field_int(str.str(),
        m_id_count[l_id_count_index], 32, UVM_DEC);
    }
    printer.print_array_footer();
  }

  printer.print_field_int("enable_report_id_count_summary", enable_report_id_count_summary,
      enable_report_id_count_summary, UVM_BIN, ".", "bool");
  printer.print_field_int("record_all_messages", record_all_messages,
      record_all_messages, UVM_BIN, ".", "bool");
  printer.print_field_int("show_verbosity", show_verbosity,
      show_verbosity, UVM_BIN, ".", "bool");
  printer.print_field_int("show_terminator", show_terminator,
      show_terminator, UVM_BIN, ".", "bool");
}

//----------------------------------------------------------------------------
// member function: get_max_quit_count
//
//! Get the maximum number of count actions that can be tolerated
//! before an UVM_EXIT action is taken.
//----------------------------------------------------------------------------

int uvm_default_report_server::get_max_quit_count() const
{
  return m_max_quit_count;
}

//----------------------------------------------------------------------------
// member function: set_max_quit_count
//
//! Set the maximum number of count actions that can be tolerated
//! before an UVM_EXIT action is taken. The default is 0, which specifies
//! no maximum.
//----------------------------------------------------------------------------

void uvm_default_report_server::set_max_quit_count( int count, bool overridable )
{
  if (!max_quit_overridable)
  {
    std::ostringstream str;
    str << "The max quit count setting of "
        << m_max_quit_count
        << " is not overridable to "
        << count
        << " due to a previous setting.";
    uvm_report_info("NOMAXQUITOVR", str.str(), UVM_NONE);
    return;
  }

  max_quit_overridable = overridable;
  m_max_quit_count = (count < 0) ? 0 : count;
}

//----------------------------------------------------------------------------
// member function: get_quit_count
//----------------------------------------------------------------------------

int uvm_default_report_server::get_quit_count() const
{
  return m_quit_count;
}

//----------------------------------------------------------------------------
// member function: set_quit_count
//----------------------------------------------------------------------------

void uvm_default_report_server::set_quit_count(int quit_count)
{
  m_quit_count = (quit_count < 0) ? 0 : quit_count;
}

//----------------------------------------------------------------------------
// member function: incr_quit_count
//----------------------------------------------------------------------------

void uvm_default_report_server::incr_quit_count()
{
  m_quit_count++;
}

//----------------------------------------------------------------------------
// member function: reset_quit_count
//
// Set, get, increment, or reset to 0 the quit count, i.e., the number of
// COUNT actions issued.
//----------------------------------------------------------------------------

void uvm_default_report_server::reset_quit_count()
{
  m_quit_count = 0;
}

//----------------------------------------------------------------------------
// member function: is_quit_count_reached
//
// If is_quit_count_reached returns 1, then the quit counter has reached
// the maximum.
//----------------------------------------------------------------------------

bool uvm_default_report_server::is_quit_count_reached()
{
  return (m_quit_count >= m_max_quit_count);
}

//----------------------------------------------------------------------------
// member function: get_severity_count
//----------------------------------------------------------------------------

int uvm_default_report_server::get_severity_count( uvm_severity severity ) const
{
  return m_severity_count.find(severity)->second;
}

//----------------------------------------------------------------------------
// member function: set_severity_count
//----------------------------------------------------------------------------

void uvm_default_report_server::set_severity_count( uvm_severity severity, int count )
{
  m_severity_count[severity] = (count < 0) ? 0 : count;
}

//----------------------------------------------------------------------------
// member function: incr_severity_count
//----------------------------------------------------------------------------

void uvm_default_report_server::incr_severity_count( uvm_severity severity )
{
  m_severity_count[severity]++;
}

//----------------------------------------------------------------------------
// member function: reset_severity_counts
//
// Set, get, or increment the counter for the given severity, or reset
// all severity counters to 0.
//----------------------------------------------------------------------------

void uvm_default_report_server::reset_severity_counts()
{
  for( unsigned int i = 0; i < 4; i++) // cycle through all four states
    m_severity_count[(uvm_severity)i] = 0; // TODO check
}

//----------------------------------------------------------------------------
// member function: get_id_count
//----------------------------------------------------------------------------

int uvm_default_report_server::get_id_count( const std::string& id ) const
{
  if(m_id_count.find(id) != m_id_count.end()) // exists
    return m_id_count.find(id)->second;

  return 0;
}

//----------------------------------------------------------------------------
// member function: set_id_count
//----------------------------------------------------------------------------

void uvm_default_report_server::set_id_count( const std::string& id, int count )
{
  m_id_count[id] = (count < 0) ? 0 : count;
}

//----------------------------------------------------------------------------
// member function: incr_id_count
//
// Set, get, or increment the counter for reports with the given id.
//----------------------------------------------------------------------------

void uvm_default_report_server::incr_id_count( const std::string& id )
{
  if( m_id_count.find(id) != m_id_count.end() ) // exists
    m_id_count[id]++;
  else
    m_id_count[id] = 1;
}

/* TODO transaction recording
//----------------------------------------------------------------------------
// member function: set_message_database
//
//! Sets the uvm_tr_database used for recording messages
//----------------------------------------------------------------------------

void uvm_default_report_server::set_message_database( uvm_tr_database* database )
{
  m_message_db = database;
}

//----------------------------------------------------------------------------
// member function: get_message_database
//
//! Returns the uvm_tr_database used for recording messages
//----------------------------------------------------------------------------

uvm_tr_database* uvm_default_report_server::get_message_database() const
{
  return m_message_db;
}

*/

//----------------------------------------------------------------------------
// member function: get_severity_set
//
//----------------------------------------------------------------------------

void uvm_default_report_server::get_severity_set( std::vector<uvm_severity>& q ) const
{
  for( severity_count_citt it = m_severity_count.begin();
       it != m_severity_count.end(); ++it)
  {
    uvm_severity sev = it->first;
    q.push_back(sev);
  }
}

//----------------------------------------------------------------------------
// member function: get_id_set
//
//----------------------------------------------------------------------------

void uvm_default_report_server::get_id_set( std::vector<std::string>& q ) const
{
  for( id_count_citt it = m_id_count.begin();
       it != m_id_count.end(); ++it)
  {
    std::string idx = it->first;
    q.push_back(idx);
  }
}

//----------------------------------------------------------------------------
// member function: f_display
//
//! This member function sends string severity to the command line if file is 0 and to
//! the file(s) specified by file if it is not 0.
//----------------------------------------------------------------------------

void uvm_default_report_server::f_display( UVM_FILE file, const std::string& str ) const
{
  if (file == 0) // NULL pointer
    std::cout << str << std::endl;
  else
    *file << str << std::endl;
}

//----------------------------------------------------------------------------
// member function: process_report_message
//
//----------------------------------------------------------------------------

void uvm_default_report_server::process_report_message( uvm_report_message* report_message )
{
  bool report_ok = true;

  // Set the report server for this message
  report_message->set_report_server(this);

  if(report_ok)
    report_ok = uvm_report_catcher::process_all_report_catchers(report_message);

  if(report_message->get_action() == UVM_NO_ACTION)
    report_ok = false;

  if(report_ok)
  {
    std::string m;
    uvm_coreservice_t* cs = uvm_coreservice_t::get();
    // give the global server a chance to intercept the calls
    uvm_report_server* svr = cs->get_report_server();

    // no need to compose when neither UVM_DISPLAY nor UVM_LOG is set
    if (report_message->get_action() & (UVM_LOG|UVM_DISPLAY))
      m = svr->compose_report_message(report_message);

    svr->execute_report_message(report_message, m);
  }
}

//----------------------------------------------------------------------------
// member function: execute_report_message (virtual)
//
// Processes the provided message per the actions contained within.
// Expert users can overload this method to customize action processing.
//----------------------------------------------------------------------------

void uvm_default_report_server::execute_report_message(
    uvm_report_message* report_message,
    const std::string& composed_message )
{
  // Update counts
  incr_severity_count(report_message->get_severity());
  incr_id_count(report_message->get_id());

  if (record_all_messages)
    report_message->set_action(report_message->get_action() | UVM_RM_RECORD);

  /* TODO transaction recording

  // UVM_RM_RECORD action
  if(report_message->get_action() & UVM_RM_RECORD)
  {
    uvm_tr_stream* stream;

    uvm_report_object ro = report_message->get_report_object();
    uvm_report_handler rh = report_message->get_report_handler();

    // Check for pre-existing stream
    if( (m_streams.find(ro.get_name()) != m_streams.end()) &&
        (m_streams[ro.get_name()].find(rh.get_name())) != m_streams.end() )
      stream = m_streams[ro.get_name()][rh.get_name()];

    // If no pre-existing stream (or for some reason pre-existing stream was ~null~)
    if (stream == NULL)
    {
      uvm_tr_database* db;

      // Grab the database
      db = get_message_database();

      // If database is ~null~, use the default database
      if (db == NULL)
      {
        uvm_coreservice_t* cs = uvm_coreservice_t::get();
        db = cs->get_default_tr_database();
      }

      if (db != NULL)
      {
        // Open the stream.  Name=report object name, scope=report handler name, type=MESSAGES
        stream = db->open_stream(ro.get_name(), rh.get_name(), "MESSAGES");
        // Save off the openned stream
        m_streams[ro.get_name()][rh.get_name()] = stream;
      }
    }

    if (stream != NULL)
    {
      uvm_recorder* recorder = stream->open_recorder(report_message->get_name(),,report_message->get_type_name());
      if (recorder != NULL)
      {
        report_message->record(recorder);
        recorder->free();
      }
    }
  }
  */

  // DISPLAY action (to stdout)
  if(report_message->get_action() & UVM_DISPLAY)
    std::cout << composed_message << std::endl;

  // LOG action (to file)
  if(report_message->get_action() & UVM_LOG)
  {
    UVM_FILE tmp_file = report_message->get_file();
    f_display(tmp_file, composed_message);
  }

  // Process the UVM_COUNT action
  if(report_message->get_action() & UVM_COUNT)
  {
    if(get_max_quit_count() != 0)
    {
      incr_quit_count();
      // If quit count is reached, add the UVM_EXIT action.
      if(is_quit_count_reached())
      {
        report_message->set_action(report_message->get_action() | UVM_EXIT);
      }
    }
  }

  // Process the UVM_EXIT action
  if(report_message->get_action() & UVM_EXIT)
  {
    uvm_root* l_root;
    uvm_coreservice_t* cs;
    cs = uvm_coreservice_t::get();
    l_root = cs->get_root();
    l_root->die();
  }

  // Process the UVM_STOP action
  if (report_message->get_action() & UVM_STOP)
    sc_core::sc_stop(); // TODO different stop mechanism?
}

//----------------------------------------------------------------------------
// member function: compose_report_message (virtual)
//
// Constructs the actual string sent to the file or command line
// from the severity, component name, report id, and the message itself.
//
// Expert users can overload this method to customize report formatting.
//----------------------------------------------------------------------------

std::string uvm_default_report_server::compose_report_message(
    uvm_report_message* report_message,
    const std::string& report_object_name ) const
{
  std::string sev_string;
  std::string rep_name;

  std::string filename_line_string;
  std::string time_str;
  std::string context_str;
  std::string verbosity_str;
  std::string terminator_str;
  std::string msg_body_str;

  uvm_report_message_element_container* el_container;
  std::string prefix;
  uvm_report_handler* l_report_handler;

  sev_string = uvm_severity_name[report_message->get_severity()];

  if (!report_message->get_filename().empty())
  {
    std::ostringstream str;
    str << report_message->get_filename()
        << "("
        << report_message->get_line()
        << ") ";
    filename_line_string = str.str();
  }

  // Make definable in terms of units.

  std::ostringstream time_ostr;
  time_ostr << sc_core::sc_time_stamp();
  time_str = time_ostr.str();

  if (!report_message->get_context().empty())
    context_str = "@@" + report_message->get_context();

  if (show_verbosity)
  {
    int verb = report_message->get_verbosity();
      if ( (verb == UVM_NONE) ||
           (verb == UVM_LOW) ||
           (verb == UVM_MEDIUM) ||
           (verb == UVM_HIGH) ||
           (verb == UVM_FULL) ||
           (verb == UVM_DEBUG) )
      verbosity_str = uvm_verbosity_name[verb/100];
    else
    {
      std::ostringstream str;
      str << "(" << report_message->get_verbosity() << ")";
      verbosity_str = str.str();
    }
  }

  if (show_terminator)
    terminator_str = " -" + sev_string;

  el_container = report_message->get_element_container();
  if (el_container->size() == 0)
    msg_body_str = report_message->get_message();
  else
  {
    prefix = uvm_default_printer->knobs.prefix;
    uvm_default_printer->knobs.prefix = " +";
    msg_body_str = report_message->get_message() + "\n" + el_container->sprint();
    uvm_default_printer->knobs.prefix = prefix;
  }

  if (report_object_name.empty())
  {
    l_report_handler = report_message->get_report_handler();
    rep_name = l_report_handler->get_full_name();
  }
  else
    rep_name = report_object_name;

  std::string message = sev_string + verbosity_str + " " + filename_line_string + "@ " +
      time_str + ": " + rep_name + context_str +
      " [" + report_message->get_id() + "] " + msg_body_str + terminator_str;

  return message;
}

//----------------------------------------------------------------------------
// member function: report_summarize (virtual)
//
//----------------------------------------------------------------------------

void uvm_default_report_server::report_summarize( UVM_FILE file ) const
{
  std::string id;
  std::string name;
  std::string output_str;
  std::vector<std::string> q;
  std::string space= "                                     ";

  uvm_report_catcher::summarize();

  q.push_back("\n--- UVM Report Summary ---\n\n");

  if(m_max_quit_count != 0)
  {
    if ( m_quit_count >= m_max_quit_count )
      q.push_back("Quit count reached!\n");

    std::ostringstream output_str;
    output_str << "Quit count : "
               << std::setw(5)
               << m_quit_count
               << " of "
               << std::setw(5)
               << m_max_quit_count
               << "\n";
    q.push_back(output_str.str());
  }

  q.push_back("** Report counts by severity\n");

  for( severity_count_citt it = m_severity_count.begin();
       it != m_severity_count.end(); ++it)
  {
    std::ostringstream output_str;
    output_str << uvm_severity_name[it->first]
               << space.substr(1, 13-std::string(uvm_severity_name[it->first]).length())
               << " :"
               << std::setw(4)
               << it->second // count
               << "\n";
    q.push_back(output_str.str());
  }

  if (enable_report_id_count_summary)
  {
    q.push_back("** Report counts by id\n");
    for( id_count_citt it = m_id_count.begin();
         it != m_id_count.end();
         it++)
    {
      std::ostringstream output_str;
      output_str << "["
                 << it->first // id (string)
                 << "] "
                 << space.substr(1, 20-(it->first).length())
                 << std::setw(2)
                 << it->second // id_count[id] (int);
                 << "\n";
      q.push_back(output_str.str());
    }
  }

  UVM_INFO("UVM/REPORT/SERVER", UVM_STRING_QUEUE_STREAMING_PACK(q), UVM_LOW);
}


//----------------------------------------------------------------------------
// member function: get_type_name
//
// Internal member function. Needed for callbacks
//----------------------------------------------------------------------------

const std::string uvm_default_report_server::get_type_name() const
{
  return "uvm_default_report_server";
}


} // namespace uvm
