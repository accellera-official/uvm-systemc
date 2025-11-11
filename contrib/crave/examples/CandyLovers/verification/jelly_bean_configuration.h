// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#pragma once

#include <uvm>
#include <string>

class jelly_bean_configuration : public uvm::uvm_object {
public:
    UVM_OBJECT_UTILS(jelly_bean_configuration);

    jelly_bean_configuration(const std::string& name_ = "") : uvm::uvm_object(name_) {
    };

    virtual ~jelly_bean_configuration() {
    };
};
