// Copyright 2015-2016 The CRAVE developers, University of Bremen, Germany. All rights reserved.//

#include "jelly_bean_sb_subscriber.h"

void jelly_bean_sb_subscriber::write(const jelly_bean_transaction& t){
  jelly_bean_scoreboard* jb_sb = dynamic_cast<jelly_bean_scoreboard*>(this->m_comp_parent);
  jb_sb->check_jelly_bean_taste(t);
}
