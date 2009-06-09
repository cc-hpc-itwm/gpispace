#ifndef PLACE_H_
#define PLACE_H_
// std
#include <ostream>
#include <string>
#include <vector>
using namespace std;
// xerces-c
#include <xercesc/dom/DOM.hpp>
XERCES_CPP_NAMESPACE_USE
//gwdl
#include <gwdl/Token.h>
#include <gwdl/Properties.h>
#include <gwdl/CapacityException.h>

namespace gwdl
{
	
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
	
private: 
	string id;
	string tokenType;
	vector<Token*> tokens;
    unsigned int capacity;
    string description;
    Properties properties;
    
    string generateID();
	
public:

	/**
	 * Constructor for empty place with capacity INT_MAX.
	 * Note: The unique identifier can not be changed after creating the place!
	 * @param _id The unique identifier of this place. If id is set to "", then an id will be generated automatically. 
	 */
	Place(string _id);
	
	/**
	 * Constructor for place to be build from DOMElement.
	 */
	Place(DOMElement::DOMElement* element) throw(CapacityException);
	
	/**
	 * Destructor for place.
	 */
	virtual ~Place();
	
	/**
	 * Get the unique identifier of this place.
	 */
	string getID();
	
	/**
	 * Set the type of this token.
	 */
	void setTokenType(string type);
	
	/**
	 * Get the type of this token.
	 */
 	string getTokenType();
 	
 	/**
 	 * Check, if the place is empty.
 	 * @return "true" if there is no token on this place, "false" otherwise.
 	 */
	bool isEmpty();
	
	/**
	 * Put an additional token on this place.
	 * The maximum number of tokens is defined by the capacity of the place.
	 * Note: This method adds a copy of the token to the place and not a reference or pointer!
	 * @param token The pointer to the token object.
	 */
	void addToken(Token* token) throw(CapacityException);
	
	/**
	 * Remove the i-th token from this place.
	 * @param i The index of the token to be removed.
	 */
	void removeToken(int i);
	
	/**
	 * Get the vector of tokens.
	 * @return The return vector contains pointers to the tokens.
	 */
	vector<Token*>& getTokens();
	
	/**
	 * Removes a specific token from the vector of tokens.
	 * @param token A pointer to the token to be removed.
	 */
	void removeToken(Token* token);

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
	int getCapacity();
	
	/**
	 * Get the amount of tokens which are located on this place.
	 * @return the number of tokens.
	 */
	int getTokenNumber();
	
	/**
	 * Set the human-readable description for this place.
	 * @param d The description as string.
	 */ 
	void setDescription(string d);
	
	/**
	 * Get the human-readable descripton of this place.
	 * @return the description as string.
	 */
	string& getDescription();
	
	/**
	 * Set all the properties of this place.
	 * @param props The properties object.
	 */
	void setProperties(Properties props);

	/**
	 * Get a reference to the properties of this place.
	 * @return A reference to the properties of this place.
	 */
	Properties& getProperties();
	
	/**
	 * Convert this place into a DOMElement.
	 * @param doc The master document this element should belong to.
	 * @return The DOMElement.
	 */
	DOMElement* toElement(DOMDocument* doc);
	
}; // end class Place

} // end namespace

ostream& operator<< (ostream &out, gwdl::Place &place);

#endif /*PLACE_H_*/
