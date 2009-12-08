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
private:
	/**
     * place the edge is pointing to.
     */
    Place::ptr_t _placeP;

    /**
     * expression of edge.
     */
    std::string _expression;
    
public:

    typedef gwdl::shared_ptr<Edge> ptr_t;

	explicit Edge(Place::ptr_t placeP, std::string expression = "") { _placeP = placeP; _expression = expression; }
	
	~Edge(){};

    /**
     * set Place the Edge is pointing to.
     *
     * @param p Place Edge should point to
     */
    void setPlace(Place::ptr_t p) { _placeP = p;}

    /**
     * get place the edge is pointing to.
     *
     * @return Place Edge points to
     */
    Place::ptr_t getPlace() { return _placeP; }

    std::string getPlaceID() const { return _placeP != NULL ? _placeP->getID() : "";}

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
	
};

}

#endif /*EDGE_H_*/
