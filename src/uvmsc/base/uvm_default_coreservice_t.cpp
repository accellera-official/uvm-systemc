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

#include <cstddef>

#include "uvmsc/base/uvm_version.h"
#include "uvmsc/base/uvm_root.h"
#include "uvmsc/base/uvm_component.h"
#include "uvmsc/base/uvm_component_name.h"
#include "uvmsc/factory/uvm_factory.h"
#include "uvmsc/factory/uvm_default_factory.h"
#include "uvmsc/cb/uvm_callbacks_base.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"
#include "uvmsc/misc/uvm_status_container.h"
#include "uvmsc/conf/uvm_resource_pool.h"
#include "uvmsc/policy/uvm_packer.h"
#include "uvmsc/policy/uvm_comparer.h"
#include "uvmsc/print/uvm_printer.h"
#include "uvmsc/print/uvm_table_printer.h"
#include "uvmsc/print/uvm_tree_printer.h"
#include "uvmsc/print/uvm_line_printer.h"
#include "uvmsc/phasing/uvm_phase_queue.h"
#include "uvmsc/phasing/uvm_phase.h"
#include "uvmsc/phasing/uvm_domain.h"
#include "uvmsc/phasing/uvm_common_phases.h"
#include "uvmsc/phasing/uvm_runtime_phases.h"
#include "uvmsc/reg/uvm_reg_map.h"
#include "uvmsc/reg/uvm_reg_read_only_cbs.h"
#include "uvmsc/reg/uvm_reg_write_only_cbs.h"
#include "uvmsc/report/uvm_report_catcher.h"
#include "uvmsc/report/uvm_report_catcher_data.h"
#include "uvmsc/report/uvm_report_message.h"
#include "uvmsc/report/uvm_report_server.h"
#include "uvmsc/report/uvm_default_report_server.h"

namespace uvm {

uvm_default_coreservice_t::uvm_default_coreservice_t()
  : factory(nullptr),
    report_server(nullptr),
    inst_count(0),
    event_count(0),
    resource_base_default_precedence(1000u),
    default_factory_debug_pass(false),
    recorder_handle_counter(0),
    sequencer_id_counter(1),
    sequence_id_counter(1),
    request_id_counter(0),
    reg_max_size(0u),
    mem_max_size(0u),
    reg_field_predefined(false),
    reg_field_max_size(0u),
    reg_block_id_counter(0),
    default_packer(nullptr),
    default_comparer(nullptr),
    default_table_printer(nullptr),
    default_tree_printer(nullptr),
    default_line_printer(nullptr),
    default_printer(nullptr),
    common_domain(nullptr),
    uvm_domain_ptr(nullptr),
    resource_pool_has_wildcard_names(false),
    phase_trace_enabled(false),
    root_timeout_overridable(true),
    component_global_timeout_spawned(false),
    component_print_config_matches(false),
    resource_options_auditing(true),
    resource_db_options_ready(false),
    resource_db_options_tracing(false),
    config_db_options_ready(false),
    config_db_options_tracing(false),
    root_header_printed(false),
    report_catcher_in_catcher(false),
    callbacks_tracing(true),
    regex_buffer()
{
}

uvm_default_coreservice_t::~uvm_default_coreservice_t()
{
}

//----------------------------------------------------------------------------
// member function: get_factory (virtual)
//
// Returns the currently enabled uvm factory.
// When no factory has been set before, instantiates a uvm_default_factory
//----------------------------------------------------------------------------

std::shared_ptr<uvm_factory> uvm_default_coreservice_t::get_factory()
{
  if(factory == nullptr)
  {
    factory = std::shared_ptr<uvm_factory>(new uvm_default_factory());
  }
  return factory;
}

//----------------------------------------------------------------------------
// member function: set_factory (virtual)
//
// Sets the current uvm factory.
// Please note: it is up to the user to preserve the contents of the original
// factory or delegate calls to the original factory
//----------------------------------------------------------------------------

void uvm_default_coreservice_t::set_factory( std::shared_ptr<uvm_factory> f )
{
  factory = f;
}

//----------------------------------------------------------------------------
// member function: get_default_tr_database (virtual)
//
// returns the current default record database
//
// If no default record database has been set before this method
// is called, returns an instance of <uvm_text_tr_database>
//----------------------------------------------------------------------------

/* TODO TR database
uvm_tr_database* uvm_default_coreservice_t::get_default_tr_database() const
{
  if (tr_database == nullptr)
  {
    //   process p = process::self();
    uvm_text_tr_database* tx_db;
    std::string s;

    //if(p != nullptr)
    //  s = p.get_randstate();

    tx_db = new uvm_tr_database("default_tr_database");
    tr_database = tx_db;

    //if(p != null)
    //  p.set_randstate(s);
  }
  return tr_database;
}

//----------------------------------------------------------------------------
// member function: set_default_tr_database (virtual)
//
// Sets the current default record database to ~db~
//----------------------------------------------------------------------------

void uvm_default_coreservice_t::set_default_tr_database( uvm_tr_database* db )
{
  tr_database = db;
}
*/

//----------------------------------------------------------------------------
// member function: get_report_server (virtual)
//
// returns the current global report_server
// if no report server has been set before, returns an instance of
// uvm_default_report_server
//----------------------------------------------------------------------------

std::shared_ptr<uvm_report_server> uvm_default_coreservice_t::get_report_server()
{
  if(report_server == nullptr)
  { 
    report_server = std::make_shared<uvm_default_report_server>();
  }
  return report_server;
}

//----------------------------------------------------------------------------
// member function: set_report_server (virtual)
//
// sets the central report server to ~server~
//----------------------------------------------------------------------------

void uvm_default_coreservice_t::set_report_server( std::shared_ptr<uvm_report_server> server )
{
  report_server = server;
}
//----------------------------------------------------------------------------
// member function: get_default_packer (virtual)
// Retrieves the default packer policy instance, as defined by set_default_packer If
// set_default_packer has not been called or has been called with a value of null, the implementation returns the
// implementation’s default packer instance.
std::shared_ptr<uvm_packer> uvm_default_coreservice_t::get_default_packer()
{
	if (default_packer == nullptr) {
		default_packer = std::shared_ptr<uvm_packer>(new uvm_packer());
	}
  return default_packer;
}

void uvm_default_coreservice_t::set_default_packer( std::shared_ptr<uvm_packer> packer)
{
  default_packer = packer;
}

std::shared_ptr<uvm_printer> uvm_default_coreservice_t::get_default_printer() 
{
	if (default_printer == nullptr) {
		default_printer = std::shared_ptr<uvm_printer>(get_uvm_default_table_printer(), [](uvm_printer*){});
	}
	return default_printer;
}

void uvm_default_coreservice_t::set_default_printer( std::shared_ptr<uvm_printer> printer )
{
  default_printer = printer;
}

int uvm_default_coreservice_t::allocate_uvm_object_m_inst_count()
{
  return inst_count++;
}

int uvm_default_coreservice_t::get_uvm_object_m_inst_count() 
{
  return inst_count;
}

int& uvm_default_coreservice_t::get_uvm_event_g_cnt()
{
  return event_count;
}

unsigned int& uvm_default_coreservice_t::get_uvm_resource_base_default_precedence()
{
  return resource_base_default_precedence;
}

bool& uvm_default_coreservice_t::get_uvm_default_factory_m_debug_pass()
{
  return default_factory_debug_pass;
}

int& uvm_default_coreservice_t::get_uvm_recorder_handle()
{
  return recorder_handle_counter;
}

uvm_default_coreservice_t::recorder_handle_map_t& uvm_default_coreservice_t::get_uvm_recorder_m_handles()
{
  return recorder_handles;
}

int& uvm_default_coreservice_t::get_uvm_sequencer_base_g_sequencer_id()
{
  return sequencer_id_counter;
}

int& uvm_default_coreservice_t::get_uvm_sequencer_base_g_sequence_id()
{
  return sequence_id_counter;
}

int& uvm_default_coreservice_t::get_uvm_sequencer_base_g_request_id()
{
  return request_id_counter;
}

unsigned int& uvm_default_coreservice_t::get_uvm_reg_m_max_size()
{
  return reg_max_size;
}

unsigned int& uvm_default_coreservice_t::get_uvm_mem_m_max_size()
{
  return mem_max_size;
}

bool& uvm_default_coreservice_t::get_uvm_reg_field_m_predefined()
{
  return reg_field_predefined;
}

unsigned int& uvm_default_coreservice_t::get_uvm_reg_field_m_max_size()
{
  return reg_field_max_size;
}

uvm_default_coreservice_t::reg_field_policy_map_t& uvm_default_coreservice_t::get_uvm_reg_field_m_policy_names()
{
  return reg_field_policy_names;
}

int& uvm_default_coreservice_t::get_uvm_reg_block_id()
{
  return reg_block_id_counter;
}

uvm_default_coreservice_t::reg_block_roots_map_t& uvm_default_coreservice_t::get_uvm_reg_block_m_roots()
{
  return reg_block_roots;
}

uvm_default_coreservice_t::typeid_map_t& uvm_default_coreservice_t::get_uvm_typeid_base_typeid_map()
{
  return typeid_to_callback_map;
}

uvm_default_coreservice_t::type_map_t& uvm_default_coreservice_t::get_uvm_typeid_base_type_map()
{
  return callback_to_typeid_map;
}

uvm_reg_read_only_cbs* uvm_default_coreservice_t::get_uvm_reg_read_only_cbs_m_me() 
{
  if (reg_read_only_cbs == nullptr)
    reg_read_only_cbs.reset(new uvm_reg_read_only_cbs());

  return reg_read_only_cbs.get();
}

uvm_reg_write_only_cbs* uvm_default_coreservice_t::get_uvm_reg_write_only_cbs_m_me() 
{
  if (reg_write_only_cbs == nullptr)
    reg_write_only_cbs.reset(new uvm_reg_write_only_cbs());

  return reg_write_only_cbs.get();
}

uvm_report_catcher_data& uvm_default_coreservice_t::get_uvm_report_catcher_data_instance()
{
  if (report_catcher_data == nullptr)
    report_catcher_data.reset(new uvm_report_catcher_data());

  return *report_catcher_data;
}

uvm_default_coreservice_t::regex_buffer_t& uvm_default_coreservice_t::get_uvm_globals_uvm_re()
{
  return regex_buffer;
}

std::shared_ptr<uvm_comparer> uvm_default_coreservice_t::get_default_comparer() {
	if (default_comparer == nullptr){
		default_comparer = std::shared_ptr<uvm_comparer>(new uvm_comparer());
    }
	return default_comparer;
}

void uvm_default_coreservice_t::set_default_comparer(std::shared_ptr<uvm_comparer> comparer)
{
  default_comparer = comparer;
}


uvm_status_container* uvm_default_coreservice_t::get_uvm_object__m_uvm_status_container() 
{
  if (status_container == nullptr)
    status_container.reset(new uvm_status_container());

  return status_container.get();
}

uvm_resource_pool* uvm_default_coreservice_t::get_uvm_resource_pool_rp() 
{
  if (resource_pool == nullptr)
    resource_pool.reset(new uvm_resource_pool());

  return resource_pool.get();
}

void uvm_default_coreservice_t::reset_uvm_resource_pool_rp()
{
  resource_pool.reset();
  resource_pool_has_wildcard_names = false;
}

bool& uvm_default_coreservice_t::get_uvm_resource_pool_m_has_wildcard_names()
{
  return resource_pool_has_wildcard_names;
}

uvm_table_printer* uvm_default_coreservice_t::get_uvm_default_table_printer() 
{
	if (default_table_printer == nullptr) {
		default_table_printer = new uvm_table_printer;
	}
  return default_table_printer;
}

void uvm_default_coreservice_t::set_uvm_default_table_printer( uvm_table_printer* printer )
{
  default_table_printer = printer;
}


uvm_tree_printer* uvm_default_coreservice_t::get_uvm_default_tree_printer() 
{
    if (default_tree_printer == nullptr) {
        default_tree_printer = new uvm_tree_printer;
    }
  return default_tree_printer;
}

void uvm_default_coreservice_t::set_uvm_default_tree_printer( uvm_tree_printer* printer )
{
  default_tree_printer = printer;
}


uvm_line_printer* uvm_default_coreservice_t::get_uvm_default_line_printer() 
{
    if (default_line_printer == nullptr) {
        default_line_printer = new uvm_line_printer;
    }
  return default_line_printer;
}

void uvm_default_coreservice_t::set_uvm_default_line_printer( uvm_line_printer* printer )
{
  default_line_printer = printer;
}

uvm_phase_queue<uvm_phase*>* uvm_default_coreservice_t::get_uvm_phase_m_phase_hopper() 
{
  if (phase_hopper == nullptr)
    phase_hopper.reset(new uvm_phase_queue<uvm_phase*>());

  return phase_hopper.get();
}

bool& uvm_default_coreservice_t::get_uvm_phase_m_phase_trace()
{
  return phase_trace_enabled;
}

bool& uvm_default_coreservice_t::get_uvm_root_m_uvm_timeout_overridable()
{
  return root_timeout_overridable;
}

bool& uvm_default_coreservice_t::get_uvm_component_global_timeout_spawned_()
{
  return component_global_timeout_spawned;
}

bool& uvm_default_coreservice_t::get_uvm_component__print_config_matches()
{
  return component_print_config_matches;
}

bool& uvm_default_coreservice_t::get_uvm_resource_options_auditing()
{
  return resource_options_auditing;
}

bool& uvm_default_coreservice_t::get_uvm_resource_db_options_ready()
{
  return resource_db_options_ready;
}

bool& uvm_default_coreservice_t::get_uvm_resource_db_options_tracing()
{
  return resource_db_options_tracing;
}

bool& uvm_default_coreservice_t::get_uvm_config_db_options_ready()
{
  return config_db_options_ready;
}

bool& uvm_default_coreservice_t::get_uvm_config_db_options_tracing()
{
  return config_db_options_tracing;
}

bool& uvm_default_coreservice_t::get_uvm_root_m_uvm_header_done()
{
  return root_header_printed;
}

bool& uvm_default_coreservice_t::get_uvm_report_catcher_in_catcher()
{
  return report_catcher_in_catcher;
}

bool& uvm_default_coreservice_t::get_uvm_callbacks_base_m_tracing()
{
  return callbacks_tracing;
}

uvm_reg_map* uvm_default_coreservice_t::get_uvm_reg_map_m_backdoor() 
{
  if (reg_map_backdoor == nullptr)
    reg_map_backdoor.reset(new uvm_reg_map("Backdoor"));

  return reg_map_backdoor.get();
}

uvm_phase* uvm_default_coreservice_t::get_uvm_domain_m_uvm_schedule() 
{
  if (uvm_schedule == nullptr)
    uvm_schedule.reset(new uvm_phase("uvm_sched", UVM_PHASE_SCHEDULE));

  return uvm_schedule.get();
}

uvm_domain* uvm_default_coreservice_t::get_uvm_domain_m_common_domain() 
{
  return common_domain;
}

void uvm_default_coreservice_t::set_uvm_domain_m_common_domain( uvm_domain* domain )
{
  common_domain = domain;
}

uvm_domain* uvm_default_coreservice_t::get_uvm_domain_m_uvm_domain() 
{
  return uvm_domain_ptr;
}

void uvm_default_coreservice_t::set_uvm_domain_m_uvm_domain( uvm_domain* domain )
{
  uvm_domain_ptr = domain;
}

uvm_build_phase* uvm_default_coreservice_t::get_uvm_build_phase_m_inst() 
{
  if (build_phase_ptr == nullptr)
    build_phase_ptr.reset(new uvm_build_phase("build"));

  return build_phase_ptr.get();
}

uvm_connect_phase* uvm_default_coreservice_t::get_uvm_connect_phase_m_inst() 
{
  if (connect_phase_ptr == nullptr)
    connect_phase_ptr.reset(new uvm_connect_phase("connect"));

  return connect_phase_ptr.get();
}

uvm_end_of_elaboration_phase* uvm_default_coreservice_t::get_uvm_end_of_elaboration_phase_m_inst() 
{
  if (end_of_elaboration_phase_ptr == nullptr)
    end_of_elaboration_phase_ptr.reset(new uvm_end_of_elaboration_phase("end_of_elaboration"));

  return end_of_elaboration_phase_ptr.get();
}

uvm_start_of_simulation_phase* uvm_default_coreservice_t::get_uvm_start_of_simulation_phase_m_inst() 
{
  if (start_of_simulation_phase_ptr == nullptr)
    start_of_simulation_phase_ptr.reset(new uvm_start_of_simulation_phase("start_of_simulation"));

  return start_of_simulation_phase_ptr.get();
}

uvm_run_phase* uvm_default_coreservice_t::get_uvm_run_phase_m_inst() 
{
  if (run_phase_ptr == nullptr)
    run_phase_ptr.reset(new uvm_run_phase("run"));

  return run_phase_ptr.get();
}

uvm_extract_phase* uvm_default_coreservice_t::get_uvm_extract_phase_m_inst() 
{
  if (extract_phase_ptr == nullptr)
    extract_phase_ptr.reset(new uvm_extract_phase("extract"));

  return extract_phase_ptr.get();
}

uvm_check_phase* uvm_default_coreservice_t::get_uvm_check_phase_m_inst() 
{
  if (check_phase_ptr == nullptr)
    check_phase_ptr.reset(new uvm_check_phase("check"));

  return check_phase_ptr.get();
}

uvm_report_phase* uvm_default_coreservice_t::get_uvm_report_phase_m_inst() 
{
  if (report_phase_ptr == nullptr)
    report_phase_ptr.reset(new uvm_report_phase("report"));

  return report_phase_ptr.get();
}

uvm_final_phase* uvm_default_coreservice_t::get_uvm_final_phase_m_inst() 
{
  if (final_phase_ptr == nullptr)
    final_phase_ptr.reset(new uvm_final_phase("final"));

  return final_phase_ptr.get();
}

uvm_pre_reset_phase* uvm_default_coreservice_t::get_uvm_pre_reset_phase_m_inst() 
{
  if (pre_reset_phase_ptr == nullptr)
    pre_reset_phase_ptr.reset(new uvm_pre_reset_phase());

  return pre_reset_phase_ptr.get();
}

uvm_reset_phase* uvm_default_coreservice_t::get_uvm_reset_phase_m_inst() 
{
  if (reset_phase_ptr == nullptr)
    reset_phase_ptr.reset(new uvm_reset_phase());

  return reset_phase_ptr.get();
}

uvm_post_reset_phase* uvm_default_coreservice_t::get_uvm_post_reset_phase_m_inst() 
{
  if (post_reset_phase_ptr == nullptr)
    post_reset_phase_ptr.reset(new uvm_post_reset_phase());

  return post_reset_phase_ptr.get();
}

uvm_pre_configure_phase* uvm_default_coreservice_t::get_uvm_pre_configure_phase_m_inst() 
{
  if (pre_configure_phase_ptr == nullptr)
    pre_configure_phase_ptr.reset(new uvm_pre_configure_phase());

  return pre_configure_phase_ptr.get();
}

uvm_configure_phase* uvm_default_coreservice_t::get_uvm_configure_phase_m_inst() 
{
  if (configure_phase_ptr == nullptr)
    configure_phase_ptr.reset(new uvm_configure_phase());

  return configure_phase_ptr.get();
}

uvm_post_configure_phase* uvm_default_coreservice_t::get_uvm_post_configure_phase_m_inst() 
{
  if (post_configure_phase_ptr == nullptr)
    post_configure_phase_ptr.reset(new uvm_post_configure_phase());

  return post_configure_phase_ptr.get();
}

uvm_pre_main_phase* uvm_default_coreservice_t::get_uvm_pre_main_phase_m_inst() 
{
  if (pre_main_phase_ptr == nullptr)
    pre_main_phase_ptr.reset(new uvm_pre_main_phase());

  return pre_main_phase_ptr.get();
}

uvm_main_phase* uvm_default_coreservice_t::get_uvm_main_phase_m_inst() 
{
  if (main_phase_ptr == nullptr)
    main_phase_ptr.reset(new uvm_main_phase());

  return main_phase_ptr.get();
}

uvm_post_main_phase* uvm_default_coreservice_t::get_uvm_post_main_phase_m_inst() 
{
  if (post_main_phase_ptr == nullptr)
    post_main_phase_ptr.reset(new uvm_post_main_phase());

  return post_main_phase_ptr.get();
}

uvm_pre_shutdown_phase* uvm_default_coreservice_t::get_uvm_pre_shutdown_phase_m_inst() 
{
  if (pre_shutdown_phase_ptr == nullptr)
    pre_shutdown_phase_ptr.reset(new uvm_pre_shutdown_phase());

  return pre_shutdown_phase_ptr.get();
}

uvm_shutdown_phase* uvm_default_coreservice_t::get_uvm_shutdown_phase_m_inst() 
{
  if (shutdown_phase_ptr == nullptr)
    shutdown_phase_ptr.reset(new uvm_shutdown_phase());

  return shutdown_phase_ptr.get();
}

uvm_post_shutdown_phase* uvm_default_coreservice_t::get_uvm_post_shutdown_phase_m_inst() 
{
  if (post_shutdown_phase_ptr == nullptr)
    post_shutdown_phase_ptr.reset(new uvm_post_shutdown_phase());

  return post_shutdown_phase_ptr.get();
}
//----------------------------------------------------------------------------
// member function: get_root (virtual)
//----------------------------------------------------------------------------

uvm_root* uvm_default_coreservice_t::get_root()
{
  if (root_ptr == nullptr)
  {
    root_ptr.reset(new uvm_root(uvm_component_name(sc_core::sc_module_name("uvm_top"))));
    root_ptr->m_domain = uvm_domain::get_uvm_domain();
  }

  return root_ptr.get();
}

uvm_report_message* uvm_default_coreservice_t::get_uvm_report_message_l_report_message( const std::string& name ) 
{
  if (shared_report_message == nullptr)
    shared_report_message.reset(new uvm_report_message(name));

  return shared_report_message.get();
}

uvm_report_cb_iter* uvm_default_coreservice_t::get_uvm_report_catcher_name_iter() 
{
  if (report_catcher_name_iter == nullptr)
    report_catcher_name_iter.reset(new uvm_report_cb_iter(nullptr));

  return report_catcher_name_iter.get();
}

uvm_report_cb_iter* uvm_default_coreservice_t::get_uvm_report_catcher_print_iter() 
{
  if (report_catcher_print_iter == nullptr)
    report_catcher_print_iter.reset(new uvm_report_cb_iter(nullptr));

  return report_catcher_print_iter.get();
}

uvm_default_coreservice_t::callback_pool_map_t& uvm_default_coreservice_t::get_uvm_callbacks_base_m_pool()
{
  if (callback_pool == nullptr)
    callback_pool.reset(new callback_pool_map_t());

  return *callback_pool;
}

uvm_callbacks_base* uvm_default_coreservice_t::get_uvm_callbacks_base_m_b_inst() 
{
  if (callbacks_base == nullptr)
    callbacks_base.reset(new uvm_callbacks_base());

  return callbacks_base.get();
}

uvm_default_coreservice_t::domain_map_t& uvm_default_coreservice_t::get_uvm_domain_domains()
{
  return domains;
}

uvm_default_coreservice_t::objection_list_t& uvm_default_coreservice_t::get_uvm_objection_objections()
{
  return objections;
}

uvm_default_coreservice_t::objection_context_list_t& uvm_default_coreservice_t::get_uvm_objection_scheduled_list()
{
  return scheduled_objection_contexts;
}

sc_core::sc_event& uvm_default_coreservice_t::get_uvm_objection_scheduled_list_changed()
{
  return scheduled_objection_list_changed;
}

uvm_default_coreservice_t::phase_execution_map_t& uvm_default_coreservice_t::get_uvm_phase_executing_phases()
{
  return executing_phases;
}

uvm_default_coreservice_t::phase_node_map_t& uvm_default_coreservice_t::get_uvm_phase_phase_nodes()
{
  return phase_nodes;
}

uvm_default_coreservice_t::typed_store_map_t& uvm_default_coreservice_t::get_typed_store()
{
  return typed_store;
}

//----------------------------------------------------------------------------
// member function: set_component_visitor (virtual)
//
// sets the component visitor to ~v~
// (this visitor is being used for the traversal at end_of_elaboration_phase
// for instance for name checking)
//----------------------------------------------------------------------------

/* TODO uvm_visitor
void uvm_default_coreservice_t::set_component_visitor(uvm_visitor<uvm_component> v) const
{
  m_visitor=v;
}
*/

//----------------------------------------------------------------------------
// member function: get_component_visitor (virtual)
//
// retrieves the current component visitor
// if unset(or ~null~) returns a <uvm_component_name_check_visitor> instance
//----------------------------------------------------------------------------

/* TODO uvm_visitor
uvm_visitor<uvm_component> uvm_default_coreservice_t::get_component_visitor() const
{
  if(m_visitor == nullptr)
  {
    uvm_component_name_check_visitor* v = new uvm_component_name_check_visitor("name-check-visitor");
    m_visitor=v;
  }
  return m_visitor;
}

*/
} // namespace uvm
