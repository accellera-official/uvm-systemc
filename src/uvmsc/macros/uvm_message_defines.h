//----------------------------------------------------------------------
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc.
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
//----------------------------------------------------------------------

#ifndef UVM_MESSAGE_DEFINES_H_
#define UVM_MESSAGE_DEFINES_H_

//----------------------------------------------------------------------
// Some generic defines which are used for all reporting macros
//----------------------------------------------------------------------

#ifndef UVM_LINE_WIDTH
#define UVM_LINE_WIDTH 120
#endif

#ifndef UVM_NUM_LINES
#define UVM_NUM_LINES 120
#endif

#ifdef UVM_REPORT_DISABLE_FILE_LINE
#define UVM_REPORT_DISABLE_FILE
#define UVM_REPORT_DISABLE_LINE
#endif

#ifdef UVM_REPORT_DISABLE_FILE
#define UVM_FILE_M ""
#else
#define UVM_FILE_M __FILE__
#endif

#ifdef UVM_REPORT_DISABLE_LINE
#define UVM_LINE_M 0
#else
#define UVM_LINE_M __LINE__
#endif

//----------------------------------------------------------------------
// MACRO: UVM_INFO
//
//! Calls uvm_report_info if \p VERBOSITY is lower than the configured
//! verbosity of the associated reporter. \p ID is given as the message tag
//! and \p MSG is given as the message text. The file and line are also sent
//! to the #uvm_report_info call.
//----------------------------------------------------------------------

#define UVM_INFO(ID,MSG,VERBOSITY) \
  if (uvm::uvm_report_enabled(VERBOSITY, ::uvm::UVM_INFO, ID)) \
    uvm::uvm_report_info(ID, MSG, VERBOSITY, __FILE__, __LINE__)

//----------------------------------------------------------------------
// MACRO: UVM_WARNING
//
//! Calls uvm_report_warning with a verbosity of UVM_NONE. The message can not
//! be turned off using the reporter's verbosity setting, but can be turned off
//! by setting the action for the message. \p ID is given as the message tag and
//! \p MSG is given as the message text. The file and line are also sent to the
//! uvm_report_warning call.
//----------------------------------------------------------------------

#define UVM_WARNING(ID,MSG) \
  if (uvm::uvm_report_enabled((int)::uvm::UVM_NONE, ::uvm::UVM_WARNING, std::string(ID))) \
    uvm::uvm_report_warning(ID, MSG, ::uvm::UVM_NONE, UVM_FILE_M, UVM_LINE_M)

//----------------------------------------------------------------------
// MACRO: UVM_ERROR
//
//! Calls uvm_report_error with a verbosity of UVM_NONE. The message can not
//! be turned off using the reporter's verbosity setting, but can be turned off
//! by setting the action for the message. \p ID is given as the message tag and
//! \p MSG is given as the message text. The file and line are also sent to the
//! #uvm_report_error call.
//----------------------------------------------------------------------

#define UVM_ERROR(ID,MSG) \
  if (uvm::uvm_report_enabled(::uvm::UVM_NONE, ::uvm::UVM_ERROR, ID)) \
    uvm::uvm_report_error(ID, MSG, ::uvm::UVM_NONE, UVM_FILE_M, UVM_LINE_M)

//----------------------------------------------------------------------
// MACRO: UVM_FATAL
//
//! Calls uvm_report_fatal with a verbosity of UVM_NONE. The message can not
//! be turned off using the reporter's verbosity setting, but can be turned off
//! by setting the action for the message. \p ID is given as the message tag and
//! \p MSG is given as the message text. The file and line are also sent to the
//! #uvm_report_fatal call.
//----------------------------------------------------------------------

#define UVM_FATAL(ID,MSG) \
  if (uvm::uvm_report_enabled(::uvm::UVM_NONE, ::uvm::UVM_FATAL, ID)) \
    uvm::uvm_report_fatal(ID, MSG, ::uvm::UVM_NONE, UVM_FILE_M, UVM_LINE_M)


#endif /* UVM_MESSAGE_DEFINES_H_ */
