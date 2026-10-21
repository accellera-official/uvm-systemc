//------------------------------------------------------------------------------
//   Copyright 2013-2016 NXP B.V.
//   Copyright 2007-2010 Mentor Graphics Corporation
//   Copyright 2007-2009 Cadence Design Systems, Inc.
//   Copyright 2010 Synopsys, Inc.
//   All Rights Reserved Worldwide
//
//   Licensed under the Apache License, Version 2.0 (the
//   "License")), you may not use this file except in
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

#include "uvmsc/report/uvm_report_catcher_data.h"

namespace uvm {

//------------------------------------------------------------------------------
// Initialization static data members
//------------------------------------------------------------------------------

uvm_report_catcher_data::uvm_report_catcher_data()
  : m_modified_report_message(nullptr),
    m_orig_report_message(nullptr),
    m_set_action_called(false),
    m_demoted_fatal(0),
    m_demoted_error(0),
    m_demoted_warning(0),
    m_caught_fatal(0),
    m_caught_error(0),
    m_caught_warning(0),
    DO_NOT_CATCH(1),
    DO_NOT_MODIFY(2),
    m_debug_flags(0),
    do_report(false)
{
}

} // namespace uvm

