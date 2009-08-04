#ifndef TESTXPATHEVALUATION_H_
#define TESTXPATHEVALUATION_H_
// libxml2
#include <libxml/xpathInternals.h>

void testXPathEvaluation();
bool xPathCondition(char* xPathExprChar, char* xmlContextChar);
void printXmlNodes(xmlNodeSetPtr nodes);

#endif /*TESTXPATHEVALUATION_H_*/
