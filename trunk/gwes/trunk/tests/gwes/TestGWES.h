#ifndef TESTGWES_H_
#define TESTGWES_H_
#include <gwes/GWES.h>

void testGWES(gwes::GWES &gwes) ;

bool endsWith(const std::string& s1, const std::string& s2) ;
bool startsWith(const std::string& s1, const std::string& s2) ;

std::string getWorkflowDirectory();
std::string getTestWorkflowDirectory();

#endif /*TESTGWES_H_*/
