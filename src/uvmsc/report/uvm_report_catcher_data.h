//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#ifndef UVM_REPORT_CATCHER_DATA_H_
#define UVM_REPORT_CATCHER_DATA_H_

#include <string>

#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"
#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/report/uvm_report_object.h"

namespace uvm {

//----------------------------------------------------------------------------
// CLASS: uvm_report_catcher_data
//
//! TODO: add description
//----------------------------------------------------------------------------

class uvm_report_catcher_data
{
 private:
  friend class uvm_default_coreservice_t;

  uvm_report_catcher_data();

  // Disabled
  uvm_report_catcher_data(uvm_report_catcher_data const&);
  void operator=(uvm_report_catcher_data const&);

 public:

  static uvm_report_catcher_data& get() {
    return uvm_coreservice_t::get()->get_uvm_report_catcher_data_instance();
  }

  uvm_report_message* m_modified_report_message;
  uvm_report_message* m_orig_report_message;

  bool m_set_action_called;

  // Counts for the demoteds and caughts

  int m_demoted_fatal;
  int m_demoted_error;
  int m_demoted_warning;
  int m_caught_fatal;
  int m_caught_error;
  int m_caught_warning;

  // Flag counts

  const int DO_NOT_CATCH;
  const int DO_NOT_MODIFY;
  int m_debug_flags;


  bool do_report;

}; // class uvm_report_catcher_data

} // namespace uvm

#endif // UVM_REPORT_CATCHER_DATA_H_
