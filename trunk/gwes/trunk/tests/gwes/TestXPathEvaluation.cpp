// test
#include "TestXPathEvaluation.h"
// libxml
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
// std
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
 
#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)

void testXPathEvaluation() {
	cout << "============== BEGIN XpathEvaluation TEST =============" << endl;
    cout << "xpath evaluation..." << endl;
    
	cout << "============== END XpathEvaluation TEST =============" << endl;
}


#else
int main(void) {
    cerr << "XPath support not compiled in" << endl;
    exit(1);
}
#endif
