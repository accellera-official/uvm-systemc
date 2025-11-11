// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#pragma once

#include <uvm>

#include "jelly_bean_transaction.h"

using crave::crv_constraint;
using std::string;

class sugar_free_jelly_bean_transaction : public jelly_bean_transaction {
public:
    crv_constraint c_sugarFree{sugar_free() == true};

    UVM_OBJECT_UTILS(sugar_free_jelly_bean_transaction);

    sugar_free_jelly_bean_transaction(const string& name = "sugar_free_transaction") : jelly_bean_transaction(name) {
    };

    virtual ~sugar_free_jelly_bean_transaction() {
    };
};
