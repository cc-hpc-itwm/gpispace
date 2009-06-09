/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef TOKEN_H_
#define TOKEN_H_

#include <string>
using namespace std;

#include <xercesc/dom/DOM.hpp>
XERCES_CPP_NAMESPACE_USE

#include <gwdl/Properties.h>
#include <gwdl/Data.h>

namespace gwdl
{

class Transition;

/**
 * This class represents the tokens that reside on places on the Petri Net.
 * There are two types of tokens: control tokens holding boolean values and data tokens holding data objects.
 * <p>Code Example:
 * <pre>
 * 	// create default control token with value true
 * 	Token * token1 = new Token();
 *	cout << *token1 << endl;
 * 
 *	// create default control token with value false
 *	Token * token2 = new Token(false);
 *	cout << *token2 << endl;
 * 
 * 	// create control token with properties
 *	Properties *props = new Properties();
 *	props->put("key1","value1");
 *	props->put("key2","value2");
 *	Token * token3 = new Token(*props,true);
 *	cout << *token3 << endl;
 * 
 *	// create data token with data constructed from string
 *	string* str = new string("<data><x>1</x><y>2</y></data>");
 *	Data* data5 = new Data(*str);
 *	Token* token5 = new Token(data5);
 *	cout << *token5 << endl;
 * 
 *	// get data from token
 *	if (token5->isData())
 * 	{
 *		Data *data5b = token5->getData();
 *		string str5b = data5b->toString();
 *		cout << str5b << endl;
 * 	} 
 * </pre>
 *  
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 * 
 */ 
class Token
{
	
private:
	long id;
    Data* data;
    Properties properties;
    bool control;
    Transition* p_lock;
    
    long generateID() {static long counter = 0; return counter++;}
	
public:

	/**
	 * Construct a default control token with control = <code>true</code>.
	 */ 
	Token() {id = generateID(); data=NULL; control = true; p_lock = NULL;}
	
	/**
	 * Constructor for control token with specified value.
	 * @param _control Boolean value of the control token.
	 */ 
	Token(bool _control){id = generateID(); data=NULL; control = _control; p_lock = NULL;}
	
	/**
	 * Constructor for control token with specified value and properties.
	 * @param _properties The properties of this token.
	 * @param _control The control value of this token.
	 */
	Token(Properties _properties, bool _control)
	 {id = generateID(); data=NULL; control = _control; properties = _properties; p_lock = NULL;}
	
	/**
	 * Constructor for data token.
	 * Note: When the token is deleted, then also the data object will be deleted! 
	 * @param _data XML content of the data token as data object.
	 */  
	Token(Data* _data);
	
	/**
	 * Constructor for data token with specific properties.
	 * Note: When the token is deleted, then also the data object will be deleted! 
	 * @param _properties The properties of this data token.
	 * @param _data The data of this token.
	 */
	Token(Properties _properties, Data* _data)
	  {id = generateID(); properties = _properties; data = _data; p_lock = NULL;} 
	
	/**
	 * Construct a data token based on its DOMElement.
	 * Note: When the token is deleted, then also the data object will be deleted! 
	 * @param element XML content of the data token as DOMElement.
	 */
	Token(DOMElement* element);
	
	/**
	 * Convert this token into a DOMElement.
	 * @param doc The master document this element should belong to.
	 * @return The DOMElement.
	 */
	DOMElement* toElement(DOMDocument* doc);
	
	/**
	 * Destructor for data token.
	 * Note: When the token is deleted, then also the data object will be deleted! 
	 */
	virtual ~Token() {
		try {
			delete data;
		} catch (DOMException e) {
			///
		}
		data = NULL;
	}
	
	/**
	 * Get a reference to the properties of this token.
	 * @return A reference to the properties of this token.
	 */
	Properties& getProperties() {return properties;}
	
	/**
	 * Get the data content of this data token.
	 * @return The data of this token.
	 */
	Data* getData() {return data;}
	
	/**
	 * Check, whether this token is a control or data token.
	 * @return <code>true</code> if token is data token, false otherwise.
	 */
	bool isData() {return (data!=NULL);}
	
	/**
	 * Get the control value of this control token.
	 */
	bool getControl() {return control;}
	
	/**
	 * Get internal id of this token.
	 */
	long getID() {return id;}
	
	/**
	 * Lock this token. Locked tokens will not be regarded in the decision whether a transition is enabled or not.
     * The lock has no XML representation in the GWorkflowDL and is not propagated to distributed instances.
     * @param p_transition The transition that locked the token.
     */
	void lock(Transition* p_transition) {p_lock = p_transition;}
	
	/**
	 * Unlock this token.
	 */
	void unlock() {p_lock = NULL;}
	
	/**
	 * Test if this token is locked.
     * @return returns <code>true</code> if the token is locked, <code>false</code> otherwise.
     */
	bool isLocked() {return p_lock != NULL;}
	
	/**
     * Test if this token is locked by a specific transition
     * @param p_transition
     * @return returns <code>true</code> if the token has been locked by the specific transition, <code>false</code> otherwise.
     */
    bool isLockedBy(Transition* p_transition) {return p_lock == p_transition;}
	
}; // end class Token

} // end namespace gwdl

ostream& operator<< (ostream &out, gwdl::Token &token);

#endif /*TOKEN_H_*/
