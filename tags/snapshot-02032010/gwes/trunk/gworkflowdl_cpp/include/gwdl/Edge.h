/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef EDGE_H_
#define EDGE_H_
//gwdl
#include <gwdl/Memory.h> // shared_ptr
#include <gwdl/Place.h>
//std
#include <string>

namespace gwdl
{

/**
 * An Edge represents an arc from a place to a transition or vice versa.
 * It is used to specify the control or data flow in a Petri net.
 * 
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class Edge
{

public:

	typedef gwdl::shared_ptr<Edge> ptr_t;

	enum scope_t {
		SCOPE_READ=0, // read token
		SCOPE_INPUT,  // consume token
		SCOPE_WRITE,  // write token
		SCOPE_OUTPUT  // put token
	};

	explicit Edge(scope_t scope, Place::ptr_t placeP, std::string expression = "") { _scope = scope; _placeP = placeP; _expression = expression; }

	~Edge(){};

	/**
	 * Get the scope of this edge.
	 */
	scope_t getScope() const { return _scope; }

	/**
	 * get place the edge is pointing to.
	 *
	 * @return Place Edge points to
	 */
	const Place::ptr_t& getPlace() const { return _placeP; }

	const std::string& getPlaceID() const { return _placeP->getID();}

	/**
	 * set Edge's expression.
	 *
	 * @param ex Edge expression
	 */
	void setExpression(const std::string& ex) { _expression = ex; }


	/**
	 * get Edge's expression.
	 *
	 * @return Edge expression
	 */
	const std::string& getExpression() const { return _expression; } 

private:

	/**
	 * scope of the edge (read, input, write, output).
	 */
	scope_t _scope;

	/**
	 * place the edge is pointing to.
	 */
	Place::ptr_t _placeP;

	/**
	 * expression of edge.
	 */
	std::string _expression;

};

}

#endif /*EDGE_H_*/
