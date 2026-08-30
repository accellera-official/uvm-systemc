// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#pragma once

#include <uvm>
#include <crave/SystemC.hpp>
#include <crave/ConstrainedRandom.hpp>
#include <crave/frontend/Constraint.hpp>
#include <crave/experimental/Experimental.hpp>

/**
 * \brief Overrides uvm_sequence_item and crv_sequence_item to merge its functionality.
 *
 * This class is intendant as an adapter to merge CRAVE functionality into UVM-SystemC
 * processes without harming them. To use it, simply inherit from this class instead from
 * uvm_sequence_item. You may now use CRAVE variables and constraints in your UVM-Sequence-Item.
 */

class uvm_randomized_sequence_item : public uvm::uvm_sequence_item, public crave::crv_sequence_item {
public:
    /**
     * \brief Register uvm_randomized_sequence_item with UVM-SystemC.
     */
    UVM_OBJECT_UTILS(uvm_randomized_sequence_item);

    /**
     * \brief Creates a new sequence_item with given name.
     *
     * Since both CRAVE and UVM-SystemC are using some kind of name for theier objects
     * , both parent will be named the same. std::string is the lowest common denominator
     * between the nameing type of CRAVE and of UVM-SystemC. Therefore you may pass a
     * std::string or a crave::crv_object_name as argument.
     *
     * \param name Name of this sequence.
     */
    uvm_randomized_sequence_item(crave::crv_object_name name = "crv_uvm_seq_item")
    : uvm::uvm_sequence_item(name()) {
    }

    virtual ~uvm_randomized_sequence_item() {
    };
};

/**
 * \example jelly_bean_transaction.h
 *
 * Demonstrates basic randomizsation of UVM-Transactions using
 * uvm_randomized_sequence_item.
 */
