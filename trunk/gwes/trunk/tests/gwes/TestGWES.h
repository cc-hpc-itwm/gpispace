#ifndef TESTGWES_H_
#define TESTGWES_H_
#include "../../gwes_cpp/src/GWES.h"

void testGWES(gwes::GWES &gwes) ;

bool endsWith(const std::string& s1, const std::string& s2) ;

std::string getTestWorkflowDirectory();

#endif /*TESTGWES_H_*/
