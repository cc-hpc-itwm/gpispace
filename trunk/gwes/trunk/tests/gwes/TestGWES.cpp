#include <unistd.h>  // getcwd() definition
#include <sys/param.h>  // MAXPATHLEN definition
// std
#include <iostream>
#include <sstream>
#include <assert.h>
// tests
#include "TestGWES.h"

using namespace std;
using namespace gwdl;
using namespace gwes;
 
void testGWES(GWES &gwes) 
{
	cout << "============== BEGIN GWES TEST =============" << endl;
	cout << "create workflow ..." << endl;
   
   	Workflow *wf = new Workflow();
   	// p0
	Place* p0 = new Place("p0");
	Token* token0 = new Token(true);
	p0->addToken(token0);
	wf->addPlace(p0);
	// p1
	Place* p1 = new Place("p1");
	wf->addPlace(p1);
	// p2
	Place* p2 = new Place("p2");
	wf->addPlace(p2);
	// p3
	Place* p3 = new Place("p3");
	Data* da1 = new Data("<data><param>param</param></data>");
	Token* token1 = new Token(da1);
	p3->addToken(token1);
	wf->addPlace(p3);
	// t0
	Transition* t0 = new Transition("t0");
	Edge* arc0 = new Edge(wf->getPlace("p0"));
	t0->addInEdge(arc0);
	Edge* arc1 = new Edge(wf->getPlace("p1"));
    t0->addOutEdge(arc1);
	wf->addTransition(t0);
	// t1
	Transition* t1 = new Transition("t1");
	Edge* arc2 = new Edge(wf->getPlace("p1"));
	t1->addInEdge(arc2);
	Edge* arc4 = new Edge(wf->getPlace("p3"));
	arc4->setExpression("input");
	t1->addInEdge(arc4);
	Edge* arc3 = new Edge(wf->getPlace("p2"));
	t1->addOutEdge(arc3);
	wf->addTransition(t1);
	// operations
	Operation* op = new Operation();
	wf->getTransition("t1")->setOperation(op);	
	OperationClass* opc = new OperationClass();
	opc->setName("date");
	wf->getTransition("t1")->getOperation()->setOperationClass(opc);
	OperationCandidate* opcand = new OperationCandidate();
	opcand->setType("cli");
	opcand->setOperationName("date");
	opcand->setResourceName("/bin/date");
	opcand->setSelected(true);
	wf->getTransition("t1")->getOperation()->getOperationClass()->addOperationCandidate(opcand);

	cout << "initiate workflow ..." << endl;
	string id = gwes.initiate(*wf, "test");
	
	// print workflow to stdout	
	cout << *wf << endl;
	
	WorkflowHandlerTable& wfht = gwes.getWorkflowHandlerTable();
	assert(wfht.get(id)->getID()==id);
	
	cout << "execute workflow ..." << endl;
	gwes.execute(*wf);

	// print workflow to stdout	
	cout << *wf << endl;
	
	
    cout << "============== END GWES TEST =============" << endl;
   
}

bool endsWith(const string& s1, const string& s2) {
	if ( s2.size() > s1.size() ) return false;
	if ( s1.compare(s1.size()-s2.size(),s2.size(),s2 ) == 0) {
	   return true;
    }
	return false;
}

string getTestWorkflowDirectory() {

	// get GWES_CPP_HOME
	char* gwesHomeP = getenv("GWES_CPP_HOME");
	if (gwesHomeP != NULL) {
		string gwesHome(gwesHomeP);
		if ( gwesHome.size() > 0) {
			return gwesHome + "/workflows/test";
		}
	}
	
	// get current working directory
	char pathC[MAXPATHLEN];
	getcwd(pathC, MAXPATHLEN);
	string path(pathC);

	// if */build/tests
	string s2("/build/tests");
	if ( endsWith(path,s2) ) {
		return string("../../workflows/test");
	}
	
	// if */gwes/trunk
	s2 = string("/gwes/trunk");
	if ( endsWith(path,s2) ) {
		return string("workflows/test");
	}
}
