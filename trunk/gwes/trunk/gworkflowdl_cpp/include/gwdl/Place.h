/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef PLACE_H_
#define PLACE_H_
//gwdl
#include <gwdl/Memory.h> // shared_ptr
#include <gwdl/Token.h>
#include <gwdl/Properties.h>
#include <gwdl/CapacityException.h>
// std
#include <ostream>
#include <string>
#include <vector>

namespace gwdl
{
    class Token;
    class Transition;
	
#define Place_DEFAULT_CAPACITY INT_MAX
#define Place_DEFAULT_TOKEN_NUMBER 0

/**
 * This class represents places of Petri nets, which hold the state of the net.
 * Places are connected with directed edges (=arcs) with Transitions. Places can hold tokens. 
 * The maximum number of tokens on a place is defined by its capacity.
 * 
 * <p>For a code example please refer to the description of the class gwdl::Workflow.
 * 
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */ 
class Place
{
	
public:
	
    typedef gwdl::shared_ptr<Place> ptr_t;

	/**
	 * Constructor for empty place with capacity INT_MAX.
	 * Note: The unique identifier can not be changed after creating the place!
	 * @param _id The unique identifier of this place. If id is set to "", then an id will be generated automatically. 
	 */
	explicit Place(const std::string& id);
	
	/**
	 * Destructor for place.
	 */
	virtual ~Place();
	
	/**
	 * Get the unique identifier of this place.
	 */
	const std::string& getID() const;
	
	/**
	 * Set the type of this token.
	 */
	void setTokenType(const std::string& type);
	
	/**
	 * Get the type of this token.
	 */
 	const std::string& getTokenType() const;
 	
 	/**
 	 * Check, if the place is empty.
 	 * @return "true" if there is no token on this place, "false" otherwise.
 	 */
	bool isEmpty() const;
	
	/**
	 * Put an additional token on this place.
	 * The maximum number of tokens is defined by the capacity of the place.
	 * Note: This method adds a pointer of the token to the place and not a copy!
	 * @param token The pointer to the token object.
	 */
	void addToken(Token::ptr_t& tokenP) throw(CapacityException);
	
	/**
	 * Remove the i-th token from this place.
	 * @param i The index of the token to be removed.
	 */
	void removeToken(int i);
	
	/**
	 * Get the vector of tokens.
	 * @return The return vector contains pointers to the tokens.
	 */
	const std::vector<Token::ptr_t>& getTokens() const;
	
	/**
	 * Removes a specific token from the vector of tokens.
	 * @param token A pointer to the token to be removed.
	 */
	void removeToken(Token::ptr_t& token);

	/**
	 * Remove all tokens that are placed on this place.
	 */
	void removeAllTokens();
	
	/**
	 * Set the maximum numbers of tokens that are allowed on this place.
	 */
	void setCapacity(unsigned int capacity) throw(CapacityException);
	
	/**
	 * Get the maximum number of tokens that are allowed on this place.
	 */
	int getCapacity() const;
	
	/**
	 * Get the amount of tokens which are located on this place.
	 * @return the number of tokens.
	 */
	int getTokenNumber() const;
	
	/**
	 * Set the human-readable description for this place.
	 * @param d The description as std::string.
	 */ 
	void setDescription(const std::string& d);
	
	/**
	 * Get the human-readable descripton of this place.
	 * @return the description as string.
	 */
	const std::string& getDescription() const;
	
	/**
	 * Set all the properties of this place.
	 * @param props The properties object.
	 */
	void setProperties(Properties::ptr_t& propsP);

	/**
	 * Get a shared pointer to the properties of this place.
	 * Returns NULL if there are no properties. 
	 * @return A shared pointer to the place properties or NULL if there are no properties.
	 */
	Properties::ptr_t& getProperties();
	
	/**
	 * Get a read-only pointer to the properties of this place.
	 * Returns NULL if there are no properties. 
	 * @return A read-only shared pointer to the properties or NULL if there are no properties.
	 */
	const Properties::ptr_t& readProperties() const;

	/**
	 * Put new name/value pair into properties.
	 * Overwrites old property with same name. Generates new Property map if required. 
	 * @param name The name of the property.
	 * @param value The value of the property.
	 */
	void putProperty(const std::string& name, const std::string& value);
	
	/**
	 * Lock a token. 
	 * Locked tokens will not be regarded in the decision whether a transition is enabled or not.
     * The lock has no XML representation in the GWorkflowDL and is not propagated to distributed instances.
     * @param p_token The token to lock
     * @param p_transition The transition that locked the token.
     */
	void lockToken(Token::ptr_t& tokenP, Transition* transitionP);
	
	/**
	 * Unlock a token.
	 */
	void unlockToken(Token::ptr_t& tokenP);
	
	/**
	 * Get the next unlocked token of this place.
	 * Returns NULL if there is no unlocked token on this place.
	 */ 
	Token::ptr_t& getNextUnlockedToken();
	
private: 
	std::string _id;
	std::string _tokenType;
	std::vector<Token::ptr_t> _tokens;
    unsigned int _capacity;
    std::string _description;
    Properties::ptr_t _propertiesP;
    Token::ptr_t _nextUnlockedTokenP;
    
    std::string generateID() const;
	
}; // end class Place

} // end namespace

#endif /*PLACE_H_*/
