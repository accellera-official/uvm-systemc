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

#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"
#include "uvmsc/policy/uvm_comparer.h"

namespace uvm {

const char* uvm_seq_state_name[] = {
    "UVM_CREATED",
    "UVM_PRE_START",
    "UVM_PRE_BODY",
    "UVM_BODY",
    "UVM_POST_BODY",
    "UVM_POST_START",
    "UVM_ENDED",
    "UVM_STOPPED",
    "UVM_FINISHED"
};

const char* uvm_phase_type_name[] = {
  "UVM_PHASE_IMP",
  "UVM_PHASE_NODE",
  "UVM_PHASE_TERMINAL",
  "UVM_PHASE_SCHEDULE",
  "UVM_PHASE_DOMAIN",
  "UVM_PHASE_GLOBAL"
};


const char* uvm_phase_state_name[] = {
  "UVM_PHASE_DORMANT",
  "UVM_PHASE_SCHEDULED",
  "UVM_PHASE_SYNCING",
  "UVM_PHASE_STARTED",
  "UVM_PHASE_EXECUTING",
  "UVM_PHASE_READY_TO_END",
  "UVM_PHASE_ENDED",
  "UVM_PHASE_CLEANUP",
  "UVM_PHASE_DONE",
  "UVM_PHASE_JUMPING"
};

const char* uvm_radix_enum_name[] = {
    "UVM_BIN",
    "UVM_DEC",
    "UVM_UNSIGNED",
    "UVM_UNFORMAT2",
    "UVM_UNFORMAT4",
    "UVM_OCT",
    "UVM_HEX",
    "UVM_STRING",
    "UVM_TIME",
    "UVM_ENUM",
    "UVM_REAL",
    "UVM_REAL_DEC",
    "UVM_REAL_EXP",
    "UVM_NORADIX"
};

const char* uvm_seq_arb_name[] = {
  "SEQ_ARB_FIFO",
  "SEQ_ARB_WEIGHTED",
  "SEQ_ARB_RANDOM",
  "SEQ_ARB_STRICT_FIFO",
  "SEQ_ARB_STRICT_RANDOM",
  "SEQ_ARB_USER"
};

const char* uvm_severity_name[] = {
  "UVM_INFO",
  "UVM_WARNING",
  "UVM_ERROR",
  "UVM_FATAL"
};

const char* uvm_verbosity_name[] =
{
  "UVM_NONE",
  "UVM_LOW",
  "UVM_MEDIUM",
  "UVM_HIGH",
  "UVM_FULL",
  "UVM_DEBUG"
};


} /* namespace uvm */
