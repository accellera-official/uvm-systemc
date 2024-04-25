//----------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2004-2009 Synopsys, Inc.
//   Copyright 2010-2011 Mentor Graphics Corporation
//   Copyright 2010 Cadence Design Systems, Inc.
//   Copyright 2014 Université Pierre et Marie Curie, Paris
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

#ifndef UVM_REG_MODEL_H_
#define UVM_REG_MODEL_H_

#include <systemc>

#include "uvmsc/macros/uvm_reg_defines.h"
#include "uvmsc/conf/uvm_resource_db.h"
#include "uvmsc/reg/uvm_hdl_path_concat.h"

namespace uvm {


//----------------------------------------------------------------------
// Group: typedefs
//----------------------------------------------------------------------

//
// TODO define correct typedefs. Note: make sure the necessary operators
// &, >, >> work as well
//

//----------------------------------------------------------------------
// Typedef: uvm_reg_data_t
//
//! 2-state data value with <`UVM_REG_DATA_WIDTH> bits
//!
//! Depending on the size of UVM_REG_DATA_WIDTH, the appropriate
//! SystemC data type is selected
//----------------------------------------------------------------------

template<bool isSmallReg>
struct uvm_reg_data_t_select;

template<>
struct uvm_reg_data_t_select<true> // registers always smaller than 64-bit
{
  typedef sc_dt::sc_uint<UVM_REG_DATA_WIDTH> type;
};

template<>
struct uvm_reg_data_t_select<false> // arbitrary sized registers
{
  typedef sc_dt::sc_biguint<UVM_REG_DATA_WIDTH> type;
};

typedef uvm_reg_data_t_select<((UVM_REG_DATA_WIDTH)<=64)>::type uvm_reg_data_t ;


//----------------------------------------------------------------------
// Typedef: uvm_reg_data_logic_t
//
//! 4-state data value with <`UVM_REG_DATA_WIDTH> bits
//----------------------------------------------------------------------

typedef sc_dt::sc_lv<UVM_REG_DATA_WIDTH> uvm_reg_data_logic_t ;

//----------------------------------------------------------------------
// Typedef: uvm_reg_addr_t
//
//! 2-state address value with <`UVM_REG_ADDR_WIDTH> bits
//----------------------------------------------------------------------

//typedef sc_dt::sc_bv<UVM_REG_ADDR_WIDTH> uvm_reg_addr_t ;
typedef sc_dt::sc_uint<UVM_REG_ADDR_WIDTH> uvm_reg_addr_t;

//----------------------------------------------------------------------
// Typedef: uvm_reg_addr_logic_t
//
//! 4-state address value with <`UVM_REG_ADDR_WIDTH> bits
//----------------------------------------------------------------------

typedef sc_dt::sc_lv<UVM_REG_ADDR_WIDTH> uvm_reg_addr_logic_t ;


//----------------------------------------------------------------------
// Typedef: uvm_reg_byte_en_t
//
//! 2-state byte_enable value with <`UVM_REG_BYTENABLE_WIDTH> bits
//----------------------------------------------------------------------

typedef sc_dt::sc_bv<UVM_REG_BYTENABLE_WIDTH> uvm_reg_byte_en_t ;


//----------------------------------------------------------------------
// Typedef: uvm_reg_cvr_t
//
//! Coverage model value set with #UVM_REG_CVR_WIDTH bits.
//!
//! Symbolic values for individual coverage models are defined
//! by the #uvm_coverage_model_e type.
//!
//! The following bits in the set are assigned as follows
//!
//! 0-7     - UVM pre-defined coverage models
//! 8-15    - Coverage models defined by EDA vendors,
//!           implemented in a register model generator.
//! 16-23   - User-defined coverage models
//! 24..    - Reserved
//----------------------------------------------------------------------

//typedef sc_dt::sc_bv<UVM_REG_CVR_WIDTH> uvm_reg_cvr_t ;
typedef int uvm_reg_cvr_t;


typedef uvm_resource_db<uvm_reg_cvr_t> uvm_reg_cvr_rsrc_db;



//----------------------------------------------------------------------
// Group: Enumerations
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Enum: uvm_status_e
//
//! Return status for register operations
//!
//! UVM_IS_OK      - Operation completed successfully
//! UVM_NOT_OK     - Operation completed with error
//! UVM_HAS_X      - Operation completed successfully bit had unknown bits.
//----------------------------------------------------------------------

typedef enum {
  UVM_IS_OK,
  UVM_NOT_OK,
  UVM_HAS_X
} uvm_status_e;

extern const char *uvm_status_name[];

//----------------------------------------------------------------------
// Enum: uvm_path_e
//
//! Path used for register operation
//!
//! UVM_FRONTDOOR    - Use the front door
//! UVM_BACKDOOR     - Use the back door
//! UVM_PREDICT      - Operation derived from observations by a bus monitor via
//!                    the class #uvm_reg_predictor.
//! UVM_DEFAULT_PATH - Operation specified by the context
//----------------------------------------------------------------------

typedef enum {
  UVM_FRONTDOOR,
  UVM_BACKDOOR,
  UVM_PREDICT,
  UVM_DEFAULT_PATH
} uvm_path_e;

extern const char *uvm_path_name[];

//----------------------------------------------------------------------
// Enum: uvm_check_e
//
//! Read-only or read-and-check
//!
//! UVM_NO_CHECK   - Read only
//! UVM_CHECK      - Read and check
//----------------------------------------------------------------------

typedef enum {
  UVM_NO_CHECK,
  UVM_CHECK
} uvm_check_e;

extern const char* uvm_check_name[];

//----------------------------------------------------------------------
// Enum: uvm_endianness_e
//
//! Specifies byte ordering
//!
//! UVM_NO_ENDIAN      - Byte ordering not applicable
//! UVM_LITTLE_ENDIAN  - Least-significant bytes first in consecutive addresses
//! UVM_BIG_ENDIAN     - Most-significant bytes first in consecutive addresses
//! UVM_LITTLE_FIFO    - Least-significant bytes first at the same address
//! UVM_BIG_FIFO       - Most-significant bytes first at the same address
//----------------------------------------------------------------------

typedef enum {
  UVM_NO_ENDIAN,
  UVM_LITTLE_ENDIAN,
  UVM_BIG_ENDIAN,
  UVM_LITTLE_FIFO,
  UVM_BIG_FIFO
} uvm_endianness_e;

extern const char* uvm_endianness_name[];

//----------------------------------------------------------------------
// Enum: uvm_elem_kind_e
//
//! Type of element being read or written
//!
//! UVM_REG      - Register
//! UVM_FIELD    - Field
//! UVM_MEM      - Memory location
//----------------------------------------------------------------------

typedef enum {
  UVM_REG,
  UVM_FIELD,
  UVM_MEM
} uvm_elem_kind_e;

extern const char* uvm_elem_kind_name[];

//----------------------------------------------------------------------
// Enum: uvm_access_e
//
//! Type of operation begin performed
//!
//! UVM_READ     - Read operation
//! UVM_WRITE    - Write operation
//----------------------------------------------------------------------

typedef enum {
  UVM_READ,
  UVM_WRITE,
  UVM_BURST_READ,
  UVM_BURST_WRITE
} uvm_access_e;

extern const char* uvm_access_name[];

//----------------------------------------------------------------------
// Enum: uvm_hier_e
//
//! Whether to provide the requested information from a hierarchical context.
//!
//! UVM_NO_HIER - Provide info from the local context
//! UVM_HIER    - Provide info based on the hierarchical context
//----------------------------------------------------------------------

typedef enum {
  UVM_NO_HIER,
  UVM_HIER
} uvm_hier_e;

extern const char* uvm_hier_name[];

//----------------------------------------------------------------------
// Enum: uvm_predict_e
//
//! How the mirror is to be updated
//!
//! UVM_PREDICT_DIRECT  - Predicted value is as-is
//! UVM_PREDICT_READ    - Predict based on the specified value having been read
//! UVM_PREDICT_WRITE   - Predict based on the specified value having been written
//----------------------------------------------------------------------

typedef enum {
  UVM_PREDICT_DIRECT,
  UVM_PREDICT_READ,
  UVM_PREDICT_WRITE
} uvm_predict_e;

extern const char* uvm_predict_name[];

//----------------------------------------------------------------------
// Enum: uvm_coverage_model_e
//
//! Coverage models available or desired.
//! Multiple models may be specified by bitwise OR'ing individual model identifiers.
//!
//! UVM_NO_COVERAGE      - None
//! UVM_CVR_REG_BITS     - Individual register bits
//! UVM_CVR_ADDR_MAP     - Individual register and memory addresses
//! UVM_CVR_FIELD_VALS   - Field values
//! UVM_CVR_ALL          - All coverage models
//----------------------------------------------------------------------

typedef enum  { // uvm_reg_cvr_t
  UVM_NO_COVERAGE      = 0x0000,
  UVM_CVR_REG_BITS     = 0x0001,
  UVM_CVR_ADDR_MAP     = 0x0002,
  UVM_CVR_FIELD_VALS   = 0x0004,
  UVM_CVR_ALL          = ~0x0000
} uvm_coverage_model_e;


//----------------------------------------------------------------------
// Enum: uvm_reg_mem_tests_e
//
//! Select which pre-defined test sequence to execute.
//!
//! Multiple test sequences may be selected by bitwise OR'ing their
//! respective symbolic values.
//!
//! UVM_DO_REG_HW_RESET      - Run #uvm_reg_hw_reset_seq
//! UVM_DO_REG_BIT_BASH      - Run #uvm_reg_bit_bash_seq
//! UVM_DO_REG_ACCESS        - Run #uvm_reg_access_seq
//! UVM_DO_MEM_ACCESS        - Run #uvm_mem_access_seq
//! UVM_DO_SHARED_ACCESS     - Run #uvm_reg_mem_shared_access_seq
//! UVM_DO_MEM_WALK          - Run #uvm_mem_walk_seq
//! UVM_DO_ALL_REG_MEM_TESTS - Run all of the above
//!
//! Test sequences, when selected, are executed in the
//! order in which they are specified above.
//----------------------------------------------------------------------

typedef enum  { // bit mask
  UVM_DO_REG_HW_RESET      = 0x0001,
  UVM_DO_REG_BIT_BASH      = 0x0002,
  UVM_DO_REG_ACCESS        = 0x0004,
  UVM_DO_MEM_ACCESS        = 0x0008,
  UVM_DO_SHARED_ACCESS     = 0x0010,
  UVM_DO_MEM_WALK          = 0x0020,
  UVM_DO_ALL_REG_MEM_TESTS = ~0x0000
} uvm_reg_mem_tests_e;

//----------------------------------------------------------------------
// Enum: uvm_reg_field_access_e
//
//# Pre-defined access policies for #uvm_reg_field
//----------------------------------------------------------------------

typedef enum {
  RO,    // Read Only
  RW,    // Read, Write
  RC,    // Read Clears All
  RS,    // Read Sets All
  WRC,   // Write,  Read Clears All
  WRS,   // Write, Read Sets All
  WC,    // Write Clears All
  WS,    // Write Sets All
  WSRC,  // Write Sets All, Read Clears All
  WCRS,  // Write Clears All, Read Sets All
  W1C,   // Write 1 to Clear
  W1S,   // Write 1 to Set
  W1T,   // Write 1 to Toggle
  W0C,   // Write 0 to Clear
  W0S,   // Write 0 to Set
  W0T,   // Write 0 to Toggle
  W1SRC, // Write 1 to Set, Read Clears All
  W1CRS, // Write 1 to Clear, Read Sets All
  W0SRC, // Write 0 to Set, Read Clears All
  W0CRS, // Write 0 to Clear, Read Sets All
  WO,    // Write Only
  WOC,   // Write Only, Clears All
  WOS,   // Write Only Sets All
  W1,    // Write Once
  WO1    // Write Only, Once
} uvm_reg_field_access_e;

typedef struct packed {
  uvm_reg_addr_t min;
  uvm_reg_addr_t max;
  unsigned int stride;

  packed()
  {
    min = 0;
    max = 0;
    stride = 0;
  }

} uvm_reg_map_addr_range;



// concat2string

std::string uvm_hdl_concat2string( uvm_hdl_path_concat concat );

///////////

/////////////////////////////////////////////////////////////////
// some other utilities functions not defined in UVM-SV
/////////////////////////////////////////////////////////////////

uvm_reg_data_t uvm_mask_size(unsigned int size);



} // namespace uvm

#endif // UVM_REG_MODEL_H_
