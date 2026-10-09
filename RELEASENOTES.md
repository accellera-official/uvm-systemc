# UVM-SystemC Release Notes

December 31st, 2026

## License

This library is licensed under the Apache Software Foundations Apache License, 
Version 2.0, January 2004. 
The full license is available at: http://www.apache.org/licenses/

## Disclaimer

THE CONTRIBUTORS AND THEIR LICENSORS MAKE NO WARRANTY OF ANY KIND WITH REGARD
TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

## Acknowledgements

The creation of this document has been supported by the European Commission as 
part of the Seventh Framework Programme (FP7) for Research and Technological 
Development in the project 'VERIFICATION FOR HETEROGENOUS RELIABLE DESIGN AND 
INTEGRATION' (VERDI). The research leading to this result has received funding 
from the European Commission under grand agreement No 287562.

More information on the Seventh Framework Programme (FP7) and VERDI project 
can be found here:

   http://cordis.europa.eu/fp7

Special thanks to the Accellera Systems Initiative to grant authorization to 
use portions of its Universal Verification Methodology Reference Implementation
(UVM version 1.1d, March 2013) in this document.

The partners in the VERDI consortium wish to thank Cadence Design Systems Inc. 
for the initial donation of the UVM-SC Library Reference and documentation 
(UVM version 1.0, June 2011). This document has been derived from this work, 
and further enhanced and extended to make it compatible with the UVM IEEE 1800.2 
standard.

## What's new in this release?
Compared to the 1.0-beta6 release, this release adds:

  - Automatic memory management for transient objects through `uvm_handle<T>` (migration required; see below).
  - Adds a preprocessor macro ALLOW_NON_IEEE_1800_2_2020 that can be used to access some internal methods that are defined in UVM SystemVerilog but not part of the IEEE Standard
  - Bugfixes

### Migrating from the pointer-based API in the previous uvm betas

Previously, factory-created transactions remained allocated until explicitly
destroyed or simulation ended, and need to be managed manually using `T::type_id::destroy`.
The new API uses the `uvm_handle<T>` wareppr enabling automatic memory management and avoiding these implicit
copies. Objects are now deleted when their last owning handle is released.

- **Components:** keep using `T::type_id::create(...)`; the factory manages their lifetime.
- **Sequences and transactions:** use `T::type_id::create_handle(...)` instead
  of `create(...)`, access members with `->`, and remove manual `destroy()` calls
  for handle-owned objects.
- **Sequencers:** to customize a default sequence before it runs, pass the
  configured instance through `uvm_config_db<uvm::uvm_handle<uvm::uvm_sequence_base>>`
  instead of `uvm_config_db<uvm::uvm_sequence_base*>`. The database and running
  process retain ownership of that instance.
  See `test_2m_4s` in [the ubus example](examples/uvmsc/integrated/ubus/test_lib.h).

  ```cpp
  auto seq = my_seq::type_id::create_handle("seq");
  seq->num_transactions = 20; // example user-defined setting
  uvm::uvm_config_db<uvm::uvm_handle<uvm::uvm_sequence_base>>::set(this,
    "env.agent.sequencer.run_phase", "default_sequence", seq);
  ```

  If no instance-specific setup is needed, continue passing the sequence type
  as before; the sequencer creates the instance automatically:

  ```cpp
  uvm::uvm_config_db<uvm::uvm_object_wrapper*>::set(this,
    "env.agent.sequencer.run_phase", "default_sequence", my_seq::type_id::get());
  ```
  If both are configured, a non-empty instance handle takes precedence.

- **Drivers:** use `auto req = seq_item_port->get_next_item()` and pass response
  handles to `put_response()`.
- **Monitors and subscribers:** use `uvm_analysis_port<uvm_handle<T>>` and
  `uvm_subscriber<uvm_handle<T>>`, with `void write(const uvm_handle<T>&) override`.

Allocate a fresh transaction before reusing data that a queue or subscriber may
still retain. Use `uvm::make_handle<T>(...)` when factory overrides are not needed.

Use `dynamic_handle_cast<T>()` or `static_handle_cast<T>()`
when a handle conversion requires a cast.

### Method replacements

Sequence methods belong to `uvm_sequence_base`; driver operations are accessed
through `seq_item_port` or the sequencer. The old overloads below are removed.

| Where / operation | Previous API | Handle API / recommended change |
| --- | --- | --- |
| Item: copy IDs | `void set_id_info(uvm_sequence_item& item)` | `void set_id_info(uvm_handle<uvm_sequence_item> item)` |
| Sequence: start item | `void start_item(uvm_sequence_item* item, int set_priority = -1, uvm_sequencer_base* sequencer = nullptr)` | `void start_item(uvm_handle<uvm_sequence_item> item, int set_priority = -1, uvm_sequencer_base* sequencer = nullptr)` |
| Sequence: finish item | `void finish_item(uvm_sequence_item* item, int set_priority = -1)` | `void finish_item(uvm_handle<uvm_sequence_item> item, int set_priority = -1)` |
| Sequence: send request | `void send_request(uvm_sequence_item* request, bool rerandomize = false)` | `void send_request(uvm_handle<uvm_sequence_item> request, bool rerandomize = false)` |
| Driver: obtain request | `REQ get_next_item(REQ* req = nullptr)`; `void get_next_item(REQ& req)` | `uvm_handle<REQ> get_next_item()`; use `auto req = seq_item_port->get_next_item()` |
| Driver: try to obtain request | `bool try_next_item(REQ& req)` | `uvm_handle<REQ> try_next_item()`; an empty handle means no item |
| Driver: complete with response | `void item_done(const RSP& item, bool use_item = true)`; interface: `void item_done(const RSP& item)` | Call `item_done()`, then `put_response(rsp_handle)` if a response is needed; omit the latter when `use_item` was false |
| Driver: send response | `void put(const RSP& rsp)` | `void put_response(uvm_handle<RSP> rsp)` |
| Driver: obtain and complete request | `REQ get(REQ* req = nullptr)`; `void get(REQ& req)` | `uvm_handle<REQ> get()` |
| Driver: inspect request | `REQ peek(REQ* req = nullptr)`; `void peek(REQ& req)` | `uvm_handle<REQ> peek()` |

## Known bugs
The simple/registers/models/aliasing test is failing in case the TODO section
from its source file tb_test.h is uncommented. This test has been literally
copied from UVM-SystemVerilog. We doubt whether this test case passes in the
UVM-SystemVerilog environment, and if it does, whether it should. See [1].

[1]: https://forums.accellera.org/topic/7400-clarification-needed-on-mix-between-bit-bash-sequence-and-single-register-write/

## Bugs and Suggestions

Please report bugs and suggestions about this library to:

  https://github.com/accellera-official/uvm-systemc


## Library status

This library is the reference implementation of the UVM SystemC standard. 
In addition to the functionality described in the UVM SystemC LRM it contains 
additional features (e.g. the UVM Register Adaption Layer), whose API is not 
stable and might change without notice.
