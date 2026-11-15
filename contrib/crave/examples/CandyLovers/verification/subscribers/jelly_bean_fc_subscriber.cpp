// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#include "jelly_bean_fc_subscriber.h"

void jelly_bean_fc_subscriber::write(const jelly_bean_transaction& t){
  jb_tx = t;
  jb_cg.sample();
}
