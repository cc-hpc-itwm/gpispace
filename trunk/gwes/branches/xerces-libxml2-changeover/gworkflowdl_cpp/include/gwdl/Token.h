/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef TOKEN_H_
#define TOKEN_H_
// gwdl
#include <gwdl/Memory.h> // shared_ptr
#include <gwdl/Properties.h>
#include <gwdl/IToken.h>
#include <gwdl/Data.h>
//#include <gwdl/Transition.h>

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
 *	Token::ptr_t token5 = new Token(data5);
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
class Token : public IToken
{

public:
	
    typedef gwdl::shared_ptr<Token> ptr_t;
    
    enum control_t {
    	CONTROL_FALSE = 0,
    	CONTROL_TRUE = 1
    };

	/**
	 * Construct a default control token with control = <code>true</code>.
	 */ 
	Token();
	
	/**
	 * Constructor for control token with specified value.
	 * @param _control Boolean value of the control token.
     *
     * FIXME: C++ automatically converts nearly everything to bool
     *        which means that code like 'Token("foo")' compiles,
     *        it just creates a Control-Token with control==true
     *
     * TODO: remove this constructor and replace it with explicit
     *       factory method
	 */ 
	explicit Token(control_t control);
	
	/**
	 * Constructor for control token with specified value and properties.
	 * @param _properties The properties of this token.
	 * @param _control The control value of this token.
	 */
	explicit Token(Properties::ptr_t propertiesP, control_t control);
	
	/**
	 * Constructor for data token.
	 * Note: When the token is deleted, then also the data object will be deleted! 
	 * @param _data XML content of the data token as data object.
	 */  
	explicit Token(Data::ptr_t dataP);
	
	/**
	 * Constructor for data token with specific properties.
	 * Note: When the token is deleted, then also the data object will be deleted! 
	 * @param _properties The properties of this data token.
	 * @param _data The data of this token.
	 */
	explicit Token(Properties::ptr_t propertiesP, Data::ptr_t dataP);
	
	/**
	 * Destructor for data token.
	 * Note: When the token is deleted, then also the data object will be deleted! 
	 */
	virtual ~Token();
	
	/**
	 * Get a shared pointer to the properties of this token. 
	 * Returns NULL if there are no properties. 
	 * @return A shared pointer to the Token properties or NULL if there are no properties.
	 */
	Properties::ptr_t getProperties() {return _propertiesP;}
	
	/**
	 * Get a read-only pointer to the properties of this token.
	 * Returns NULL if there are no properties. 
	 * @return A read-only shared pointer to the properties or NULL if there are no properties.
	 */
	const Properties::ptr_t readProperties() const {return _propertiesP;}

	/**
	 * Put new name/value pair into properties.
	 * Overwrites old property with same name. Generates new Property map if required. 
	 * @param name The name of the property.
	 * @param value The value of the property.
	 */
	void putProperty(const std::string& name, const std::string& value);
	
	/**
	 * Get a shared pointer to the data object of this data token.
	 * @return The data of this token.
	 */
	const Data::ptr_t getData() const {return _dataP;}
	
	/**
	 * Check, whether this token is a control or data token.
	 * @return <code>true</code> if token is data token, false otherwise.
	 */
	bool isData() const {return (_dataP!=NULL);}
	
	/**
	 * Get the control value of this control token.
	 */
	bool getControl() const {return _control;}
	
	/**
	 * Get internal id of this token.
	 */
	long getID() const {return _id;}
	
	/**
	 * Internal method to lock this token. Please use Place.lockToken() instead to keep track of the 
	 * next unlocked token.
	 * Locked tokens will not be regarded in the decision whether a transition is enabled or not.
     * The lock has no XML representation in the GWorkflowDL and is not propagated to distributed instances.
     * @param transitionP The transition that locked the token.
     */
	void lock(Transition* transitionP) {_lockP = transitionP;}
	
	/**
	 * Internal method to unlock this token. Please use Place.unlockToken() instead to keep track of the 
	 * next unlocked token.
	 */
	void unlock() {_lockP = NULL; }
	
	/**
	 * Test if this token is locked.
     * @return returns <code>true</code> if the token is locked, <code>false</code> otherwise.
     */
	bool isLocked() const {return _lockP != NULL;}
	
	/**
     * Test if this token is locked by a specific transition
     * @param transitionP
     * @return returns <code>true</code> if the token has been locked by the specific transition, <code>false</code> otherwise.
     */
    bool isLockedBy(Transition* transitionP) const {return _lockP == transitionP;}
    
	/**
	 * Make a deep copy of this Token object and return a pointer to the new Token.
	 * Note: If token contains Data object, then also a deep copy of this data object is made.
	 * The new token will have a new id.
	 * Locks from transitions will be removed on the cloned token.
	 * @return Pointer to the cloned Token object.
	 */ 
	ptr_t deepCopy() const;
	
private:
	long _id;
    Data::ptr_t _dataP;
    Properties::ptr_t _propertiesP;
    control_t _control;
    Transition* _lockP;
    
    long generateID() {static long counter = 0; return counter++;}
	
}; // end class Token

} // end namespace gwdl

#endif /*TOKEN_H_*/
