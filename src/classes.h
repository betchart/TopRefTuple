#ifndef TOPREF_CLASSES_H
#define TOPREF_CLASSES_H

#include "DataFormats/Common/interface/Wrapper.h"
#include <string>
#include <map>

namespace {
  struct dictionary {
    std::map<std::string,int> dummi0;
    edm::Wrapper<std::map<std::string,int> > dummi1;
  };
}

#endif 
