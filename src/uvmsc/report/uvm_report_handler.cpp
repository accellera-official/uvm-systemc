//----------------------------------------------------------------------------
//   Copyright 2013-2017 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc. 
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2013 NVIDIA Corporation
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

#include <sstream>

#include "uvmsc/print/uvm_printer.h"
#include "uvmsc/report/uvm_report_handler.h"
#include "uvmsc/report/uvm_report_server.h"
#include "uvmsc/report/uvm_report_message.h"
#include "uvmsc/base/uvm_version.h"
#include "uvmsc/macros/uvm_string_defines.h"
#include "uvmsc/macros/uvm_message_defines.h"
#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"
#include "uvmsc/base/uvm_globals.h"
#include "uvmsc/misc/uvm_misc.h"




namespace uvm {


//----------------------------------------------------------------------------
// Constructor
//
//! Creates and initializes a new #uvm_report_handler object.
//----------------------------------------------------------------------------

uvm_report_handler::uvm_report_handler( const std::string name )
: uvm_object(name)
{
  initialize();
}


//----------------------------------------------------------------------------
// member function: do_print (virtual)
//
// The uvm_report_handler implements the <uvm_object::do_print()> such that
// ~print~ method provides UVM printer formatted output
// of the current configuration. A snippet of example output is shown here:
//----------------------------------------------------------------------------

void uvm_report_handler::do_print( const uvm_printer& printer ) const
{
  uvm_severity l_severity;
  std::string idx;
  int l_int;

  // print max verbosity
  if ( (m_max_verbosity_level == UVM_NONE) ||
       (m_max_verbosity_level == UVM_LOW) ||
       (m_max_verbosity_level == UVM_MEDIUM) ||
       (m_max_verbosity_level == UVM_HIGH) ||
       (m_max_verbosity_level == UVM_FULL) ||
       (m_max_verbosity_level == UVM_DEBUG) )
    printer.print_generic("max_verbosity_level", "uvm_verbosity", 32,
        uvm_verbosity_name[m_max_verbosity_level/100]);
  else
    printer.print_field_int("max_verbosity_level", m_max_verbosity_level, 32, UVM_DEC,
        ".", "int");

  // print id verbosity
  if(id_verbosities.size() != 0)
  {
    printer.print_array_header("id_verbosities", (int)id_verbosities.size(), "uvm_pool");
    for (id_verbosities_mapcitt it = id_verbosities.begin(); it != id_verbosities.end(); ++it)
    {
      l_int = it->second;
      idx = it->first;
      if ( (l_int == UVM_NONE) ||
           (l_int == UVM_LOW) ||
           (l_int == UVM_MEDIUM) ||
           (l_int == UVM_HIGH) ||
           (l_int == UVM_FULL) ||
           (l_int == UVM_DEBUG) )
        printer.print_generic("[" + idx + "]", "uvm_verbosity", 32, uvm_verbosity_name[l_int/100]);
      else
      {
        std::ostringstream l_str;
        l_str << l_int;
        printer.print_generic("[" + idx + "]", "int", 32, l_str.str());
      }
    }
    printer.print_array_footer();
  }

  // print sev and id verbosities
  if(severity_id_verbosities.size() != 0)
  {
    int _total_cnt = 0;
    for( severity_id_verbosities_mapcitt it = severity_id_verbosities.begin();
         it != severity_id_verbosities.end(); ++it) // iterate over full map
    {
      l_severity = it->first;
      _total_cnt += (int)severity_id_verbosities.find(l_severity)->second.size();
    }

    printer.print_array_header("severity_id_verbosities", _total_cnt, "array");

    for( severity_id_verbosities_mapcitt it = severity_id_verbosities.begin();
        it != severity_id_verbosities.end(); ++it )
    {
      l_severity = it->first;
      uvm_id_verbosities_array id_v_ary = severity_id_verbosities.find(l_severity)->second;

      for( id_verbosities_mapcitt it2 = id_v_ary.begin();
          it2 != id_v_ary.end(); ++it2 )
      {
        l_int = it2->second;
        idx = it2->first;
        std::ostringstream str;
        str << "[" << uvm_severity_name[l_severity] << ":" << idx << "]";

        if ( (l_int == UVM_NONE) ||
            (l_int == UVM_LOW) ||
            (l_int == UVM_MEDIUM) ||
            (l_int == UVM_HIGH) ||
            (l_int == UVM_FULL) ||
            (l_int == UVM_DEBUG) )
        {
          printer.print_generic(str.str(), "uvm_verbosity", 32, uvm_verbosity_name[l_int/100]);
        }
        else
        {
          std::ostringstream l_str;
          l_str << l_int;
          printer.print_generic(l_str.str(), "int", 32, l_str.str());
        }
      } // for it2
    } // for it

    printer.print_array_footer();
  }

  // print id actions
  if(id_actions.size() != 0)
  {
    printer.print_array_header("id_actions", (int)id_actions.size(), "uvm_pool");

    for( id_actions_mapcitt it = id_actions.begin();
         it != id_actions.end(); ++it )
    {
      l_int = it->second;
      idx = it->first;
      printer.print_generic("[" + idx + "]", "uvm_action", 32, format_action(l_int));
    }

    printer.print_array_footer();
  }

  // print severity actions
  if(severity_actions.size() != 0)
  {
    printer.print_array_header("severity_actions", 4, "array");

    for( severity_actions_mapcitt it = severity_actions.begin();
         it != severity_actions.end(); ++it)
    {
      l_severity = it->first;
      std::ostringstream str;
      str << "[" << uvm_severity_name[l_severity] << "]";
      printer.print_generic(str.str() , "uvm_action", 32,
          format_action(severity_actions.find(l_severity)->second));
    }

    printer.print_array_footer();
  }

  // print sev and id actions
  if(severity_id_actions.size() != 0)
  {
    int _total_cnt = 0;

    for( severity_id_actions_mapcitt it = severity_id_actions.begin();
         it != severity_id_actions.end(); ++it)
    {
      l_severity = it->first;
      _total_cnt += (int)severity_id_actions.find(l_severity)->second.size();
    }

    printer.print_array_header("severity_id_actions", _total_cnt, "array");

    for( severity_id_actions_mapcitt it = severity_id_actions.begin();
        it != severity_id_actions.end(); ++it)
    {
      l_severity = it->first;
      uvm_id_actions_array id_a_ary = severity_id_actions.find(l_severity)->second;

      for( id_actions_mapcitt it2 = id_a_ary.begin();
          it2 != id_a_ary.end(); ++it2)
      {
        idx = it2->first;
        uvm_action action = it2->second;
        std::ostringstream str;
        str << "[" << uvm_severity_name[l_severity] << ":" << idx << "]";
        printer.print_generic(str.str(), "uvm_action", 32, format_action(action));
      }
    }

    printer.print_array_footer();
  }

  // print sev overrides
  if(sev_overrides.size() != 0 )
  {
    printer.print_array_header("sev_overrides", (int)sev_overrides.size(), "uvm_pool");

    for( sev_overrides_mapcitt it = sev_overrides.begin();
      it != sev_overrides.end(); ++it)
    {
      uvm_severity l_severity_first = it->first;
      uvm_severity l_severity_second = it->second;
      std::ostringstream str;
      str << "[" << uvm_severity_name[l_severity_first] << "]";

      printer.print_generic(str.str(),
          "uvm_severity", 32, uvm_severity_name[l_severity_second]);
    }

    printer.print_array_footer();
  }

  // print sev and id overrides
  if(sev_id_overrides.size() != 0)
  {
    int _total_cnt = 0;
    for( sev_id_overrides_mapcitt it = sev_id_overrides.begin();
        it != sev_id_overrides.end(); ++it )
    {
      idx = it->first;
      _total_cnt += (int)sev_id_overrides.find(idx)->second.size();
    }

    printer.print_array_header("sev_id_overrides", _total_cnt, "array");

    for( sev_id_overrides_mapcitt it = sev_id_overrides.begin();
        it != sev_id_overrides.end(); ++it)
    {
      idx = it->first;
      uvm_sev_override_array sev_o_ary = sev_id_overrides.find(idx)->second;

      for( sev_overrides_mapcitt it2 = sev_o_ary.begin();
          it2 != sev_o_ary.end(); ++it2)
      {
        uvm_severity sev_first = it2->first;
        uvm_severity sev_second = it2->second;
        std::ostringstream str;
        str << "[" << uvm_severity_name[sev_first]
            << ":" << idx << "]";
        printer.print_generic(str.str(),
            "uvm_severity", 32, uvm_severity_name[sev_second]);
      }
    }
    printer.print_array_footer();
  }

  // print default file handle
  int def_file_handle = 0x1; // TODO no file handle when using ostream. Orig: default_file_handle;
  printer.print_field_int("default_file_handle", def_file_handle, 32, UVM_HEX,
      ".", "int");

  // print id files
  if(id_file_handles.size()!=0)
  {
    printer.print_array_header("id_file_handles", (int)id_file_handles.size(),
        "uvm_pool");

    for( id_file_handles_mapcitt it = id_file_handles.begin();
        it != id_file_handles.end(); ++it)
    {
      idx = it->first;
      int file_handle = 0x1; // TODO no file handle when using ostream. Orig: id_file_handles[idx];
      printer.print_field_int("[" + idx + "]", file_handle, 32,
          UVM_HEX, ".", "UVM_FILE");
    }

    printer.print_array_footer();
  }

  // print severity files
  if(severity_file_handles.size() != 0)
  {
    printer.print_array_header("severity_file_handles", 4, "array");

    for( severity_file_handles_mapcitt it = severity_file_handles.begin();
        it != severity_file_handles.end(); ++it)
    {
      l_severity = it->first;
      std::ostringstream str;
      str << "[" << uvm_severity_name[l_severity] << "]";
      int file_handle = 0x1; // TODO no file handle when using ostream. Orig: severity_file_handles[l_severity];
      printer.print_field_int(str.str(), file_handle, 32, UVM_HEX, ".", "UVM_FILE");
    }

    printer.print_array_footer();
  }

  // print sev and id files
  if(severity_id_file_handles.size() != 0)
  {
    int _total_cnt = 0;
    for( severity_id_file_handles_mapcitt it = severity_id_file_handles.begin();
        it != severity_id_file_handles.end(); ++it)
    {
      l_severity = it->first;
      _total_cnt += (int)severity_id_file_handles.find(l_severity)->second.size();
    }
    printer.print_array_header("severity_id_file_handles", _total_cnt, "array");


    for( severity_id_file_handles_mapcitt it = severity_id_file_handles.begin();
        it != severity_id_file_handles.end(); ++it)
    {
      l_severity = it->first;
      uvm_id_file_array id_f_ary = severity_id_file_handles.find(l_severity)->second;

      for( id_file_handles_mapcitt it2 = id_f_ary.begin();
          it2 != id_f_ary.end(); ++it2)
      {
        idx = it2->first;
        std::ostringstream str;
        str << "[" << uvm_severity_name[l_severity] << ":" << idx << "]";
        int file_handle = 0x1; // TODO no file handle when using ostream. Orig: id_f_ary[idx];
        printer.print_field_int(str.str(),
            file_handle, 32, UVM_HEX, ".", "UVM_FILE"); // check cast UVM_FILE
      }
    }
    printer.print_array_footer();
  }
}

//----------------------------------------------------------------------------
// Group: Message Processing
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// member function: process_report_message
//
// This is the common handler method used by the four core reporting methods
// (e.g. <uvm_report_error>) in <uvm_report_object>.
//----------------------------------------------------------------------------

void uvm_report_handler::process_report_message(uvm_report_message* report_message)
{
  auto srvr = uvm_report_server::get_server();

  std::string id = report_message->get_id();
  uvm_severity severity = report_message->get_severity();

  // Check for severity overrides and apply them before calling the server.
  // An id specific override has precedence over a generic severity override.

  if(sev_id_overrides.find(id) != sev_id_overrides.end()) //exists
  {
    if(sev_id_overrides[id].find(severity) != sev_id_overrides[id].end()) //exists
    {
      severity = sev_id_overrides[id][severity];
      report_message->set_severity(severity);
    }
  }
  else
  {
    if(sev_overrides.find(severity) != sev_overrides.end()) //exists
    {
      severity = sev_overrides[severity];
      report_message->set_severity(severity);
    }
  }
  report_message->set_file(get_file_handle(severity, id));
  report_message->set_report_handler(this);
  report_message->set_action(get_action(severity, id));
  srvr->process_report_message(report_message);
}

//----------------------------------------------------------------------------
// member function: get_verbosity_level
//
//! Returns the verbosity associated with the given \p severity and \p id.
//!
//! First, if there is a verbosity associated with the \p severity, \p id pair,
//! return that. Else, if there is an verbosity associated with the \p id, return
//! that. Else, return the max verbosity setting.
//----------------------------------------------------------------------------

int uvm_report_handler::get_verbosity_level( uvm_severity severity,
                                             const std::string& id )
{
  uvm_id_verbosities_array array;
  std::string loc_id = id;

  if( severity_id_verbosities.find(severity) != severity_id_verbosities.end() ) //exists
  {
    array = severity_id_verbosities[severity];
    if(array.find(loc_id) != array.end())
      return array[loc_id];
  }

  if(id_verbosities.find(loc_id) != id_verbosities.end())
    return id_verbosities[loc_id];

  return m_max_verbosity_level;
}

//----------------------------------------------------------------------------
// member function: get_action
//
//! Returns the action associated with the given \p severity and \p id.
//!
//! First, if there is an action associated with the \p (severity,id) pair,
//! return that. Else, if there is an action associated with the \p id, return
//! that. Else, if there is an action associated with the \p severity, return
//! that. Else, return the default action associated with the \p severity.
//----------------------------------------------------------------------------

uvm_action uvm_report_handler::get_action( uvm_severity severity,
                                           const std::string& id )
{
  uvm_id_actions_array array;
  std::string loc_id = id;

  if(severity_id_actions.find(severity) != severity_id_actions.end())
  {
    array = severity_id_actions[severity];
    if(array.find(loc_id) != array.end())
      return array[loc_id];
  }

  if(id_actions.find(loc_id) != id_actions.end())
    return id_actions[loc_id];

  return severity_actions[severity];
}

//----------------------------------------------------------------------------
// member function: get_file_handle
//
//! Returns the file descriptor associated with the given \p severity and id.
//!
//! First, if there is a file handle associated with the \p (severity,id) pair,
//! return that. Else, if there is a file handle associated with the \p id , return
//! that. Else, if there is an file handle associated with the \p severity, return
//! that. Else, return the default file handle.
//----------------------------------------------------------------------------

UVM_FILE uvm_report_handler::get_file_handle( uvm_severity severity,
                                              const std::string& id )
{
  UVM_FILE file;
  std::string loc_id = id;

  file = get_severity_id_file(severity, id);
  if (file != 0)
    return file;

  if (id_file_handles.find(loc_id) != id_file_handles.end())
  {
    file = id_file_handles[loc_id];
    if (file != 0)
      return file;
  }

  if (severity_file_handles.find(severity) != severity_file_handles.end()) //exists
  {
    file = severity_file_handles[severity];
    if(file != 0)
      return file;
  }

  return default_file_handle;
}

//----------------------------------------------------------------------------
// member function: report
//
//! This is the common handler method used by the four core reporting methods
//! (e.g., #uvm_report_error) in #uvm_report_object.
//----------------------------------------------------------------------------

void uvm_report_handler::report( uvm_severity severity,
                                 const std::string& name,
                                 const std::string& id,
                                 const std::string& message,
                                 int verbosity_level,
                                 const std::string& filename,
                                 int line,
                                 uvm_report_object* client )
{
  uvm_report_message* l_report_message;

  uvm_coreservice_t* cs;
  cs = uvm_coreservice_t::get();

  if (!uvm_report_enabled(verbosity_level, UVM_INFO, id))
    return;

  if (client == nullptr)
    client = cs->get_root();

  l_report_message = uvm_report_message::new_report_message();
  l_report_message->set_report_message(severity, id, message,
                                      verbosity_level, filename, line, name);
  l_report_message->set_report_object(client);
  l_report_message->set_action(get_action(severity,id));
  process_report_message(l_report_message);
}

//----------------------------------------------------------------------------
// member function: format_action
//
//! Returns a string representation of the \p action, e.g., "DISPLAY".
//----------------------------------------------------------------------------

std::string uvm_report_handler::format_action( uvm_action action ) const
{
  std::string s;

  if(uvm_action_type(action) == UVM_NO_ACTION)
    s = "NO ACTION";
  else
  {
    s.clear(); s = "";
    if(action & UVM_DISPLAY)   s = s + "DISPLAY ";
    if(action & UVM_LOG)       s = s + "LOG ";
    if(action & UVM_RM_RECORD) s = s + "RM_RECORD ";
    if(action & UVM_COUNT)     s = s + "COUNT ";
    if(action & UVM_CALL_HOOK) s = s + "CALL_HOOK ";
    if(action & UVM_EXIT)      s = s + "EXIT ";
    if(action & UVM_STOP)      s = s + "STOP ";
  }

  return s;
}



////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------------

uvm_report_handler::~uvm_report_handler()
{
}


//----------------------------------------------------------------------------
// member function: m_initialize
//
//! This method is called by the constructor to initialize the arrays and
//! other variables described above to their default values.
//----------------------------------------------------------------------------

void uvm_report_handler::initialize()
{
  set_default_file(0);
  m_max_verbosity_level = UVM_MEDIUM;

  set_severity_action(UVM_INFO,    UVM_DISPLAY);
  set_severity_action(UVM_WARNING, UVM_DISPLAY);
  set_severity_action(UVM_ERROR,   UVM_DISPLAY | UVM_COUNT);
  set_severity_action(UVM_FATAL,   UVM_DISPLAY | UVM_EXIT);

  set_severity_file(UVM_INFO, default_file_handle);
  set_severity_file(UVM_WARNING, default_file_handle);
  set_severity_file(UVM_ERROR,   default_file_handle);
  set_severity_file(UVM_FATAL,   default_file_handle);
}

//----------------------------------------------------------------------------
// member function: get_severity_id_file
//
//! Return the file id based on the \p severity and the \p id.
//----------------------------------------------------------------------------

UVM_FILE uvm_report_handler::get_severity_id_file( uvm_severity severity,
                                                   const std::string& id )
{
  uvm_id_file_array array;
  std::string loc_id = id;

  if(severity_id_file_handles.find(severity) != severity_id_file_handles.end() ) // exists
  {
    array = severity_id_file_handles[severity];
    if(array.find(loc_id) != array.end())
      return array[loc_id];
  }

  if(id_file_handles.find(loc_id) != id_file_handles.end())
    return id_file_handles[loc_id];

  if(severity_file_handles.find(severity) != severity_file_handles.end()) // exists
    return severity_file_handles[severity];

  return default_file_handle;
}


//----------------------------------------------------------------------------
// member function: set_verbosity_level
//
//! Internal method called by uvm_report_object.
//----------------------------------------------------------------------------

void uvm_report_handler::set_verbosity_level(int verbosity_level)
{
  m_max_verbosity_level = verbosity_level;
}


//----------------------------------------------------------------------------
// member function: set_severity_action
//
//! Internal method called by #uvm_report_object.
//----------------------------------------------------------------------------

void uvm_report_handler::set_severity_action( uvm_severity severity,
                          uvm_action action )
{
  severity_actions[severity] = action;
}

//----------------------------------------------------------------------------
// member function: set_id_action
//
//! Internal method called by #uvm_report_object.
//----------------------------------------------------------------------------

void uvm_report_handler::set_id_action( const std::string& id,
                                        uvm_action action )
{
  id_actions[id] = action;
}

//----------------------------------------------------------------------------
// member function: set_severity_id_action
//
//! Internal method called by #uvm_report_object.
//----------------------------------------------------------------------------

void uvm_report_handler::set_severity_id_action( uvm_severity severity,
                                                 const std::string& id,
                                                 uvm_action action )
{
  //if(severity_id_actions.find(severity) == severity_id_actions.end() ) // not exists
  //  severity_id_actions[severity] = new;

  // TODO check - no need to new item?
  severity_id_actions[severity][id] = action;
}

//----------------------------------------------------------------------------
// member function: set_id_verbosity
//
//! Internal method called by #uvm_report_object.
//----------------------------------------------------------------------------

void uvm_report_handler::set_id_verbosity( const std::string& id,
                                           int verbosity )
{
  id_verbosities[id] = verbosity;
}

//----------------------------------------------------------------------------
// member function: set_severity_id_verbosity
//
//! Internal method called by #uvm_report_object.
//----------------------------------------------------------------------------

void uvm_report_handler::set_severity_id_verbosity( uvm_severity severity,
                                                    const std::string& id,
                                                    int verbosity )
{
  //if(!severity_id_verbosities.exists(severity))
  //  severity_id_verbosities[severity] = new;

  // TODO check - no need to new item?
  severity_id_verbosities[severity][id] = verbosity;
}

//----------------------------------------------------------------------------
// member function: set_default_file
//
//! Internal methods called by #uvm_report_object.
//----------------------------------------------------------------------------

void uvm_report_handler::set_default_file( UVM_FILE file )
{
  default_file_handle = file;
}

//----------------------------------------------------------------------------
// member function: set_severity_file
//
//! Internal methods called by #uvm_report_object.
//----------------------------------------------------------------------------

void uvm_report_handler::set_severity_file( uvm_severity severity,
                                            UVM_FILE file )
{
  severity_file_handles[severity] = file;
}

//----------------------------------------------------------------------------
// member function: set_id_file
//
//! Internal methods called by #uvm_report_object.
//----------------------------------------------------------------------------

void uvm_report_handler::set_id_file( const std::string& id,
                                      UVM_FILE file )
{
  id_file_handles[id] = file;
}

//----------------------------------------------------------------------------
// member function: set_severity_id_file
//
//! Internal methods called by #uvm_report_object.
//----------------------------------------------------------------------------

void uvm_report_handler::set_severity_id_file( uvm_severity severity,
                                               const std::string& id,
                                               UVM_FILE file)
{
//  if(severity_id_file_handles.find(severity) == severity_id_file_handles.end()) // not exists
//    severity_id_file_handles[severity] = new;

  // TODO check - no need to new item?
  severity_id_file_handles[severity][id] = file;
}

//----------------------------------------------------------------------------
// member function: set_severity_override
//
//! Internal methods called by #uvm_report_object.
//----------------------------------------------------------------------------

void uvm_report_handler::set_severity_override( uvm_severity cur_severity,
                                                uvm_severity new_severity)
{
  sev_overrides[cur_severity] = new_severity;
}

//----------------------------------------------------------------------------
// member function: set_severity_id_override
//
//! Internal methods called by #uvm_report_object.
//----------------------------------------------------------------------------

void uvm_report_handler::set_severity_id_override( uvm_severity cur_severity,
                                                   const std::string& id,
                                                   uvm_severity new_severity)
{
  // has precedence over set_severity_override
  // silently override previous setting
  sev_id_overrides[id][cur_severity] = new_severity;
}


//----------------------------------------------------------------------------
// member function: dump_state
//
//! Internal method for debug.
//! NOTE: DEPRECATED, will be removed soon
//----------------------------------------------------------------------------

void uvm_report_handler::dump_state()
{
  std::string s;
  UVM_FILE file;
  std::string idx;

  std::vector<std::string> q;

  uvm_id_actions_array id_a_ary;
  uvm_id_verbosities_array id_v_ary;
  uvm_id_file_array id_f_ary;

  q.push_back("\n----------------------------------------------------------------------\n");
  q.push_back("report handler state dump \n\n");

  // verbosities

  q.push_back("\n+-----------------+\n");
  q.push_back("|   Verbosities   |\n");
  q.push_back("+-----------------+\n\n");

  std::ostringstream str;
  str << "max verbosity level = "
      << m_max_verbosity_level
      << "\n";
  q.push_back(str.str());

  q.push_back("*** verbosities by id\n");

  for( std::map<std::string, int>::iterator it = id_verbosities.begin();
       it != id_verbosities.end(); ++it)
  {
    uvm_verbosity v = (uvm_verbosity) it->second;
    std::ostringstream s;
    s << "["
      << it->first
      << "] --> "
      << uvm_verbosity_name[v/100]
      << "\n";
    q.push_back(s.str());
  }

  // verbosities by id

  q.push_back("*** verbosities by id and severity\n");

  for( severity_id_verbosities_mapcitt it = severity_id_verbosities.begin();
       it != severity_id_verbosities.end();
       it++)
  {
    uvm_severity sev = it->first; //severity;
    id_v_ary = it->second; // severity_id_verbosities[severity];
    for( std::map<std::string, int>::iterator idit = id_v_ary.begin();
         idit != id_v_ary.end(); ++it)
    {
      uvm_verbosity v = (uvm_verbosity) idit->second;
      std::ostringstream s;
      s << uvm_severity_name[sev]
        << ":"
        << idit->first
        << " --> "
        << uvm_verbosity_name[v/100]
        << "\n";
      q.push_back(s.str());
    }
  }

  // actions

  q.push_back("\n+-------------+\n");
  q.push_back("|   actions   |\n");
  q.push_back("+-------------+\n\n");

  q.push_back("*** actions by severity\n");

  for ( severity_actions_mapcitt it = severity_actions.begin();
        it != severity_actions.end();
        it++)
  {
    uvm_severity sev = it->first; // = severity
    std::ostringstream s;
    s << uvm_severity_name[sev]
      << " = "
      << format_action(it->second) // = severity_actions[severity]
      << "\n";
    q.push_back(s.str());
  }

  q.push_back("\n*** actions by id\n");

  for( std::map<std::string, uvm_action>::iterator it = id_actions.begin();
       it != id_actions.end(); ++it )
  {
    std::ostringstream s;
    s << "["
      << it->first
      << "] --> "
      << format_action(it->second)
      << "\n";
    q.push_back(s.str());
  }

  // actions by id

  q.push_back("\n*** actions by id and severity\n");

  for ( severity_id_verbosities_mapcitt it = severity_id_actions.begin();
        it != severity_id_actions.end();
        it++)
  {
      uvm_severity sev = it->first; // = severity;
      id_a_ary = it->second; // = severity_id_actions[severity];
      for( std::map<std::string, uvm_action>::iterator ait = id_a_ary.begin();
           ait != id_a_ary.end(); ++ait)
      {
        std::ostringstream s;
        s << uvm_severity_name[sev]
          << ":"
          << ait->first
          << " --> "
          << format_action(ait->second)
          << "\n";
        q.push_back(s.str());
      }
  }

  // Files

  q.push_back("\n+-------------+\n");
  q.push_back("|    files    |\n");
  q.push_back("+-------------+\n\n");

  str.clear();
  str << "default file handle = "
      << default_file_handle
      << "\n\n";
  q.push_back(str.str());

  q.push_back("*** files by severity\n");

  for( severity_file_handles_mapcitt it = severity_file_handles.begin();
       it != severity_file_handles.end();
       it++)
  {
    uvm_severity sev = it->first; // = severity;
    file = it->second; //severity_file_handles[severity];
    std::ostringstream s;
    s << uvm_severity_name[sev]
      << " = "
      << file
      << "\n";
    q.push_back(s.str());
  }

  q.push_back("\n*** files by id\n");

  for( std::map<std::string, UVM_FILE>::iterator it = id_file_handles.begin();
       it != id_file_handles.end(); ++it)
  {
    file = it->second;
    std::ostringstream s;
    s << "id "
      << it->first
      << " --> "
      << file
      << "\n";
    q.push_back(s.str());
  }

  q.push_back("\n*** files by id and severity\n");

  for( severity_id_file_handles_mapcitt it = severity_id_file_handles.begin();
       it != severity_id_file_handles.end();
       it++)
  {
    uvm_severity sev = it->first; // = severity;
    id_f_ary = it->second; // = severity_id_file_handles[severity];
    for (std::map<std::string, UVM_FILE>::iterator fit = id_f_ary.begin(); fit != id_f_ary.end(); ++fit) {
      std::ostringstream s;
      s << uvm_severity_name[sev]
        << ":"
        << fit->first
        << " --> "
        << fit->second
         << "\n";
      q.push_back(s.str());
    }
  }

  {
    auto srvr = uvm_report_server::get_server();
    srvr->report_summarize();
  }

  q.push_back("----------------------------------------------------------------------\n");

  UVM_INFO("UVM/REPORT/HANDLER", UVM_STRING_QUEUE_STREAMING_PACK(q), UVM_LOW);
}

} // namespace uvm
