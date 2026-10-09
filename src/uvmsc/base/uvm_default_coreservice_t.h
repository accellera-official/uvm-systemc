//----------------------------------------------------------------------
//   Copyright 2013 Cadence Design Inc
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

#ifndef UVM_DEFAULT_CORESERVICE_T_
#define UVM_DEFAULT_CORESERVICE_T_

#include <array>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>
#include <systemc>

#include "uvmsc/base/uvm_coreservice_t.h"

namespace uvm {

// forward declaration of necessary classes.
class uvm_factory;
class uvm_report_server;
class uvm_root;
class uvm_report_message;
class uvm_packer;
class uvm_comparer;
class uvm_status_container;
class uvm_resource_pool;
class uvm_reg_map;
class uvm_domain;
class uvm_build_phase;
class uvm_connect_phase;
class uvm_end_of_elaboration_phase;
class uvm_start_of_simulation_phase;
class uvm_run_phase;
class uvm_extract_phase;
class uvm_check_phase;
class uvm_report_phase;
class uvm_final_phase;
class uvm_pre_reset_phase;
class uvm_reset_phase;
class uvm_post_reset_phase;
class uvm_pre_configure_phase;
class uvm_configure_phase;
class uvm_post_configure_phase;
class uvm_pre_main_phase;
class uvm_main_phase;
class uvm_post_main_phase;
class uvm_pre_shutdown_phase;
class uvm_shutdown_phase;
class uvm_post_shutdown_phase;
class uvm_printer;
class uvm_table_printer;
class uvm_tree_printer;
class uvm_line_printer;
template <typename T> class uvm_phase_queue;
class uvm_phase;
template <typename OBJ, typename CB> class uvm_callback_iter;
class uvm_report_object;
class uvm_report_catcher;
typedef uvm_callback_iter<uvm_report_object, uvm_report_catcher> uvm_report_cb_iter;
class uvm_callback;
class uvm_typeid_base;
class uvm_callbacks_base;
class uvm_objection;
class uvm_objection_context_object;
class uvm_reg_block;
class uvm_reg_read_only_cbs;
class uvm_reg_write_only_cbs;
class uvm_report_catcher_data;
//class uvm_visitor;
class uvm_component;
class uvm_tr_database;
class uvm_object;
template <typename T> class uvm_queue;

//----------------------------------------------------------------------
// Helper types for typed store
//----------------------------------------------------------------------

struct uvm_coreservice_store_base
{
  virtual ~uvm_coreservice_store_base() {}
  virtual const std::type_info& type() const = 0;
};

template <typename T>
struct uvm_coreservice_store : public uvm_coreservice_store_base
{
  template <typename... Args>
  explicit uvm_coreservice_store( Args&&... args )
    : value(std::forward<Args>(args)...)
  {
  }

  virtual const std::type_info& type() const
  {
    return typeid(T);
  }

  T value;
};

//----------------------------------------------------------------------
// Class: uvm_default_coreservice_t
//
// uvm_default_coreservice_t provides a default implementation of the 
// uvm_coreservice_t API. It instantiates uvm_default_factory, uvm_default_report_server, 
// uvm_root.
//----------------------------------------------------------------------

class uvm_default_coreservice_t : public uvm_coreservice_t
{
 public:
  friend class uvm_coreservice_t;

  // Type aliases for implementation-defined storage
  typedef std::map<uvm_object*, uvm_queue<uvm_callback*>*> callback_pool_map_t;
  typedef std::map<std::string, uvm_domain*> domain_map_t;
  typedef std::vector<uvm_objection*> objection_list_t;
  typedef std::vector<uvm_objection_context_object*> objection_context_list_t;
  typedef std::map<uvm_phase*, bool> phase_execution_map_t;
  typedef std::map<std::string, uvm_phase*> phase_node_map_t;
  typedef std::map<uvm_reg_block*, bool> reg_block_roots_map_t;
  typedef std::map<int, bool> recorder_handle_map_t;
  typedef std::map<std::string, bool> reg_field_policy_map_t;
  typedef std::map<uvm_typeid_base*, uvm_callbacks_base*> typeid_map_t;
  typedef std::map<uvm_callbacks_base*, uvm_typeid_base*> type_map_t;
  typedef std::array<char, 2048> regex_buffer_t;
  typedef std::map<std::string, std::unique_ptr<uvm_coreservice_store_base> > typed_store_map_t;

  // Standard API overrides
  std::shared_ptr<uvm_factory> get_factory() override;
  void set_factory( std::shared_ptr<uvm_factory> f ) override;

//  virtual uvm_tr_database* get_default_tr_database() const;
//  virtual void set_default_tr_database( uvm_tr_database* db );

  std::shared_ptr<uvm_report_server> get_report_server() override;
  void set_report_server( std::shared_ptr<uvm_report_server> server ) override;

  uvm_root* get_root() override;
  typed_store_map_t& get_typed_store();

  template <typename T, typename... Args>
  T& get_or_create_typed_store( const std::string& key, Args&&... args )
  {
    typed_store_map_t& store = get_typed_store();
    typed_store_map_t::iterator it = store.find(key);

    if (it == store.end())
    {
      std::unique_ptr<uvm_coreservice_store_base> slot(
        new uvm_coreservice_store<T>(std::forward<Args>(args)...));
      it = store.insert(std::make_pair(key, std::move(slot))).first;
    }

    if (it->second->type() != typeid(T))
      throw std::logic_error("uvm_default_coreservice_t typed store key reused with different type");

    return static_cast<uvm_coreservice_store<T>*>(it->second.get())->value;
  }

  // Standard API overrides (packer, comparer, printer)
  std::shared_ptr<uvm_packer> get_default_packer() override;
  void set_default_packer(std::shared_ptr<uvm_packer>) override;

  std::shared_ptr<uvm_comparer> get_default_comparer() override;
  void set_default_comparer(std::shared_ptr<uvm_comparer>) override;

  std::shared_ptr<uvm_printer> get_default_printer() override;
  void set_default_printer(std::shared_ptr<uvm_printer>) override;

  // uvm_object: m_inst_count
  int allocate_uvm_object_m_inst_count();
  int get_uvm_object_m_inst_count();

  // uvm_event: g_cnt
  int& get_uvm_event_g_cnt();

  // uvm_resource_base: default_precedence
  unsigned int& get_uvm_resource_base_default_precedence();

  // uvm_default_factory: m_debug_pass
  bool& get_uvm_default_factory_m_debug_pass();

  // uvm_recorder: handle counter and m_handles map
  int& get_uvm_recorder_handle();
  recorder_handle_map_t& get_uvm_recorder_m_handles();

  // uvm_sequencer_base: g_sequencer_id, g_sequence_id, g_request_id
  int& get_uvm_sequencer_base_g_sequencer_id();
  int& get_uvm_sequencer_base_g_sequence_id();
  int& get_uvm_sequencer_base_g_request_id();

  // uvm_reg: m_max_size
  unsigned int& get_uvm_reg_m_max_size();

  // uvm_mem: m_max_size
  unsigned int& get_uvm_mem_m_max_size();

  // uvm_reg_field: m_predefined, m_max_size, m_policy_names
  bool& get_uvm_reg_field_m_predefined();
  unsigned int& get_uvm_reg_field_m_max_size();
  reg_field_policy_map_t& get_uvm_reg_field_m_policy_names();

  // uvm_reg_block: id counter and m_roots map
  int& get_uvm_reg_block_id();
  reg_block_roots_map_t& get_uvm_reg_block_m_roots();

  // uvm_typeid_base: typeid_map and type_map
  typeid_map_t& get_uvm_typeid_base_typeid_map();
  type_map_t& get_uvm_typeid_base_type_map();

  // uvm_reg_read_only_cbs: m_me singleton
  uvm_reg_read_only_cbs* get_uvm_reg_read_only_cbs_m_me();

  // uvm_reg_write_only_cbs: m_me singleton
  uvm_reg_write_only_cbs* get_uvm_reg_write_only_cbs_m_me();

  // uvm_report_catcher_data: singleton instance
  uvm_report_catcher_data& get_uvm_report_catcher_data_instance();

  // uvm_globals: uvm_re regex scratch buffer
  regex_buffer_t& get_uvm_globals_uvm_re();

  // uvm_object: __m_uvm_status_container
  uvm_status_container* get_uvm_object__m_uvm_status_container();

  // uvm_resource_pool: rp singleton, m_has_wildcard_names
  uvm_resource_pool* get_uvm_resource_pool_rp();
  void reset_uvm_resource_pool_rp();
  bool& get_uvm_resource_pool_m_has_wildcard_names();

  // uvm_printer_globals: uvm_default_table_printer, uvm_default_tree_printer, uvm_default_line_printer
  uvm_table_printer* get_uvm_default_table_printer();
  void set_uvm_default_table_printer( uvm_table_printer* printer );
  uvm_tree_printer* get_uvm_default_tree_printer();
  void set_uvm_default_tree_printer( uvm_tree_printer* printer );
  uvm_line_printer* get_uvm_default_line_printer();
  void set_uvm_default_line_printer( uvm_line_printer* printer );

  // uvm_phase: m_phase_hopper, m_phase_trace
  uvm_phase_queue<uvm_phase*>* get_uvm_phase_m_phase_hopper();
  bool& get_uvm_phase_m_phase_trace();

  // uvm_root: m_uvm_timeout_overridable
  bool& get_uvm_root_m_uvm_timeout_overridable();

  // uvm_component: global_timeout_spawned_, _print_config_matches
  bool& get_uvm_component_global_timeout_spawned_();
  bool& get_uvm_component__print_config_matches();

  // uvm_resource_options: auditing
  bool& get_uvm_resource_options_auditing();

  // uvm_resource_db_options: ready, tracing
  bool& get_uvm_resource_db_options_ready();
  bool& get_uvm_resource_db_options_tracing();

  // uvm_config_db_options: ready, tracing
  bool& get_uvm_config_db_options_ready();
  bool& get_uvm_config_db_options_tracing();

  // uvm_root: m_uvm_header_done
  bool& get_uvm_root_m_uvm_header_done();

  // uvm_report_catcher: in_catcher
  bool& get_uvm_report_catcher_in_catcher();

  // uvm_callbacks_base: m_tracing
  bool& get_uvm_callbacks_base_m_tracing();

  // uvm_reg_map: m_backdoor singleton
  uvm_reg_map* get_uvm_reg_map_m_backdoor();

  // uvm_domain: m_uvm_schedule, m_common_domain, m_uvm_domain
  uvm_phase* get_uvm_domain_m_uvm_schedule();
  uvm_domain* get_uvm_domain_m_common_domain();
  void set_uvm_domain_m_common_domain( uvm_domain* domain );
  uvm_domain* get_uvm_domain_m_uvm_domain();
  void set_uvm_domain_m_uvm_domain( uvm_domain* domain );

  // uvm_build_phase: m_inst singleton
  uvm_build_phase* get_uvm_build_phase_m_inst();

  // uvm_connect_phase: m_inst singleton
  uvm_connect_phase* get_uvm_connect_phase_m_inst();

  // uvm_end_of_elaboration_phase: m_inst singleton
  uvm_end_of_elaboration_phase* get_uvm_end_of_elaboration_phase_m_inst();

  // uvm_start_of_simulation_phase: m_inst singleton
  uvm_start_of_simulation_phase* get_uvm_start_of_simulation_phase_m_inst();

  // uvm_run_phase: m_inst singleton
  uvm_run_phase* get_uvm_run_phase_m_inst();

  // uvm_extract_phase: m_inst singleton
  uvm_extract_phase* get_uvm_extract_phase_m_inst();

  // uvm_check_phase: m_inst singleton
  uvm_check_phase* get_uvm_check_phase_m_inst();

  // uvm_report_phase: m_inst singleton
  uvm_report_phase* get_uvm_report_phase_m_inst();

  // uvm_final_phase: m_inst singleton
  uvm_final_phase* get_uvm_final_phase_m_inst();

  // uvm_pre_reset_phase: m_inst singleton
  uvm_pre_reset_phase* get_uvm_pre_reset_phase_m_inst();

  // uvm_reset_phase: m_inst singleton
  uvm_reset_phase* get_uvm_reset_phase_m_inst();

  // uvm_post_reset_phase: m_inst singleton
  uvm_post_reset_phase* get_uvm_post_reset_phase_m_inst();

  // uvm_pre_configure_phase: m_inst singleton
  uvm_pre_configure_phase* get_uvm_pre_configure_phase_m_inst();

  // uvm_configure_phase: m_inst singleton
  uvm_configure_phase* get_uvm_configure_phase_m_inst();

  // uvm_post_configure_phase: m_inst singleton
  uvm_post_configure_phase* get_uvm_post_configure_phase_m_inst();

  // uvm_pre_main_phase: m_inst singleton
  uvm_pre_main_phase* get_uvm_pre_main_phase_m_inst();

  // uvm_main_phase: m_inst singleton
  uvm_main_phase* get_uvm_main_phase_m_inst();

  // uvm_post_main_phase: m_inst singleton
  uvm_post_main_phase* get_uvm_post_main_phase_m_inst();

  // uvm_pre_shutdown_phase: m_inst singleton
  uvm_pre_shutdown_phase* get_uvm_pre_shutdown_phase_m_inst();

  // uvm_shutdown_phase: m_inst singleton
  uvm_shutdown_phase* get_uvm_shutdown_phase_m_inst();

  // uvm_post_shutdown_phase: m_inst singleton
  uvm_post_shutdown_phase* get_uvm_post_shutdown_phase_m_inst();

  // uvm_report_message: function-local l_report_message static
  uvm_report_message* get_uvm_report_message_l_report_message( const std::string& name );

  // uvm_report_catcher: name_iter, print_iter
  uvm_report_cb_iter* get_uvm_report_catcher_name_iter();
  uvm_report_cb_iter* get_uvm_report_catcher_print_iter();

  // uvm_callbacks_base: m_pool, m_b_inst
  callback_pool_map_t& get_uvm_callbacks_base_m_pool();
  uvm_callbacks_base* get_uvm_callbacks_base_m_b_inst();

  // uvm_domain: domains map
  domain_map_t& get_uvm_domain_domains();

  // uvm_objection: objections list, scheduled_list, scheduled_list_changed event
  objection_list_t& get_uvm_objection_objections();
  objection_context_list_t& get_uvm_objection_scheduled_list();
  sc_core::sc_event& get_uvm_objection_scheduled_list_changed();

  // uvm_phase: executing_phases map, phase_nodes map
  phase_execution_map_t& get_uvm_phase_executing_phases();
  phase_node_map_t& get_uvm_phase_phase_nodes();

//  virtual void set_component_visitor( uvm_visitor<uvm_component> v );
//  virtual uvm_visitor<uvm_component> get_component_visitor() const;

 private:
  uvm_default_coreservice_t();
  ~uvm_default_coreservice_t();

  template <typename ArrayT>
  const char* lookup_name( const ArrayT& names, int index ) const
  {
    return (index >= 0 && static_cast<std::size_t>(index) < names.size())
      ? names[static_cast<std::size_t>(index)]
      : "<unknown>";
  }

  std::shared_ptr<uvm_factory> factory;
//  mutable uvm_tr_database* tr_database;
  std::shared_ptr<uvm_report_server> report_server;
  int inst_count;
  int event_count;
  unsigned int resource_base_default_precedence;
  bool default_factory_debug_pass;
  int recorder_handle_counter;
  recorder_handle_map_t recorder_handles;
  int sequencer_id_counter;
  int sequence_id_counter;
  int request_id_counter;
  unsigned int reg_max_size;
  unsigned int mem_max_size;
  bool reg_field_predefined;
  unsigned int reg_field_max_size;
  reg_field_policy_map_t reg_field_policy_names;
  int reg_block_id_counter;
  reg_block_roots_map_t reg_block_roots;
  typeid_map_t typeid_to_callback_map;
  type_map_t callback_to_typeid_map;
  std::shared_ptr<uvm_packer> default_packer;
  std::shared_ptr<uvm_comparer> default_comparer;
  std::unique_ptr<uvm_status_container> status_container;
  std::unique_ptr<uvm_resource_pool> resource_pool;
  uvm_table_printer* default_table_printer;
  uvm_tree_printer* default_tree_printer;
  uvm_line_printer* default_line_printer;
  std::unique_ptr<uvm_phase_queue<uvm_phase*> > phase_hopper;
  std::unique_ptr<uvm_reg_map> reg_map_backdoor;
  std::unique_ptr<uvm_phase> uvm_schedule;
  std::unique_ptr<uvm_root> root_ptr;
  std::unique_ptr<uvm_report_message> shared_report_message;
  std::unique_ptr<uvm_report_cb_iter> report_catcher_name_iter;
  std::unique_ptr<uvm_report_cb_iter> report_catcher_print_iter;
  std::unique_ptr<callback_pool_map_t> callback_pool;
  std::unique_ptr<uvm_callbacks_base> callbacks_base;
  std::unique_ptr<uvm_reg_read_only_cbs> reg_read_only_cbs;
  std::unique_ptr<uvm_reg_write_only_cbs> reg_write_only_cbs;
  std::unique_ptr<uvm_report_catcher_data> report_catcher_data;
  std::unique_ptr<uvm_build_phase> build_phase_ptr;
  std::unique_ptr<uvm_connect_phase> connect_phase_ptr;
  std::unique_ptr<uvm_end_of_elaboration_phase> end_of_elaboration_phase_ptr;
  std::unique_ptr<uvm_start_of_simulation_phase> start_of_simulation_phase_ptr;
  std::unique_ptr<uvm_run_phase> run_phase_ptr;
  std::unique_ptr<uvm_extract_phase> extract_phase_ptr;
  std::unique_ptr<uvm_check_phase> check_phase_ptr;
  std::unique_ptr<uvm_report_phase> report_phase_ptr;
  std::unique_ptr<uvm_final_phase> final_phase_ptr;
  std::unique_ptr<uvm_pre_reset_phase> pre_reset_phase_ptr;
  std::unique_ptr<uvm_reset_phase> reset_phase_ptr;
  std::unique_ptr<uvm_post_reset_phase> post_reset_phase_ptr;
  std::unique_ptr<uvm_pre_configure_phase> pre_configure_phase_ptr;
  std::unique_ptr<uvm_configure_phase> configure_phase_ptr;
  std::unique_ptr<uvm_post_configure_phase> post_configure_phase_ptr;
  std::unique_ptr<uvm_pre_main_phase> pre_main_phase_ptr;
  std::unique_ptr<uvm_main_phase> main_phase_ptr;
  std::unique_ptr<uvm_post_main_phase> post_main_phase_ptr;
  std::unique_ptr<uvm_pre_shutdown_phase> pre_shutdown_phase_ptr;
  std::unique_ptr<uvm_shutdown_phase> shutdown_phase_ptr;
  std::unique_ptr<uvm_post_shutdown_phase> post_shutdown_phase_ptr;
  std::shared_ptr<uvm_printer> default_printer;
  uvm_domain* common_domain;
  uvm_domain* uvm_domain_ptr;
  bool resource_pool_has_wildcard_names;
  bool phase_trace_enabled;
  bool root_timeout_overridable;
  bool component_global_timeout_spawned;
  bool component_print_config_matches;
  bool resource_options_auditing;
  bool resource_db_options_ready;
  bool resource_db_options_tracing;
  bool config_db_options_ready;
  bool config_db_options_tracing;
  bool root_header_printed;
  bool report_catcher_in_catcher;
  bool callbacks_tracing;
  domain_map_t domains;
  objection_list_t objections;
  objection_context_list_t scheduled_objection_contexts;
  sc_core::sc_event scheduled_objection_list_changed;
  phase_execution_map_t executing_phases;
  phase_node_map_t phase_nodes;
  regex_buffer_t regex_buffer;
  typed_store_map_t typed_store;
//  mutable uvm_visitor<uvm_component> m_visitor;

}; // class uvm_default_coreservice_t

} // namespace uvm

#endif // UVM_DEFAULT_CORESERVICE_T_
