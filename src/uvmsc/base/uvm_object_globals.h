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

#ifndef UVM_OBJECT_GLOBALS_H_
#define UVM_OBJECT_GLOBALS_H_

#include <systemc>
#include <iostream>     // std::cout
#include <fstream>      // std::ofstream

namespace uvm {

// forward class declarations
class uvm_printer;
class uvm_table_printer;
class uvm_line_printer;
class uvm_tree_printer;
class uvm_comparer;

//----------------------------------------------------------------------
// Macro: UVM_DEFAULT_TIMEOUT
//
// The default timeout for simulation, if not overridden by
// <uvm_root::set_timeout> or <+UVM_TIMEOUT>
// If not set, the defaults is set to 9200 sec.
//----------------------------------------------------------------------

#ifndef UVM_DEFAULT_TIMEOUT
#define UVM_DEFAULT_TIMEOUT sc_core::sc_time(9200, sc_core::SC_SEC);
#endif

//----------------------------------------------------------------------
// Enum: uvm_active_passive_enum
//
// Convenience value to define whether a component, usually an agent,
// is in "active" mode or "passive" mode.
//----------------------------------------------------------------------

typedef enum { UVM_PASSIVE = 0, UVM_ACTIVE = 1 } uvm_active_passive_enum;

//----------------------------------------------------------------------
// Enum: uvm_sequence_state_type
//
// Defines current sequence state
//
// CREATED            - The sequence has been allocated.
// PRE_START          - The sequence is started and the
//                      <uvm_sequence_base::pre_start()> task is
//                      being executed.
// PRE_BODY           - The sequence is started and the
//                      <uvm_sequence_base::pre_body()> task is
//                      being executed.
// BODY               - The sequence is started and the
//                      <uvm_sequence_base::body()> task is
//                      being executed.
// ENDED              - The sequence has completed the execution of the
//                      <uvm_sequence_base::body()> task.
// POST_BODY          - The sequence is started and the
//                      <uvm_sequence_base::post_body()> task is
//                      being executed.
// POST_START         - The sequence is started and the
//                      <uvm_sequence_base::post_start()> task is
//                      being executed.
// STOPPED            - The sequence has been forcibly ended by issuing a
//                      <uvm_sequence_base::kill()> on the sequence.
// FINISHED           - The sequence is completely finished executing.
//----------------------------------------------------------------------

typedef enum
{
  UVM_CREATED   = 1,
  UVM_PRE_START = 2,
  UVM_PRE_BODY  = 4,
  UVM_BODY      = 8,
  UVM_POST_BODY = 16,
  UVM_POST_START= 32,
  UVM_ENDED     = 64,
  UVM_STOPPED   = 128,
  UVM_FINISHED  = 256
} uvm_sequence_state_enum;

extern const char *uvm_seq_state_name[];

//----------------------------------------------------------------------
// Enum: uvm_verbosity_type
//
// Defines standard verbosity levels for reports.
//
//  UVM_NONE   - Report is always printed. Verbosity level setting can not
//               disable it.
//  UVM_LOW    - Report is issued if configured verbosity is set to UVM_LOW
//               or above.
//  UVM_MEDIUM - Report is issued if configured verbosity is set to UVM_MEDIUM
//               or above.
//  UVM_HIGH   - Report is issued if configured verbosity is set to UVM_HIGH
//               or above.
//  UVM_FULL   - Report is issued if configured verbosity is set to UVM_FULL
//               or above.
//----------------------------------------------------------------------

typedef enum
{
  UVM_NONE   = 0,
  UVM_LOW    = 100,
  UVM_MEDIUM = 200,
  UVM_HIGH   = 300,
  UVM_FULL   = 400,
  UVM_DEBUG  = 500
} uvm_verbosity;

extern const char *uvm_verbosity_name[];

//----------------------------------------------------------------------
// Define number of maximum sequences
//----------------------------------------------------------------------

#define UVM_MAX_SEQS  1000

//----------------------------------------------------------------------
// Enum: uvm_sequencer_arb_mode
//
// Specifies a sequencer's arbitration mode
//
// SEQ_ARB_FIFO          - Requests are granted in FIFO order (default)
// SEQ_ARB_WEIGHTED      - Requests are granted randomly by weight
// SEQ_ARB_RANDOM        - Requests are granted randomly
// SEQ_ARB_STRICT_FIFO   - Requests at highest priority granted in fifo order
// SEQ_ARB_STRICT_RANDOM - Requests at highest priority granted in randomly
// SEQ_ARB_USER          - Arbitration is delegated to the user-defined
//                         function, user_priority_arbitration. That function
//                         will specify the next sequence to grant.
//----------------------------------------------------------------------

typedef enum
{
  SEQ_ARB_FIFO,
  SEQ_ARB_WEIGHTED,
  SEQ_ARB_RANDOM,
  SEQ_ARB_STRICT_FIFO,
  SEQ_ARB_STRICT_RANDOM,
  SEQ_ARB_USER
} uvm_sequencer_arb_mode;

typedef uvm_sequencer_arb_mode SEQ_ARB_TYPE; // backward compatibility

extern const char *uvm_seq_arb_name[]; // TODO not in SV

//---------------
// Group: Phasing
//---------------

//----------------------------------------------------------------------
// Enum: uvm_phase_type
//
// This is an attribute of a <uvm_phase> object which defines the phase
// type.
//
//   UVM_PHASE_IMP      - The phase object is used to traverse the component
//                        hierarchy and call the component phase method as
//                        well as the ~phase_started~ and ~phase_ended~ callbacks.
//                        These nodes are created by the phase macros,
//                        `uvm_builtin_task_phase, `uvm_builtin_topdown_phase,
//                        and `uvm_builtin_bottomup_phase. These nodes represent
//                        the phase type, i.e. uvm_run_phase, uvm_main_phase.
//
//   UVM_PHASE_NODE     - The object represents a simple node instance in
//                        the graph. These nodes will contain a reference to
//                        their corresponding IMP object.
//
//   UVM_PHASE_SCHEDULE - The object represents a portion of the phasing graph,
//                        typically consisting of several NODE types, in series,
//                        parallel, or both.
//
//   UVM_PHASE_TERMINAL - This internal object serves as the termination NODE
//                        for a SCHEDULE phase object.
//
//   UVM_PHASE_DOMAIN   - This object represents an entire graph segment that
//                        executes in parallel with the 'run' phase.
//                        Domains may define any network of NODEs and
//                        SCHEDULEs. The built-in domain, ~uvm~, consists
//                        of a single schedule of all the run-time phases,
//                        starting with ~pre_reset~ and ending with
//                        ~post_shutdown~.
//----------------------------------------------------------------------

typedef enum {
  UVM_PHASE_IMP,
  UVM_PHASE_NODE,
  UVM_PHASE_TERMINAL,
  UVM_PHASE_SCHEDULE,
  UVM_PHASE_DOMAIN,
  UVM_PHASE_GLOBAL
} uvm_phase_type;


extern const char *uvm_phase_type_name[];

//----------------------------------------------------------------------
// Enum: uvm_phase_state
// ---------------------
//
// The set of possible states of a phase. This is an attribute of a schedule
// node in the graph, not of a phase, to maintain independent per-domain state
//
//   UVM_PHASE_DORMANT -  Nothing has happened with the phase in this domain.
//
//   UVM_PHASE_SCHEDULED - At least one immediate predecessor has completed.
//              Scheduled phases block until all predecessors complete or
//              until a jump is executed.
//
//   UVM_PHASE_SYNCING - All predecessors complete, checking that all synced
//              phases (e.g. across domains) are at or beyond this point
//
//   UVM_PHASE_STARTED - phase ready to execute, running phase_started() callback
//
//   UVM_PHASE_EXECUTING - An executing phase is one where the phase callbacks are
//              being executed. It's process is tracked by the phaser.
//
//   UVM_PHASE_READY_TO_END - no objections remain in this phase or in any
//              predecessors of its successors or in any sync'd phases. This
//              state indicates an opportunity for any phase that needs extra
//              time for a clean exit to raise an objection, thereby causing a
//              return to UVM_PHASE_EXECUTING.  If no objection is raised, state
//              will transition to UVM_PHASE_ENDED after a delta cycle.
//              (An example of predecessors of successors: The successor to
//              phase 'run' is 'extract', whose predecessors are 'run' and
//              'post_shutdown'. Therefore, 'run' will go to this state when
//              both its objections and those of 'post_shutdown' are all dropped.
//
//   UVM_PHASE_ENDED - phase completed execution, now running phase_ended() callback
//
//   UVM_PHASE_CLEANUP - all processes related to phase are being killed
//
//   UVM_PHASE_DONE - A phase is done after it terminated execution.  Becoming
//              done may enable a waiting successor phase to execute.
//
//    The state transitions occur as follows:
//
//|   DORMANT -> SCHED -> SYNC -> START -> EXEC -> READY -> END -> CLEAN -> DONE
//|      ^                                                            |
//|      |                      <-- jump_to                           v
//|      +------------------------------------------------------------+
//
//----------------------------------------------------------------------

typedef enum { UVM_PHASE_DORMANT      = 0, //was 1
               UVM_PHASE_SCHEDULED    = 1, //2
               UVM_PHASE_SYNCING      = 2, //4
               UVM_PHASE_STARTED      = 3, //8
               UVM_PHASE_EXECUTING    = 4, //16
               UVM_PHASE_READY_TO_END = 5, //32
               UVM_PHASE_ENDED        = 6, //64
               UVM_PHASE_CLEANUP      = 7, //128
               UVM_PHASE_DONE         = 8, //258
               UVM_PHASE_JUMPING      = 9  //512
} uvm_phase_state;


extern const char* uvm_phase_state_name[];

//----------------------------------------------------------------------
// Enum: uvm_phase_transition
//
// These are the phase state transition for callbacks which provide
// additional information that may be useful during callbacks
//
// UVM_COMPLETED   - the phase completed normally
// UVM_FORCED_STOP - the phase was forced to terminate prematurely
// UVM_SKIPPED     - the phase was in the path of a forward jump
// UVM_RERUN       - the phase was in the path of a backwards jump
//----------------------------------------------------------------------

typedef enum { UVM_COMPLETED   = 0x01,
               UVM_FORCED_STOP = 0x02,
               UVM_SKIPPED     = 0x04,
               UVM_RERUN       = 0x08
} uvm_phase_transition;


//----------------------------------------------------------------------
// Enum: uvm_wait_op
//
// Specifies the operand when using methods like <uvm_phase::wait_for_state>.
//
// UVM_EQ  - equal
// UVM_NE  - not equal
// UVM_LT  - less than
// UVM_LTE - less than or equal to
// UVM_GT  - greater than
// UVM_GTE - greater than or equal to
//----------------------------------------------------------------------
typedef enum { UVM_LT,
               UVM_LTE,
               UVM_NE,
               UVM_EQ,
               UVM_GT,
               UVM_GTE
} uvm_wait_op;


//----------------------------------------------------------------------
// Group: Objections
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Enum: uvm_objection_event
//
// Enumerated the possible objection events one could wait on. See
// <uvm_objection::wait_for>.
//
// UVM_RAISED      - an objection was raised
// UVM_DROPPED     - an objection was raised
// UVM_ALL_DROPPED - all objections have been dropped
//----------------------------------------------------------------------

typedef enum { UVM_RAISED      = 0x01,
               UVM_DROPPED     = 0x02,
               UVM_ALL_DROPPED = 0x04
} uvm_objection_event;


//----------------------------------------------------------------------
// Group: Reporting
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Enum: uvm_severity
//
// Defines all possible values for report severity.
//
//   UVM_INFO    - Informative messsage.
//   UVM_WARNING - Indicates a potential problem.
//   UVM_ERROR   - Indicates a real problem. Simulation continues subject
//                 to the configured message action.
//   UVM_FATAL   - Indicates a problem from which simulation can not
//                 recover. Simulation exits via $finish after a #0 delay.
//----------------------------------------------------------------------

typedef enum
{
  UVM_INFO,
  UVM_WARNING,
  UVM_ERROR,
  UVM_FATAL
} uvm_severity;

extern const char *uvm_severity_name[];


//----------------------------------------------------------------------
// Enum: uvm_action
//
// Defines all possible values for report actions. Each report is configured
// to execute one or more actions, determined by the bitwise OR of any or all
// of the following enumeration constants.
//
//   UVM_NO_ACTION - No action is taken
//   UVM_DISPLAY   - Sends the report to the standard output
//   UVM_LOG       - Sends the report to the file(s) for this (severity,id) pair
//   UVM_COUNT     - Counts the number of reports with the COUNT attribute.
//                   When this value reaches max_quit_count, the simulation terminates
//   UVM_EXIT      - Terminates the simulation immediately.
//   UVM_CALL_HOOK - Callback the report hook methods
//   UVM_STOP      - Causes ~$stop~ to be executed, putting the simulation into
//                   interactive mode.
//----------------------------------------------------------------------

typedef int uvm_action;

typedef enum
{
  UVM_NO_ACTION = 0,  // 0b0000000
  UVM_DISPLAY   = 1,  // 0b0000001
  UVM_LOG       = 2,  // 0b0000010
  UVM_COUNT     = 4,  // 0b0000100
  UVM_EXIT      = 8,  // 0b0001000
  UVM_CALL_HOOK = 16, // 0b0010000
  UVM_STOP      = 32, // 0b0100000
  UVM_RM_RECORD = 64  // 0b1000000
} uvm_action_type;


//----------------------------------------------------------------------
// Enum: uvm_radix_enum
//
// Specifies the radix to print or record in.
//
// UVM_BIN       - Selects binary (%b) format
// UVM_DEC       - Selects decimal (%d) format
// UVM_UNSIGNED  - Selects unsigned decimal (%u) format
// UVM_OCT       - Selects octal (%o) format
// UVM_HEX       - Selects hexidecimal (%h) format
// UVM_STRING    - Selects string (%s) format
// UVM_TIME      - Selects time (%t) format
// UVM_ENUM      - Selects enumeration value (name) format
//----------------------------------------------------------------------

typedef enum {
   UVM_BIN       = 0, //0x1000000,
   UVM_DEC       = 1, //0x2000000,
   UVM_UNSIGNED  = 2, //0x3000000,
   UVM_UNFORMAT2 = 3, //0x4000000,
   UVM_UNFORMAT4 = 4, //0x5000000,
   UVM_OCT       = 5, //0x6000000,
   UVM_HEX       = 6, //0x7000000,
   UVM_STRING    = 7, //0x8000000,
   UVM_TIME      = 8, //0x9000000,
   UVM_ENUM      = 9, //0xa000000,
   UVM_REAL      = 10, //xb000000,
   UVM_REAL_DEC  = 11, //0xc000000,
   UVM_REAL_EXP  = 12, //0xd000000,
   UVM_NORADIX   = 13 //0
} uvm_radix_enum;

extern const char *uvm_radix_enum_name[];

//----------------------------------------------------------------------
// Enum: uvm_recursion_policy_enum
//
// Specifies the policy for copying objects.
//
// UVM_DEEP      - Objects are deep copied (object must implement copy method)
// UVM_SHALLOW   - Objects are shallow copied using default SV copy.
// UVM_REFERENCE - Only object handles are copied.
//----------------------------------------------------------------------

typedef enum {
  UVM_DEFAULT_POLICY = 0,
  UVM_DEEP           = 0x400,
  UVM_SHALLOW        = 0x800,
  UVM_REFERENCE      = 0x1000
} uvm_recursion_policy_enum;

//----------------------------------------------------------------------
// Enum: uvm_field_enum
//
// Specifies the field attribute used for comparison
//
// UVM_PHYSICAL - Physical field
// UVM_ABSTRACT - Abstract field
//
// NOTE: This enum does not exist in UVM-SV, and is introduced in UVM-SystemC
// to replace uvm_comparer data members
//----------------------------------------------------------------------

typedef enum {
  UVM_PHYSICAL,
  UVM_ABSTRACT
} uvm_field_enum;


//----------------------------------------------------------------------
// Define: UVM_MAX_STREAMBITS
//
// Defines the maximum bit vector size for integral types.
//----------------------------------------------------------------------

#ifndef UVM_MAX_STREAMBITS
#define UVM_MAX_STREAMBITS 4096
#endif

#define UVM_STREAMBITS UVM_MAX_STREAMBITS;

//----------------------------------------------------------------------
// Define: UVM_PACKER_MAX_BYTES
//
// Defines the maximum bytes to allocate for packing an object using
// the #uvm_packer. Default is UVM_MAX_STREAMBITS, in bytes.
//----------------------------------------------------------------------

#ifndef UVM_PACKER_MAX_BYTES
#define UVM_PACKER_MAX_BYTES UVM_STREAMBITS
#endif


//----------------------------------------------------------------------
// Some typedefs
//
// These are implementation defined
//----------------------------------------------------------------------

template<bool isSmallStream>
struct uvm_bitstream_t_select;

template<>
struct uvm_bitstream_t_select<true> // bitstream size always smaller than 64-bit
{
  typedef sc_dt::sc_uint<UVM_MAX_STREAMBITS> type;
};

template<>
struct uvm_bitstream_t_select<false> // arbitrary sized bitstreams
{
  typedef sc_dt::sc_biguint<UVM_MAX_STREAMBITS> type;
};

typedef uvm_bitstream_t_select<((UVM_MAX_STREAMBITS)<=64)>::type uvm_bitstream_t;


typedef sc_dt::sc_uint<64> uvm_integral_t;


typedef std::ostream* UVM_FILE;

//----------------------------------------------------------------------
// typedef for UVM object names
// in the future we will replace this with a class which tracks the object
// instantiation tree
//----------------------------------------------------------------------

typedef const std::string& uvm_object_name;


} /* namespace uvm */



#endif /* UVM_OBJECT_GLOBALS_H_ */
