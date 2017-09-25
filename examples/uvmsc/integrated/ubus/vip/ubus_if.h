//----------------------------------------------------------------------
//   Copyright 2016 NXP B.V.
//   Copyright 2007-2010 Mentor Graphics Corporation
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

#ifndef UBUS_IF_H_
#define UBUS_IF_H_

#include <systemc>
#include <uvm>

// TODO check multiple writers...


typedef sc_core::sc_signal< sc_dt::sc_logic > logic;
typedef sc_core::sc_signal< sc_dt::sc_logic, sc_core::SC_UNCHECKED_WRITERS > logic_nchk;
typedef sc_core::sc_signal< sc_dt::sc_logic, sc_core::SC_MANY_WRITERS> logic_md;

typedef sc_core::sc_signal_resolved logic_res;
typedef sc_core::sc_signal< sc_dt::sc_uint<16> > uint16;
typedef sc_core::sc_signal< sc_dt::sc_lv<2> > logic2;
typedef sc_core::sc_signal< sc_dt::sc_lv<8> > logic8;

typedef sc_core::sc_signal_rv<2> logic2_res;
typedef sc_core::sc_signal_rv<8> logic8_res;
typedef sc_core::sc_signal_rv<16> logic16_res;
typedef sc_core::sc_signal< sc_dt::sc_lv<16> > logic16;
typedef sc_core::sc_signal< sc_dt::sc_uint<2> > uint2;
typedef sc_core::sc_signal< sc_dt::sc_uint<8> > uint8;



//----------------------------------------------------------------------
//  Class: ubus_if
//----------------------------------------------------------------------

class ubus_if : sc_core::sc_module 
{
 public:

  SC_HAS_PROCESS(ubus_if);

  ubus_if(sc_core::sc_module_name name)
  : sc_core::sc_module(name),
    sig_clock("sig_clock"),
    sig_reset("sig_reset"),
    sig_addr("sig_addr"),
    sig_size("sig_size"),
    sig_read("sig_read"),
    sig_write("sig_write"),
    sig_start("sig_start"),
    sig_bip("sig_bip"),
    sig_data("sig_data"),
    sig_data_out("sig_data_out"),
    sig_wait("sig_wait"),
    sig_error("sig_error"),
    rw("rw"),
    has_checks(true),
    has_coverage(true)
    {
      SC_THREAD(proc);
    }
  
  // Actual Signals
  logic sig_clock;
  logic sig_reset;
  
  logic_md sig_request[16]; // [15:0]
  logic sig_grant[16];   // [15:0]

  logic16_res sig_addr;
  logic2_res sig_size;

  logic_res sig_read;
  logic_res sig_write;
  logic sig_start;
  logic_res sig_bip;

  logic8_res sig_data;
  logic8_res sig_data_out;

  logic_res sig_wait;
  logic_res sig_error;

  logic_res rw;

  // Control flags
  bool has_checks;
  bool has_coverage;

  void proc()
  {
    while(true) // forever
    {
      sc_core::wait(sig_data_out.default_event());
      if ((rw == sc_dt::SC_LOGIC_1) || (rw == sc_dt::SC_LOGIC_X))
      {
        sig_data = sig_data_out;
        sc_core::wait(sc_core::SC_ZERO_TIME);
        std::cout << sc_core::sc_time_stamp() << ": [vif] sig_data = " << sig_data << " sig_data_out = " << sig_data_out << std::endl;
      }
      else // Z or 0
      {
        sig_data = "zzzzzzzz";
        sc_core::wait(sc_core::SC_ZERO_TIME);
        std::cout << sc_core::sc_time_stamp() << ": [vif] sig_data overloaded with 'zzzzzzzz'" << std::endl;
      }
    }
  }

/* Coverage and assertions to be implemented here.

always @(negedge sig_clock)
begin

// Address must not be X or Z during Address Phase
assertAddrUnknown:assert property (
                  disable iff(!has_checks) 
                  ($onehot(sig_grant) |-> !$isunknown(sig_addr)))
                  else
                    $error("ERR_ADDR_XZ\n Address went to X or Z \
                            during Address Phase");

// Read must not be X or Z during Address Phase
assertReadUnknown:assert property ( 
                  disable iff(!has_checks) 
                  ($onehot(sig_grant) |-> !$isunknown(sig_read)))
                  else
                    $error("ERR_READ_XZ\n READ went to X or Z during \
                            Address Phase");

// Write must not be X or Z during Address Phase
assertWriteUnknown:assert property ( 
                   disable iff(!has_checks) 
                   ($onehot(sig_grant) |-> !$isunknown(sig_write)))
                   else
                     $error("ERR_WRITE_XZ\n WRITE went to X or Z during \
                             Address Phase");

// Size must not be X or Z during Address Phase
assertSizeUnknown:assert property ( 
                  disable iff(!has_checks) 
                  ($onehot(sig_grant) |-> !$isunknown(sig_size)))
                  else
                    $error("ERR_SIZE_XZ\n SIZE went to X or Z during \
                            Address Phase");


// Wait must not be X or Z during Data Phase
assertWaitUnknown:assert property ( 
                  disable iff(!has_checks) 
                  ($onehot(sig_grant) |=> !$isunknown(sig_wait)))
                  else
                    $error("ERR_WAIT_XZ\n WAIT went to X or Z during \
                            Data Phase");


// Error must not be X or Z during Data Phase
assertErrorUnknown:assert property ( 
                   disable iff(!has_checks) 
                   ($onehot(sig_grant) |=> !$isunknown(sig_error)))
                   else
                    $error("ERR_ERROR_XZ\n ERROR went to X or Z during \
                            Data Phase");


//Reset must be asserted for at least 3 clocks each time it is asserted
assertResetFor3Clocks: assert property (
                       disable iff(!has_checks) 
                       ($rose(sig_reset) |=> sig_reset[*2]))
                       else 
                         $error("ERR_SHORT_RESET_DURING_TEST\n",
                                "Reset was asserted for less than 3 clock \
                                 cycles");

// Only one grant is asserted
//assertSingleGrant: assert property (
//                   disable iff(!has_checks)
//                   (sig_start |=> $onehot0(sig_grant)))
//                   else
//                     $error("ERR_GRANT\n More that one grant asserted");

// Read and write never true at the same time
assertReadOrWrite: assert property (
                   disable iff(!has_checks) 
                   ($onehot(sig_grant) |-> !(sig_read && sig_write)))
                   else
                     $error("ERR_READ_OR_WRITE\n Read and Write true at \
                             the same time");

end

*/

}; // class ubus_if


#endif /* UBUS_IF_H_ */
