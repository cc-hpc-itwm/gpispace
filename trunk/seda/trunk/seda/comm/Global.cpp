#include "Global.hpp"

seda::comm::Global::Ptr seda::comm::Global::instance_ =
    seda::comm::Global::Ptr((seda::comm::IGlobal*)0);
