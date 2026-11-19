//----------------------------------------------------------------------
//   Copyright 2007-2010 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc. 
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2016 NXP B.V.
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

#ifndef UVM_REPORT_MESSAGE_H_
#define UVM_REPORT_MESSAGE_H_

#include <systemc>
#include <string>

#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/macros/uvm_object_defines.h"
#include "uvmsc/factory/uvm_object_registry.h"

namespace uvm {

// forward class declarations
class uvm_report_server;
class uvm_report_handler;
class uvm_report_object;
class uvm_root;
class uvm_object;

//----------------------------------------------------------------------
// Class: uvm_report_message_element_base
//
// Base class for report message element. Defines common interface.
//----------------------------------------------------------------------

class uvm_report_message_element_base
{
 public:
  virtual const std::string get_name() const;

  virtual void set_name( const std::string& name );

  virtual uvm_action get_action() const;

  virtual void set_action( uvm_action action );

  void print( uvm_printer* printer ) const;
  virtual void do_print( const uvm_printer& printer ) const = 0;

  void record( uvm_recorder* recorder );
  virtual void do_record( const uvm_recorder& recorder ) = 0;

  void copy( const uvm_report_message_element_base& rhs );
  virtual void do_copy( const uvm_report_message_element_base& rhs ) = 0;

  uvm_report_message_element_base* clone();
  virtual uvm_report_message_element_base* do_clone() = 0;

  uvm_action m_action;
  std::string m_name;

 protected:
   ~uvm_report_message_element_base();

}; // class uvm_report_message_element_base


//----------------------------------------------------------------------
// Class: uvm_report_message_int_element
//
// Message element class for integral type
//----------------------------------------------------------------------

class uvm_report_message_int_element
: public uvm_report_message_element_base
{
 public:
  typedef uvm_report_message_int_element this_type;

  virtual uvm_bitstream_t get_value( int& size, uvm_radix_enum& radix ) const;

  virtual void set_value( uvm_bitstream_t value,
                          int size,
                          uvm_radix_enum radix);

  void do_print( const uvm_printer& printer ) const override;
  void do_record( const uvm_recorder& recorder ) override;
  void do_copy( const uvm_report_message_element_base& rhs ) override;
  uvm_report_message_element_base* do_clone() override;

 private:
  ~uvm_report_message_int_element();

  uvm_bitstream_t m_val;
  int             m_size;
  uvm_radix_enum  m_radix;

}; //class uvm_report_message_int_element


//----------------------------------------------------------------------
// Class: uvm_report_message_string_element
//
// Message element class for string type
//----------------------------------------------------------------------

class uvm_report_message_string_element
: public uvm_report_message_element_base
{
 public:
  typedef uvm_report_message_string_element this_type;

  virtual const std::string get_value() const;

  virtual void set_value( const std::string& value );

  void do_print( const uvm_printer& printer ) const override;

  void do_record( const uvm_recorder& recorder ) override;

  void do_copy( const uvm_report_message_element_base& rhs ) override;

  uvm_report_message_element_base* do_clone() override;

private:
  ~uvm_report_message_string_element();

  std::string  m_val;

}; // class uvm_report_message_string_element


//----------------------------------------------------------------------
// Class: uvm_report_message_object_element
//
// Message element class for object type
//----------------------------------------------------------------------

class uvm_report_message_object_element
: public uvm_report_message_element_base
{
public:
  typedef uvm_report_message_object_element this_type;

  virtual uvm_object* get_value() const;

  virtual void set_value( uvm_object* value );

  void do_print( const uvm_printer& printer ) const override;

  void do_record( const uvm_recorder& recorder ) override;

  void do_copy( const uvm_report_message_element_base& rhs ) override;

  uvm_report_message_element_base* do_clone() override;

private:
  ~uvm_report_message_object_element();
  uvm_object* m_obj;

}; // class uvm_report_message_object_element


//----------------------------------------------------------------------
// Class: uvm_report_message_element_container
//
// A container used by report message to contain the dynamically added elements,
// with APIs to add and delete the elements.
//----------------------------------------------------------------------

class uvm_report_message_element_container : public uvm_object
{
 public:
  typedef std::vector<uvm_report_message_element_base*> queue_of_element;

  UVM_OBJECT_UTILS(uvm_report_message_element_container);

  uvm_report_message_element_container( uvm_object_name name = "element_container" );

  virtual unsigned int size() const;

  virtual void do_delete(int index); // was delete

  virtual void delete_elements();

  virtual queue_of_element get_elements() const;

  virtual void add_int( const std::string& name,
                        uvm_bitstream_t value,
                        int size, uvm_radix_enum radix,
                        uvm_action action = (UVM_LOG | UVM_RM_RECORD) );

  virtual void add_string( const std::string& name,
                           const std::string& value,
                           uvm_action action = (UVM_LOG | UVM_RM_RECORD) );

  virtual void add_object( const std::string& name,
                           uvm_object* obj,
                           uvm_action action = (UVM_LOG | UVM_RM_RECORD) );

  void do_print( const uvm_printer& printer ) const override;

  void do_record( const uvm_recorder& recorder ) override;

  void do_copy( const uvm_object& rhs ) override;

 private:
   std::vector<uvm_report_message_element_base*> m_elements;

};


//----------------------------------------------------------------------
// Class: uvm_report_message
//
// The uvm_report_message is the basic UVM object message class.  It provides 
// the fields that are common to all messages.  It also has a message element 
// container and provides the APIs necessary to add integral types, strings and
// uvm_objects to the container. The report message object can be initialized
// with the common fields, and passes through the whole reporting system (i.e. 
// report object, report handler, report server, report catcher, etc) as an
// object. The additional elements can be added/deleted to/from the message 
// object anywhere in the reporting system, and can be printed or recorded
// along with the common fields.
//----------------------------------------------------------------------

class uvm_report_message : public uvm_object
{
public:

  uvm_report_message(const std::string& name = "uvm_report_message");

  static uvm_report_message* new_report_message(const std::string& name = "uvm_report_message");

  void do_print( const uvm_printer& printer ) const override;

  UVM_OBJECT_UTILS(uvm_report_message);

  // do_pack() not needed
  // do_unpack() not needed
  // do_compare() not needed

  void do_copy( const uvm_object& rhs ) override;

  //----------------------------------------------------------------------
  // Group: Infrastructure References
  //----------------------------------------------------------------------

  virtual uvm_report_object* get_report_object() const;

  virtual void set_report_object( uvm_report_object* ro );

  virtual uvm_report_handler* get_report_handler() const;

  virtual void set_report_handler( uvm_report_handler* rh );

  virtual uvm_report_server* get_report_server() const;
  
  virtual void set_report_server( uvm_report_server* rs );

  //----------------------------------------------------------------------
  // Group: Message Fields
  //----------------------------------------------------------------------

  virtual uvm_severity get_severity() const;

  virtual void set_severity( uvm_severity sev );

  virtual const std::string get_id() const;

  virtual void set_id( const std::string& id );

  virtual const std::string get_message() const;

  virtual void set_message( const std::string& msg );

  virtual int get_verbosity() const;

  virtual void set_verbosity( int ver );

  virtual const std::string get_filename() const;

  virtual void set_filename( const std::string& fname );

  virtual int get_line() const;

  virtual void set_line(int ln);

  virtual const std::string get_context() const;

  virtual void set_context( const std::string& cn );

  virtual uvm_action get_action() const;

  virtual void set_action( uvm_action act );

  virtual UVM_FILE get_file() const;

  virtual void set_file( UVM_FILE fl );

  virtual uvm_report_message_element_container* get_element_container() const;

  virtual void set_report_message( uvm_severity severity,
                                   const std::string& id,
                                   const std::string& message,
                                   int verbosity,
                                   const std::string& filename,
                                   int line,
                                   const std::string& context_name );

  //----------------------------------------------------------------------------
  // Group-  Message Recording
  //----------------------------------------------------------------------------

  // Implementation defined
  virtual void m_record_message( const uvm_recorder& recorder );

  // Implementation defined
  virtual void m_record_core_properties( const uvm_recorder& recorder );

  // Implementation defined
  void do_record( const uvm_recorder& recorder ) override;

  //----------------------------------------------------------------------------
  // Group: Message Element APIs
  //----------------------------------------------------------------------------

  virtual void add_int( const std::string& name,
                        uvm_bitstream_t value,
                        int size,
                        uvm_radix_enum radix,
                        uvm_action action = (UVM_LOG | UVM_RM_RECORD) );

  virtual void add_string( const std::string& name,
                           const std::string& value,
                           uvm_action action = (UVM_LOG | UVM_RM_RECORD) );

  virtual void add_object( const std::string& name,
                           uvm_object* obj,
                           uvm_action action = (UVM_LOG | UVM_RM_RECORD) );

private:
  uvm_report_object* m_report_object;
  uvm_report_handler* m_report_handler;
  uvm_report_server* m_report_server;

  uvm_severity m_severity;
  std::string m_id;
  std::string m_message;
  int m_verbosity;
  std::string m_filename;
  int m_line;
  std::string m_context_name;
  uvm_action m_action;
  UVM_FILE m_file;

  // Implementation defined
  uvm_report_message_element_container* m_report_message_element_container;

}; // class

} // namespace uvm

#endif // UVM_REPORT_MESSAGE_H_
