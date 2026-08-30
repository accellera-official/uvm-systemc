// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#include "jelly_bean_scoreboard.h"

using uvm::uvm_table_printer;

void jelly_bean_scoreboard::build_phase(uvm::uvm_phase& phase){
  uvm_scoreboard::build_phase(phase);
  jb_sb_sub = jelly_bean_sb_subscriber::type_id::create("jb_sb_sub", this);
}

void jelly_bean_scoreboard::connect_phase(uvm::uvm_phase& phase){
  uvm_scoreboard::connect_phase(phase);
  jb_analysis_export.connect(jb_sb_sub->analysis_export);
}

void jelly_bean_scoreboard::check_jelly_bean_taste(
    jelly_bean_transaction jb_tx){
  uvm_table_printer* p = new uvm_table_printer();
  std::stringstream concat;
  if (jb_tx.flavor == flavor_e::CHOCOLATE && jb_tx.sour) {
    if (jb_tx.taste == taste_e::YUCKY) {
      concat << "You have a good sense of taste." << std::endl << jb_tx.sprint(p);
      UVM_INFO("jelly_bean_scoreboard", concat.str(), uvm::UVM_LOW);
    } else {
      concat << "You lost sense of taste!" << std::endl << jb_tx.sprint(p);
      UVM_ERROR("jelly_bean_scoreboard", concat.str());
    }
  } else
    if (jb_tx.taste == taste_e::YUMMY) {
      concat << "You have a good sense of taste." << std::endl << jb_tx.sprint(p);
      UVM_INFO("jelly_bean_scoreboard", concat.str(), uvm::UVM_LOW);
    } else {
      concat << "You lost sense of taste!" << std::endl << jb_tx.sprint(p);
      UVM_ERROR("jelly_bean_scoreboard", concat.str());
    }
}
