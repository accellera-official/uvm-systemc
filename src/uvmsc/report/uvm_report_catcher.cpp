//------------------------------------------------------------------------------
//   Copyright 2013-2016 NXP B.V.
//   Copyright 2007-2010 Mentor Graphics Corporation
//   Copyright 2007-2009 Cadence Design Systems, Inc.
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
//------------------------------------------------------------------------------

#include <iostream>
#include <iomanip>

#include "uvmsc/report/uvm_report_object.h"
#include "uvmsc/report/uvm_report_message.h"
#include "uvmsc/report/uvm_report_catcher.h"
#include "uvmsc/report/uvm_report_catcher_data.h"
#include "uvmsc/report/uvm_report_handler.h"
#include "uvmsc/report/uvm_report_server.h"
#include "uvmsc/macros/uvm_callback_defines.h"
#include "uvmsc/macros/uvm_string_defines.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"

namespace uvm {

namespace {

uvm_report_cb_iter* name_iter_ref()
{
  return uvm_coreservice_t::get()->get_uvm_report_catcher_name_iter();
}

uvm_report_cb_iter* print_iter_ref()
{
  return uvm_coreservice_t::get()->get_uvm_report_catcher_print_iter();
}

bool& in_catcher_ref()
{
  return uvm_coreservice_t::get()->get_uvm_report_catcher_in_catcher();
}

} // namespace

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------

uvm_report_catcher::uvm_report_catcher( const std::string& name )
: uvm_callback(name)
{
  uvm_report_catcher_data::get().do_report = true;

  // register the catcher callback
  // this differs from UVMSV since we cannot register using the macro
  // because the static method cannot be initialize within the class declaration
  m_register_cb();
}

//------------------------------------------------------------------------------
// Group: Current Message State
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// member function: get_client
//
//! Returns the #uvm_report_object that has generated the message that
//! is currently being processes.
//------------------------------------------------------------------------------

uvm_report_object* uvm_report_catcher::get_client() const
{
  return uvm_report_catcher_data::get().m_modified_report_message->get_report_object();
}

//------------------------------------------------------------------------------
// member function: get_severity
//
//! Returns the #uvm_severity of the message that is currently being
//! processed. If the severity was modified by a previously executed
//! catcher object (which re-threw the message), then the returned
//! severity is the modified value.
//------------------------------------------------------------------------------

uvm_severity uvm_report_catcher::get_severity() const
{
  return uvm_report_catcher_data::get().m_modified_report_message->get_severity();
}

//------------------------------------------------------------------------------
// member function: get_context
//
//! Returns the context name of the message that is currently being
//! processed. This is typically the full hierarchical name of the component
//! that issued the message. However, if user-defined context is set from
//! a uvm_report_message, the user-defined context will be returned.
//------------------------------------------------------------------------------

std::string uvm_report_catcher::get_context() const
{
  std::string context_str;

  context_str = uvm_report_catcher_data::get().m_modified_report_message->get_context();
  if (context_str.empty())
  {
    uvm_report_handler* rh = uvm_report_catcher_data::get().m_modified_report_message->get_report_handler();
    context_str = rh->get_full_name();
  }

  return context_str;
}

//------------------------------------------------------------------------------
// member function: get_verbosity
//
//! Returns the verbosity of the message that is currently being
//! processed. If the verbosity was modified by a previously executed
//! catcher (which re-threw the message), then the returned
//! verbosity is the modified value.
//------------------------------------------------------------------------------

int uvm_report_catcher::get_verbosity() const
{
  return uvm_report_catcher_data::get().m_modified_report_message->get_verbosity();
}

//------------------------------------------------------------------------------
// member function: get_id
//
//! Returns the string id of the message that is currently being
//! processed. If the id was modified by a previously executed
//! catcher (which re-threw the message), then the returned
//! id is the modified value.
//------------------------------------------------------------------------------

std::string uvm_report_catcher::get_id() const
{
  return uvm_report_catcher_data::get().m_modified_report_message->get_id();
}

//------------------------------------------------------------------------------
// member function: get_message
//
//! Returns the string message of the message that is currently being
//! processed. If the message was modified by a previously executed
//! catcher (which re-threw the message), then the returned
//! message is the modified value.
//------------------------------------------------------------------------------

std::string uvm_report_catcher::get_message() const
{
  return uvm_report_catcher_data::get().m_modified_report_message->get_message();
}

//------------------------------------------------------------------------------
// member function: get_action
//
// Returns the <uvm_action> of the message that is currently being
// processed. If the action was modified by a previously executed
// catcher (which re-threw the message), then the returned
// action is the modified value.
//------------------------------------------------------------------------------

uvm_action uvm_report_catcher::get_action() const
{
  return uvm_report_catcher_data::get().m_modified_report_message->get_action();
}

//------------------------------------------------------------------------------
// member function: get_fname
//
//! Returns the file name of the message.
//------------------------------------------------------------------------------

std::string uvm_report_catcher::get_fname() const
{
  return uvm_report_catcher_data::get().m_modified_report_message->get_filename();
}

//------------------------------------------------------------------------------
// member function: get_line
//
//! Returns the line number of the message.
//------------------------------------------------------------------------------

int uvm_report_catcher::get_line() const
{
  return uvm_report_catcher_data::get().m_modified_report_message->get_line();
}

//------------------------------------------------------------------------------
// member function: get_element_container
//
//! Returns the element container of the message.
//------------------------------------------------------------------------------

uvm_report_message_element_container* uvm_report_catcher::get_element_container() const
{
  return uvm_report_catcher_data::get().m_modified_report_message->get_element_container();
}

//------------------------------------------------------------------------------
// Group: Change Message State
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// member function: set_severity
//
//! Change the severity of the message to \p severity. Any other
//! report catchers will see the modified value.
//------------------------------------------------------------------------------

void uvm_report_catcher::set_severity( uvm_severity severity )
{
  uvm_report_catcher_data::get().m_modified_report_message->set_severity(severity);
}

//------------------------------------------------------------------------------
// member function: set_verbosity
//
//! Change the verbosity of the message to \p verbosity. Any other
//! report catchers will see the modified value.
//------------------------------------------------------------------------------

void uvm_report_catcher::set_verbosity( int verbosity )
{
  uvm_report_catcher_data::get().m_modified_report_message->set_verbosity(verbosity);
}

//------------------------------------------------------------------------------
// member function: set_id
//
//! Change the id of the message to \p id. Any other
//! report catchers will see the modified value.
//------------------------------------------------------------------------------

void uvm_report_catcher::set_id( const std::string& id )
{
  uvm_report_catcher_data::get().m_modified_report_message->set_id(id);
}

//------------------------------------------------------------------------------
// member function: set_message
//
//! Change the text of the message to \p message. Any other
//! report catchers will see the modified value.
//------------------------------------------------------------------------------

void uvm_report_catcher::set_message( const std::string& message )
{
  uvm_report_catcher_data::get().m_modified_report_message->set_message(message);
}

//------------------------------------------------------------------------------
// member function: set_action
//
//! Change the action of the message to \p action. Any other
//! report catchers will see the modified value.
//------------------------------------------------------------------------------

void uvm_report_catcher::set_action( uvm_action action )
{
  uvm_report_catcher_data::get().m_modified_report_message->set_action(action);
}

//------------------------------------------------------------------------------
// member function: set_context
//
//! Change the context of the message to ~context_str~. Any other
//! report catchers will see the modified value.
//------------------------------------------------------------------------------

void uvm_report_catcher::set_context( const std::string& context_str )
{
  uvm_report_catcher_data::get().m_modified_report_message->set_context(context_str);
}

//------------------------------------------------------------------------------
// member function: add_int
//
//! Add an integral type of the name ~name~ and value ~value~ to
//! the message.  The required ~size~ field indicates the size of ~value~.
//! The required ~radix~ field determines how to display and
//! record the field. Any other report catchers will see the newly
//! added element.
//------------------------------------------------------------------------------

void uvm_report_catcher::add_int( const std::string& name,
                                  uvm_bitstream_t value,
                                  int size,
                                  uvm_radix_enum radix,
                                  uvm_action action )
{
  uvm_report_catcher_data::get().m_modified_report_message->add_int(name, value, size, radix, action);
}


//------------------------------------------------------------------------------
// member function: add_string
//
//! Adds a string of the name ~name~ and value ~value~ to the
//! message. Any other report catchers will see the newly
//! added element.
//------------------------------------------------------------------------------

void uvm_report_catcher::add_string( const std::string& name,
                                     const std::string& value,
                                     uvm_action action )
{
  uvm_report_catcher_data::get().m_modified_report_message->add_string(name, value, action);
}

//------------------------------------------------------------------------------
// member function: add_object
//
//! Adds a uvm_object of the name ~name~ and reference ~obj~ to
//! the message. Any other report catchers will see the newly
//! added element.
//------------------------------------------------------------------------------

void uvm_report_catcher::add_object( const std::string& name,
                                     uvm_object* obj,
                                     uvm_action action )
{
  uvm_report_catcher_data::get().m_modified_report_message->add_object(name, obj, action);
}


//------------------------------------------------------------------------------
// Group: Debug
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// member function: get_report_catcher (static)
//
//! Returns the first report catcher that has \p name.
//------------------------------------------------------------------------------

uvm_report_catcher* uvm_report_catcher::get_report_catcher( const std::string& name )
{
  // Refactored from the former function-local iter global to uvm_coreservice_t.
  uvm_report_cb_iter* iter = name_iter_ref();

  uvm_report_catcher* report_catcher = iter->first();

  while(report_catcher != nullptr)
  {
    if(report_catcher->get_name() == name)
      return report_catcher;
    report_catcher = iter->next();
  }

  return nullptr;
}

//------------------------------------------------------------------------------
// member function: print_catcher (static)
//
//! Prints information about all of the report catchers that are
//! registered. For finer grained detail, the uvm_callbacks<T,CB>::display
//! method can be used by calling uvm_report_cb::display(#uvm_report_object).
//------------------------------------------------------------------------------

void uvm_report_catcher::print_catcher( UVM_FILE file )
{
  std::ostringstream msg;
  std::string enabled;
  uvm_report_catcher* catcher;
  std::vector<std::string> q;

  // Refactored from the former function-local iter global to uvm_coreservice_t.
  uvm_report_cb_iter* iter = print_iter_ref();

  q.push_back("-------------UVM REPORT CATCHERS----------------------------\n");

  catcher = iter->first();

  while(catcher != nullptr)
  {
    if(catcher->callback_mode())
      enabled = "ON";
    else
      enabled = "OFF";

    msg << std::setw(20)
        << catcher->get_name()
        << " : "
        << enabled
        << "\n";
    q.push_back(msg.str());

    catcher = iter->next();
  }

  q.push_back("--------------------------------------------------------------\n");

  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_root* top = cs->get_root();

  UVM_INFO_CONTEXT("UVM/REPORT/CATCHER", UVM_STRING_QUEUE_STREAMING_PACK(q), UVM_LOW, top)
}

//------------------------------------------------------------------------------
// member function: debug_report_catcher (static)
//
//! Turn on report catching debug information. \p what is a bitwise and of
//! * DO_NOT_CATCH  -- forces catch to be ignored so that all catchers see the
//!   the reports.
//! * DO_NOT_MODIFY -- forces the message to remain unchanged
//------------------------------------------------------------------------------

void uvm_report_catcher::debug_report_catcher( int what )
{
  uvm_report_catcher_data::get().m_debug_flags = what;
}

//------------------------------------------------------------------------------
// Group: Callback Interface
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// member function: catch
//
// This is the method that is called for each registered report catcher.
// There are no arguments to this function. The <Current Message State>
// interface methods can be used to access information about the
// current message being processed.
//------------------------------------------------------------------------------

//TODO catch function?
//action_e catch();


//------------------------------------------------------------------------------
// Group: Reporting
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// member function: uvm_report_fatal
//
//! Issues a fatal message using the current message's report object.
//! This message will bypass any message catching callbacks.
//------------------------------------------------------------------------------
 
void uvm_report_catcher::uvm_report_fatal( const std::string& id,
                                           const std::string& message,
                                           int verbosity,
                                           const std::string& fname,
                                           int line,
                                           const std::string& context_name,
                                           bool report_enabled_checked )
{
  uvm_report(UVM_FATAL, id, message, UVM_NONE, fname, line,
             context_name, report_enabled_checked);

}


//------------------------------------------------------------------------------
// member function: uvm_report_error
//
//! Issues a error message using the current message's report object.
//! This message will bypass any message catching callbacks.
//------------------------------------------------------------------------------

void uvm_report_catcher::uvm_report_error( const std::string& id,
                                           const std::string& message,
                                           int verbosity,
                                           const std::string& fname,
                                           int line,
                                           const std::string& context_name,
                                           bool report_enabled_checked )
{
  uvm_report(UVM_ERROR, id, message, UVM_NONE, fname, line,
             context_name, report_enabled_checked);
}


//------------------------------------------------------------------------------
// member function: uvm_report_warning
//
//! Issues a warning message using the current message's report object.
//! This message will bypass any message catching callbacks.
//------------------------------------------------------------------------------

void uvm_report_catcher::uvm_report_warning( const std::string& id,
                                             const std::string& message,
                                             int verbosity,
                                             const std::string& fname,
                                             int line,
                                             const std::string& context_name,
                                             bool report_enabled_checked )
{
  uvm_report(UVM_WARNING, id, message, UVM_NONE, fname, line,
             context_name, report_enabled_checked);
}


//------------------------------------------------------------------------------
// member function: uvm_report_info
//
//! Issues a info message using the current message's report object.
//! This message will bypass any message catching callbacks.
//------------------------------------------------------------------------------

void uvm_report_catcher::uvm_report_info( const std::string& id,
                                          const std::string& message,
                                          int verbosity,
                                          const std::string& fname,
                                          int line,
                                          const std::string& context_name,
                                          bool report_enabled_checked )
{
  uvm_report(UVM_INFO, id, message, verbosity, fname, line,
             context_name, report_enabled_checked);
}

//------------------------------------------------------------------------------
// member function: uvm_report
//
//! Issues a message using the current message's report object.
//! This message will bypass any message catching callbacks.
//------------------------------------------------------------------------------

void uvm_report_catcher::uvm_report( uvm_severity severity,
                                     const std::string& id,
                                     const std::string& message,
                                     int verbosity,
                                     const std::string& fname,
                                     int line,
                                     const std::string& context_name,
                                     bool report_enabled_checked )

{
  uvm_report_message* l_report_message;
  if (report_enabled_checked == false)
  {
    if (!uvm_report_enabled(verbosity, severity, id))
      return;
  }

  l_report_message = uvm_report_message::new_report_message();
  l_report_message->set_report_message(severity, id, message,
                                      verbosity, fname, line, context_name);
  uvm_process_report_message(l_report_message);

}

//------------------------------------------------------------------------------
// member function: issue
//
//! Immediately issues the message which is currently being processed. This
//! is useful if the message is being CAUGHT but should still be emitted.
//!
//! Issuing a message will update the report_server stats, possibly multiple
//! times if the message is not CAUGHT.
//------------------------------------------------------------------------------

void uvm_report_catcher::issue()
{
  std::string composed_message;

  uvm_report_catcher_data& rcd = uvm_report_catcher_data::get();

  uvm_report_server* rs = rcd.m_modified_report_message->get_report_server();

  if(rcd.m_modified_report_message->get_action() != UVM_NO_ACTION)
  {
    // no need to compose when neither UVM_DISPLAY nor UVM_LOG is set
    if (rcd.m_modified_report_message->get_action() & (UVM_LOG|UVM_DISPLAY))
      composed_message = rs->compose_report_message(rcd.m_modified_report_message);
    rs->execute_report_message(rcd.m_modified_report_message, composed_message);
  }
}


//------------------------------------------------------------------------------
// member function: process_all_report_catchers (static)
//
//! Method called by report_server.report to process catchers
//------------------------------------------------------------------------------

bool uvm_report_catcher::process_all_report_catchers( uvm_report_message* rm )
{
  uvm_report_catcher_data& rcd = uvm_report_catcher_data::get();
  int iter;
  uvm_report_catcher* catcher;
  int thrown = true;
  uvm_severity orig_severity;
  // Refactored from the former function-local in_catcher global to uvm_coreservice_t.
  bool& in_catcher = in_catcher_ref();
  uvm_report_object* l_report_object = rm->get_report_object();

  if(in_catcher)
    return true;

  in_catcher = true;
  uvm_callbacks_base::tracing_enabled() = false;  //turn off cb tracing so catcher stuff doesn't print

  orig_severity = rm->get_severity();
  rcd.m_modified_report_message = rm;

  catcher = uvm_report_cb::get_first(iter, l_report_object);
  if (catcher != nullptr)
  {
    if(rcd.m_debug_flags & rcd.DO_NOT_MODIFY)
    {
      //process p = process::self(); // Keep random stability
      //string randstate;
      //if (p != null)
      //  randstate = p.get_randstate();
      rcd.m_orig_report_message = dynamic_cast<uvm_report_message*>(rm->clone()); //have to clone, rm can be extended type
      //if (p != null)
      //  p.set_randstate(randstate);
    }
  }

  while(catcher != nullptr)
  {
    uvm_severity prev_sev;

    if (!catcher->callback_mode())
    {
      catcher = uvm_report_cb::get_next(iter, l_report_object);
      continue;
    }

    prev_sev = rcd.m_modified_report_message->get_severity();
    rcd.m_set_action_called = false;
    thrown = catcher->process_report_catcher();

    // Set the action to the default action for the new severity
    // if it is still at the default for the previous severity,
    // unless it was explicitly set.
    if (!rcd.m_set_action_called &&
         rcd.m_modified_report_message->get_severity() != prev_sev &&
         rcd.m_modified_report_message->get_action() ==
           l_report_object->get_report_action(prev_sev, "*@&*^*^*#"))
    {
      rcd.m_modified_report_message->set_action(
          l_report_object->get_report_action(rcd.m_modified_report_message->get_severity(), "*@&*^*^*#"));
    }

    if(thrown == false)
    {
      switch(orig_severity)
      {
        case UVM_FATAL:   rcd.m_caught_fatal++; break;
        case UVM_ERROR:   rcd.m_caught_error++; break;
        case UVM_WARNING: rcd.m_caught_warning++; break;
        case UVM_INFO: break; // skip UVM_INFO
        default: break; // do nothing
      }
      break;
    } // if
    catcher = uvm_report_cb::get_next(iter, l_report_object);
  } //while

  //update counters if message was returned with demoted severity
  switch(orig_severity)
  {
    case UVM_FATAL: if(rcd.m_modified_report_message->get_severity() < orig_severity) rcd.m_demoted_fatal++; break;
    case UVM_ERROR: if(rcd.m_modified_report_message->get_severity() < orig_severity) rcd.m_demoted_error++; break;
    case UVM_WARNING: if(rcd.m_modified_report_message->get_severity() < orig_severity) rcd.m_demoted_warning++; break;
    case UVM_INFO: break; // nothing to do for UVM_INFO
    default: break;
  }

  in_catcher = false;
  uvm_callbacks_base::tracing_enabled() = true;  // turn tracing stuff back on

  return thrown;
}


//------------------------------------------------------------------------------
// member function: process_report_catcher
//
//! Internal method to call user do_catch() method
//------------------------------------------------------------------------------

int uvm_report_catcher::process_report_catcher()
{
  uvm_report_catcher_data& rcd = uvm_report_catcher_data::get();
  action_e act;

  act = do_catch(); // was catch in UVM-SV, but is reserved keyword in C++

  if(act == UNKNOWN_ACTION)
  {
    this->uvm_report_error("RPTCTHR",
                           "uvm_report_this.catch() in catcher instance " + get_name() +
                           " must return THROW or CAUGHT",
                           UVM_NONE, UVM_FILE_M, UVM_LINE_M );
  }

  if(rcd.m_debug_flags & rcd.DO_NOT_MODIFY)
    uvm_report_catcher_data::get().m_modified_report_message->copy(*rcd.m_orig_report_message);


  if(act == CAUGHT  && !(rcd.m_debug_flags & rcd.DO_NOT_CATCH))
    return 0;

  return 1;
}


//------------------------------------------------------------------------------
// member function: summarize (static)
//
//! This function is called automatically by uvm_report_server::summarize().
//! It prints the statistics for the active catchers.
//------------------------------------------------------------------------------

void uvm_report_catcher::summarize()
{
  std::ostringstream s;
  std::vector<std::string> q;
  uvm_report_catcher_data& rcd = uvm_report_catcher_data::get();

  if(rcd.do_report)
  {
    q.push_back("\n--- UVM Report catcher Summary ---\n\n\n");

    s.clear(); s.str("");
    s << "Number of demoted UVM_FATAL reports  :"
      << std::setw(5)
      << rcd.m_demoted_fatal
      << "\n";
    q.push_back(s.str());

    s.clear(); s.str("");
    s << "Number of demoted UVM_ERROR reports  :"
      << std::setw(5)
      << rcd.m_demoted_error
      << "\n";
    q.push_back(s.str());

    s.clear(); s.str("");
    s << "Number of demoted UVM_WARNING reports:"
      << std::setw(5)
      << rcd.m_demoted_warning
      << "\n";
    q.push_back(s.str());

    s.clear(); s.str("");
    s << "Number of caught UVM_FATAL reports   :"
      << std::setw(5)
      << rcd.m_caught_fatal
      << "\n";
    q.push_back(s.str());

    s.clear(); s.str("");
    s << "Number of caught UVM_ERROR reports   :"
      << std::setw(5)
      << rcd.m_caught_error
      << "\n";
    q.push_back(s.str());

    s.clear(); s.str("");
    s << "Number of caught UVM_WARNING reports :"
      << std::setw(5)
      << rcd.m_caught_warning
      << "\n";
    q.push_back(s.str());

    uvm_coreservice_t* cs = uvm_coreservice_t::get();
    uvm_root* top = cs->get_root();

    UVM_INFO_CONTEXT("UVM/REPORT/CATCHER", UVM_STRING_QUEUE_STREAMING_PACK(q), UVM_LOW, top );
  }
}

void uvm_report_catcher::uvm_process_report_message(uvm_report_message* msg)
{
  uvm_report_object* ro = uvm_report_catcher_data::get().m_modified_report_message->get_report_object();
  uvm_action a = ro->get_report_action(msg->get_severity(), msg->get_id());

  if(a)
  {
    std::string composed_message;
    uvm_report_server* rs = uvm_report_catcher_data::get().m_modified_report_message->get_report_server();

    msg->set_report_object(ro);
    msg->set_report_handler(uvm_report_catcher_data::get().m_modified_report_message->get_report_handler());
    msg->set_report_server(rs);
    msg->set_file(ro->get_report_file_handle(msg->get_severity(), msg->get_id()));
    msg->set_action(a);

    // no need to compose when neither UVM_DISPLAY nor UVM_LOG is set
    if (a & (UVM_LOG|UVM_DISPLAY))
      composed_message = rs->compose_report_message(msg);

    rs->execute_report_message(msg, composed_message);
  }
}


} // namespace uvm
